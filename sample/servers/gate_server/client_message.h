#pragma once
#ifndef _MESSAGE_H_
#define _MESSAGE_H_

///
/// \brief �¼��ص�
/// \author albert.xu
/// \date 2017/08/01
///
xgc_void OnClientEvt( CClientSession* net, xgc_uint32 event, xgc_uint64 code );

///
/// \brief ��Ϣ�ص�
/// \author albert.xu
/// \date 2017/08/01
///
xgc_void OnClientMsg( CClientSession* net, xgc_lpvoid data, xgc_size size, xgc_uint32 trans );

///
/// \brief ���͵� GateServer ��Ϣ����Ϣ����
/// \author albert.xu
/// \date 2017/08/01
///
xgc_void OnGateMessage( CClientSession* net, xgc_uint8 code, xgc_lpvoid data, xgc_size size );

///
/// \brief ���͵� GameServer ��Ϣ����Ϣ����
/// \author albert.xu
/// \date 2017/08/01
///
xgc_void OnGameMessage( CClientSession* net, xgc_uint8 code, xgc_lpvoid data, xgc_size size );

#endif // _MESSAGE_H_