#include "header.h"
#include "game_message.h"

///
/// \brief 管道消息
/// \author albert.xu
/// \date 2017/07/18
///
xgc_void OnPipeMsg( CPipeSession* pSession, xgc_lpvoid pData, xgc_size nSize, xgc_uint32 hHandle )
{
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
xgc_void OnPipeEvt( CPipeSession* pSession, xgc_uint32 nEvent, xgc_uint64 nBring )
{
	xgc_char szNetworkID[64] = { 0 };

	switch( nEvent )
	{
	case EVENT_CONNECT:
		DBG_INFO( "Pipe connected %p - %s", pSession, NetworkId2Str( nBring, szNetworkID ) );
		break;
	case EVENT_ERROR:
		OnPipeEvtError( pSession, nBring );
		break;
	case EVENT_CLOSE:
		DBG_INFO( "Pipe disconnect %p - %s", pSession, NetworkId2Str( nBring, szNetworkID ) );
		break;
	}
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
xgc_void OnVirtualSockMsg( CRelaySession*, xgc_lpvoid, xgc_size, xgc_uint32 )
{
}

///
/// \brief 虚拟套接字事件
/// \author albert.xu
/// \date 2017/07/18
///
xgc_void OnVirtualSockEvt( CRelaySession*, xgc_uint32, xgc_uint64 )
{
}
