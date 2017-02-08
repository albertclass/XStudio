#include "Netbase.h"
#include "asio_Header.h"

namespace xgc
{
	namespace net
	{
		xgc_bool		(*InitNetwork)( int workthreads ) = 0;
		xgc_void		(*FiniNetwork)() = 0;
		xgc_uintptr     (*StartServer)( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, MessageQueuePtr &queue ) = 0;
		xgc_uintptr     (*StartServerEx)( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, const pfnCreateHolder &creator ) = 0;
		xgc_void		(*CloseServer)( xgc_uintptr server_h ) = 0;
		xgc_bool		(*Connect)( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, MessageQueuePtr &queue ) = 0;
		xgc_bool	    (*ConnectAsync)( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, MessageQueuePtr &queue ) = 0;
		xgc_bool		(*ConnectEx)( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, const pfnCreateHolder &creator ) = 0;
		xgc_bool	    (*ConnectAsyncEx)( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, const pfnCreateHolder &creator ) = 0;
		xgc_void		(*SendPacket)( network_t handle, xgc_lpvoid data, xgc_size size ) = 0;
		xgc_void		(*SendLastPacket)( network_t handle, xgc_lpvoid data, xgc_size size ) = 0;
		xgc_void		(*SendPackets)( network_t *handle, xgc_uint32 count, xgc_lpvoid data, xgc_size size ) = 0;
		xgc_void		(*SendToGroup)( group_t group, xgc_lpvoid data, xgc_size size, xgc_bool toself ) = 0;
		xgc_void		(*CloseLink)( network_t handle ) = 0;
		xgc_uintptr		(*ExecuteState)( xgc_uint32 operate_code, xgc_uintptr param ) = 0;// 返回 0 表示成功, < 0 表示发生错误 > 0 表示可以正常执行,但有其他问题.

		XGC_DECLSPEC_THREAD xgc_lpvoid packet_base = 0;

		#define HEADER_SIZE (4)
		xgc_size ProtocalDefault::header_space()
		{
			return 4;
		}

		xgc_size ProtocalDefault::packet_length( xgc_lpstr header )
		{
			// return packet length
			return *(xgc_uint16*)header;
		}

		// filter your interesting packet, return event code
		xgc_uint32 ProtocalDefault::packet_filter( xgc_lpstr header )
		{
			if( header[2] == SYSTEM_MESSAGE_TYPE )
			{
				switch( header[3] )
				{
				case EVENT_PING: return FILTER_SYSTEM_PING;
				case EVENT_PONG: return FILTER_SYSTEM_PONG;
				}
			}
			return FILTER_PASS;
		}

		// return packet base address;
		xgc_size ProtocalDefault::packet_system( xgc_byte event, xgc_lpstr header )
		{
			// event = EVENT_CODE { EVENT_ACCEPT, EVENT_CLOSE, EVENT_ERROR, EVENT_CONNECT, EVENT_PING, EVENT_PONG }
			//switch( event )
			//{
			//case EVENT_ACCEPT:
			//case EVENT_CLOSE:
			//case EVENT_CONNECT:
			//case EVENT_ERROR:
			//case EVENT_PING:
			//case EVENT_PONG:
			//	header[2] = SYSTEM_MESSAGE_TYPE;
			//	header[3] = event;
			//	break;
			//}

			header[2] = SYSTEM_MESSAGE_TYPE;
			header[3] = event;

			return header_space();
		}

		xgc_size ProtocalDefault::packet_finial( xgc_lpstr header, xgc_size length )
		{
			*(xgc_uint16*)header = (xgc_uint16)length;
			return length;
		}

		xgc_bool ProtocalDefault::packet_verifiy( xgc_lpstr data, xgc_size size, xgc_lpvoid context)
		{
			return true;
		}

		xgc_size ProtocalDefault::packet_decrypt( xgc_lpstr data, xgc_size size, xgc_lpstr output, xgc_lpvoid context)
		{
			return 0;
		}

		xgc_size ProtocalDefault::packet_encrypt( xgc_lpstr data, xgc_size size, xgc_lpstr output, xgc_lpvoid context)
		{
			return 0;
		}

		#define SETUP_API( PIX, FNAME )	XGC_VERIFY( FNAME = XGC_CONCATENATE_MACRO(PIX,FNAME) )
		//--------------------------------------------------------//
		//	created:	3:12:2009   14:27
		//	filename: 	net
		//	author:		Albert.xu
		//
		//	purpose:	加载网络库
		//--------------------------------------------------------//
		xgc_bool CreateNetwork( eNetLibrary type )
		{
			if( type == asio )
			{
				SETUP_API( asio_, InitNetwork );
				SETUP_API( asio_, FiniNetwork );
				SETUP_API( asio_, StartServer );
				SETUP_API( asio_, StartServerEx );
				SETUP_API( asio_, CloseServer );
				SETUP_API( asio_, Connect );
				SETUP_API( asio_, ConnectAsync );
				SETUP_API( asio_, ConnectEx );
				SETUP_API( asio_, ConnectAsyncEx );
				SETUP_API( asio_, SendPacket );
				SETUP_API( asio_, SendLastPacket );
				SETUP_API( asio_, SendPackets );
				SETUP_API( asio_, SendToGroup );
				SETUP_API( asio_, CloseLink );
				SETUP_API( asio_, ExecuteState );
			}
			return true;
		}

		//--------------------------------------------------------//
		//	created:	3:12:2009   14:27
		//	filename: 	net
		//	author:		Albert.xu
		//
		//	purpose:	释放网络库
		//--------------------------------------------------------//
		xgc_void DestroyNetwork()
		{
		};

		//////////////////////////////////////////////////////////////////////////
		static pfnProcessor	deliver_table[255][255];

		xgc_void InstallDeliver( xgc_byte type, xgc_byte id, pfnProcessor fn )
		{
			deliver_table[type][id] = fn;
		}

		xgc_long DeliverMessage( MessageQueuePtr pQueue, xgc_long nCount )
		{
			xgc_long nProcess = 0;
			INetPacket* pPacketMsg = xgc_nullptr;

			while( nProcess < nCount && pQueue->PopMessage( &pPacketMsg ) )
			{
				MessageHeader* header = (MessageHeader*)pPacketMsg->header();
				XGC_ASSERT( header->length == pPacketMsg->length() );

				pfnProcessor functor = deliver_table[header->type][header->code];
				if( functor )
				{
					functor( pPacketMsg->handle(), pPacketMsg->data(), pPacketMsg->size() );
				}

				pPacketMsg->freedom();
				nProcess++;
			}

			return nProcess;
		}

	}
}
