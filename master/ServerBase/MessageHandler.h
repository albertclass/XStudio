#ifndef _MESSAGE_HANDLER_H_
#define _MESSAGE_HANDLER_H_
#include "minilzo.h"
#include "ServerBase.h"
#include "ServerDefines.h"

extern __declspec( thread ) xgc_lpvoid gpNetBuffer;

namespace Network
{
#	define SAFE_BUFFER_SIZE 1024	// 安全隔离空间大小
#	define HEAP_ALLOC(var,size) \
	lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

	extern char g_BeginSafeBufer[SAFE_BUFFER_SIZE];	// 安全隔离空间
	extern HEAP_ALLOC( wrkmem, LZO1X_1_MEM_COMPRESS );
	extern char g_EndSafeBufer[SAFE_BUFFER_SIZE];	// 安全隔离空间

#	define XGC_DECODE_MESSAGE( TYPE, STORAGE, NETDATA ) \
	TYPE STORAGE; \
	xgc_size decode = Decode( STORAGE, NETDATA ); \
	XGC_ASSERT_THROW( decode != -1, CMsgException( "消息["###TYPE##"]DECODE_MESSAGE失败", NETDATA.GetDataLen(), NETDATA.GetData(), (xgc_int32)NETDATA.GetSize() ) );\

#	define XGC_CREATE_MESSAGE( TYPE, STORAGE, NETDATA ) \
	std::unique_ptr< TYPE > STORAGE( XGC_NEW TYPE ); \
	xgc_size decode = Decode( *##STORAGE##.get(), NETDATA ); \
	XGC_ASSERT_THROW( decode != -1, CMsgException( "消息["###TYPE##"]CREATE_MESSAGE失败", NETDATA.GetDataLen(), NETDATA.GetData(), (xgc_int32)NETDATA.GetSize() ) );\

	///
	/// [2/19/2014 albert.xu]
	/// 内存布局需要与MessageHandler一致
	///
	struct MessageHandler
	{
		xgc_uint16	uMessage;
		xgc_void( *fnHandler )( ... );
	};

	XGC_INLINE xgc_bool operator<( const MessageHandler& lhs, const MessageHandler& rhs )
	{
		return lhs.uMessage < rhs.uMessage;
	}

	XGC_INLINE xgc_bool operator<( xgc_uint16 lhs, const MessageHandler& rhs )
	{
		return lhs < rhs.uMessage;
	}

	XGC_INLINE xgc_bool operator<( const MessageHandler& lhs, xgc_uint16 rhs )
	{
		return lhs.uMessage < rhs;
	}

	XGC_INLINE xgc_bool operator==( const MessageHandler& lhs, const MessageHandler& rhs )
	{
		return lhs.uMessage == rhs.uMessage;
	}

	///
	/// [2/19/2014 albert.xu]
	/// 对消息列表进行排序
	///
	xgc_bool SortMessageHandler( MessageHandler* pFirst, MessageHandler* pLast );

	///
	/// [2/28/2014 albert.xu]
	/// 消息打包
	///
	template< class Package >
	xgc_lpstr BuildPackage( xgc_uint16 nMsgID, const Package& rPkg, xgc_uint32 dwTransID = 0, xgc_bool bCompress = false )
	{
		if( gpNetBuffer == xgc_nullptr )
		{
			gpNetBuffer = malloc( 1024 * 4 );
			memset( gpNetBuffer, 0, _msize( gpNetBuffer ) );
		}

		const xgc_size nHeaderLen = sizeof( xgc_lpvoid ) + sizeof( xgc_size );
		for( ;; )
		{
			PkgHeader *pPkgHeader = (PkgHeader *) ( (xgc_lpstr) gpNetBuffer + nHeaderLen );
			MsgHeader *pMsgHeader = (MsgHeader *) ( pPkgHeader + 1 );
			pMsgHeader->uClass    = htons( nMsgID >> 10 );
			pMsgHeader->uMessage  = htons( nMsgID );
			pMsgHeader->uReserved = htonl( dwTransID );

			xgc_lpstr pCursor = (xgc_lpstr) ( pMsgHeader + 1 );

			xgc_size siz = _msize( gpNetBuffer ) - ( pCursor - (xgc_lpstr) gpNetBuffer );
			xgc_size len = Encode( rPkg, CNetData( pCursor, siz ) );
			if( len != -1 )
			{
				if( bCompress )
				{
					if( siz - len > TOTAL_HEADER_LEN )
					{
						// 确保还够一个消息头的空间
						pPkgHeader = (PkgHeader *) ( pCursor + len );
						pMsgHeader = (MsgHeader *) ( pPkgHeader + 1 );

						pMsgHeader->uClass = htons( nMsgID >> 10 );
						pMsgHeader->uMessage = htons( nMsgID );
						pMsgHeader->uReserved = htonl( dwTransID );

						xgc_lpstr dst = (xgc_lpstr) ( pMsgHeader + 1 );
						lzo_uint dst_len = (xgc_lpcstr) gpNetBuffer + _msize( gpNetBuffer ) - dst;
						if( dst_len > len + ( len >> 4 ) + 64 + 3 )
						{
							xgc_int32 ret = lzo1x_1_compress( (lzo_bytep) pCursor,
								len,
								(unsigned char*) dst, // 存放压缩数据的buffer
								(lzo_uint*) &dst_len, // 压缩buffer的长度
								wrkmem );

							if( ret == LZO_E_OK )
							{
								len = static_cast<xgc_uint16>( dst_len );

								pPkgHeader->uLength = htons( (xgc_uint16) len );
								return (xgc_lpstr) pPkgHeader;
							}
						}
					}
				}
				else
				{
					pPkgHeader->uLength = htons( (xgc_uint16) len );
					return (xgc_lpstr) pPkgHeader;
				}
			}
			//////////////////////////////////////////////////////////////////////////
			// 这个算法有BUG，多线程同时访问时，若同时多个线程发生重分配则会导致链表被破坏
			xgc_size memsize = _msize( gpNetBuffer );
			xgc_lpvoid pNewBuffer = realloc( gpNetBuffer, memsize + 1024 * 16 );
			if( pNewBuffer == xgc_nullptr )
			{
				SYS_ERROR( "network send buffer realloc failed. current size = %Iu", memsize );
				return xgc_nullptr;
			}

			SYS_INFO( "network send buffer realloc successful. current size = %Iu", memsize );
			gpNetBuffer = pNewBuffer;
		}
	}

	///
	/// 事件日志打包
	/// [12/12/2014] create by albert.xu
	///
	template< class Package >
	xgc_lpstr BuildEventLog( xgc_uint16 nEvent, const Package& rPkg )
	{
		if( gpNetBuffer == xgc_nullptr )
		{
			gpNetBuffer = malloc( 1024 * 4 );
			memset( gpNetBuffer, 0, _msize( gpNetBuffer ) );
		}

		const xgc_size nHeaderLen = sizeof( xgc_lpvoid ) + sizeof( xgc_size );
		for( ;; )
		{
			EvtHeader *pEvtHeader = (EvtHeader *) ( (xgc_lpstr) gpNetBuffer + nHeaderLen );
			pEvtHeader->magic = EVENT_MAGIC_WORD;
			pEvtHeader->type = nEvent;
			GetServerCode( nullptr, nullptr, &pEvtHeader->gameGroup, &pEvtHeader->gameArea );
			pEvtHeader->logTime = current_time();

			xgc_lpstr pCursor = (xgc_lpstr) ( pEvtHeader + 1 );

			xgc_size siz = _msize( gpNetBuffer ) - ( pCursor - (xgc_lpstr) gpNetBuffer );
			xgc_size len = Encode( rPkg, CNetData( pCursor, siz ), 0 );
			if( len != -1 )
			{
				pEvtHeader->length = (xgc_uint16) len;
				return (xgc_lpstr) pEvtHeader;
			}

			//////////////////////////////////////////////////////////////////////////
			// 这个算法有BUG，多线程同时访问时，若同时多个线程发生重分配则会导致链表被破坏
			xgc_size memsize = _msize( gpNetBuffer );
			xgc_lpvoid pNewBuffer = realloc( gpNetBuffer, memsize + 1024 * 16 );
			if( pNewBuffer == xgc_nullptr )
			{
				SYS_ERROR( "network send buffer realloc failed. current size = %Iu", memsize );
				return xgc_nullptr;
			}

			SYS_INFO( "network send buffer realloc successful. current size = %Iu", memsize );
			// 查找旧的链表指针，并修改链表中的指针
			gpNetBuffer = pNewBuffer;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 发送消息, 仅接受所列的类对象
	xgc_bool SendPackage( CPipeSession* pSession, xgc_lpcstr pData );

	xgc_bool SendPackage( CGameSession* pSession, xgc_lpcstr pData );

	xgc_bool SendPackage( CGateSession* pSession, xgc_lpcstr pData );

	///
	/// 释放所有发送缓冲
	/// [2/20/2014 albert.xu]
	///
	xgc_void FreeMessageBuffer();
}

using namespace Network;
#endif // _MESSAGE_HANDLER_H_