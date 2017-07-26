#include "header.h"
#include "game_message.h"

///
/// \brief 管道消息
/// \author albert.xu
/// \date 2017/07/18
///
xgc_ulong OnPipeMsg( CPipeSession* pSession, xgc_lpvoid pData, xgc_size nSize, xgc_uint32 hHandle )
{
	return 0;
}

///
/// \brief 管道事件
/// \author albert.xu
/// \date 2017/07/18
///
xgc_void OnPipeEvtError( CPipeSession* pSession, xgc_uint64 nErrorCode );

///
/// \brief 管道事件
/// \author albert.xu
/// \date 2017/07/18
///
xgc_ulong OnPipeEvt( CPipeSession* pSession, xgc_uint32 nEvent, xgc_uint64 nBring )
{
	switch( nEvent )
	{
	case EVENT_CONNECT:
		break;
	case EVENT_ERROR:
		OnPipeEvtError( pSession, nBring );
		break;
	case EVENT_CLOSE:
		// SAFE_DELETE( pSession );
		break;
	}
	return 0;
}

///
/// \brief 管道事件
/// \author albert.xu
/// \date 2017/07/18
///
xgc_void OnPipeEvtError( CPipeSession* pSession, xgc_uint64 nErrorCode )
{
	//switch( nErrorCode )
	//{
	//case NET_ERROR_CONNECT: 
	//case NET_ERROR_CONNECT_TIMEOUT: 
	//case NET_ERROR_NOT_ENOUGH_MEMROY: 
	//case NET_ERROR_HEADER_LENGTH: 
	//case NET_ERROR_DECRYPT_LENGTH: 
	//case NET_ERROR_PACKET_SPACE: 
	//case NET_ERROR_PACKET_INVALID: 
	//case NET_ERROR_SEND_BUFFER_FULL:
	//}
	SYS_ERROR( "网络发生错误 nError = %0X", nErrorCode );
}
///
/// \brief 虚拟套接字消息
/// \author albert.xu
/// \date 2017/07/18
///
xgc_ulong OnVirtualSockMsg( CRelaySession*, xgc_lpvoid, xgc_size, xgc_uint32 )
{
	return 0;
}

///
/// \brief 虚拟套接字事件
/// \author albert.xu
/// \date 2017/07/18
///
xgc_ulong OnVirtualSockEvt( CRelaySession*, xgc_uint32, xgc_uint64 )
{
	return 0;
}
