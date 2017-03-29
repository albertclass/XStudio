#pragma once
#ifndef _CLIENT_MESSGAES_H_ 
#define _CLIENT_MESSGAES_H_ 
//////////////////////////////////////////////////////////////////////////
///
/// \brief 客户端消息
///
/// \author albert.xu
/// \date 2017/03/27 11:24
///
xgc_void OnClientMessage( CNetSession* net, xgc_uint8 code, xgc_lpvoid data, xgc_size size );

///
/// \brief 
///
/// \author albert.xu
/// \date 2017/03/24 17:51
///
xgc_void OnFileInfoAck( CNetSession* net, xgc_lpvoid data, xgc_size size );

///
/// \brief 
///
/// \author albert.xu
/// \date 2017/03/24 17:51
///
xgc_void OnFileStreamAck( CNetSession* net, xgc_lpvoid data, xgc_size size );

#endif // _CLIENT_MESSGAES_H_ 