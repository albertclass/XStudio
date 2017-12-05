#pragma once
#ifndef _SERVER_MESSAGES_H_ 
#define _SERVER_MESSAGES_H_ 
//////////////////////////////////////////////////////////////////////////
///
/// \brief �����������Ϣ
///
/// \author albert.xu
/// \date 2017/03/27 11:26
///
xgc_void OnServerMessage( CNetSession* net, xgc_uint16 code, xgc_lpvoid data, xgc_size size );

///
/// \brief �����ļ�
///
/// \author albert.xu
/// \date 2017/03/24 17:50
///
xgc_void OnFileInfoReq( CNetSession* net, xgc_lpvoid data, xgc_size size );

///
/// \brief 
///
/// \author albert.xu
/// \date 2017/03/24 17:51
///
xgc_void OnFileStreamReq( CNetSession* net, xgc_lpvoid data, xgc_size size );

#endif // _SERVER_MESSAGES_H_ 