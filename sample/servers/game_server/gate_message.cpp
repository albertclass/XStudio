#include "header.h"
#include "gate_message.h"

///
/// \brief �ܵ���Ϣ
/// \author albert.xu
/// \date 2017/07/18
///
xgc_void OnPipeMsg( CPipeSession* pSession, xgc_lpvoid pData, xgc_size nSize, xgc_uint32 hHandle )
{
}

///
/// \brief �ܵ�����
/// \author albert.xu
/// \date 2017/07/18
///
xgc_void OnPipeEvtError( CPipeSession* pSession, xgc_uint64 nErrorCode )
{
	SYS_ERROR( "���緢������ nError = %0X", nErrorCode );
}

///
/// \brief �ܵ��¼�
/// \author albert.xu
/// \date 2017/07/18
///
xgc_void OnPipeEvt( CPipeSession* pSession, xgc_uint32 nEvent, xgc_uint64 nBring )
{
	xgc_char szNetworkID[64] = { 0 };

	switch( nEvent )
	{
		case EVENT_ACCEPT:
		DBG_INFO( "Pipe Accept %p - %s", pSession, NetworkId2Str( nBring, szNetworkID ) );
		break;
		case EVENT_ERROR:
		OnPipeEvtError( pSession, nBring );
		break;
		case EVENT_CLOSE:
		DBG_INFO( "Pipe Disonnect %p - %llu", pSession, NetworkId2Str( nBring, szNetworkID ) );
		break;
	}
}

///
/// \brief �����׽�����Ϣ
/// \author albert.xu
/// \date 2017/07/18
///
xgc_void OnVirtualSockMsg( CRelaySession*, xgc_lpvoid, xgc_size, xgc_uint32 )
{
}

///
/// \brief �����׽����¼�
/// \author albert.xu
/// \date 2017/07/18
///
xgc_void OnVirtualSockEvt( CRelaySession* pSession, xgc_uint32 nEvent, xgc_uint64 nBring )
{
	switch( nEvent )
	{
		case EVENT_CONNECT:
		DBG_INFO( "Virtual sock connect %p - %llu", pSession, nBring );
		break;
		case EVENT_ERROR:
		DBG_INFO( "Virtual sock error %p - %llu", pSession, nBring );
		break;
		case EVENT_CLOSE:
		DBG_INFO( "Virtual sock disonnect %p - %llu", pSession, nBring );
		break;
	}
}
