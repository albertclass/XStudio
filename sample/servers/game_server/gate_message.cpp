#include "header.h"
#include "gate_message.h"

///
/// \brief �ܵ���Ϣ
/// \author albert.xu
/// \date 2017/07/18
///
xgc_ulong OnPipeMsg( CPipeSession* pSession, xgc_lpvoid pData, xgc_size nSize, xgc_uint32 hHandle )
{
	return 0;
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
		break;
	}
	return 0;
}

///
/// \brief �����׽�����Ϣ
/// \author albert.xu
/// \date 2017/07/18
///
xgc_ulong OnVirtualSockMsg( CRelaySession*, xgc_lpvoid, xgc_size, xgc_uint32 )
{
	return 0;
}

///
/// \brief �����׽����¼�
/// \author albert.xu
/// \date 2017/07/18
///
xgc_ulong OnVirtualSockEvt( CRelaySession*, xgc_uint32, xgc_uint64 )
{
	return 0;
}
