#pragma once
#ifndef _GATE_MESSAGE_H_
#define _GATE_MESSAGE_H_
///
/// \brief 管道消息
/// \author albert.xu
/// \date 2017/07/18
///
xgc_ulong OnPipeMsg( CPipeSession*, xgc_lpvoid, xgc_size, xgc_uint32 );

///
/// \brief 管道事件
/// \author albert.xu
/// \date 2017/07/18
///
xgc_ulong OnPipeEvt( CPipeSession*, xgc_uint32, xgc_uint64 );

///
/// \brief 虚拟套接字消息
/// \author albert.xu
/// \date 2017/07/18
///
xgc_ulong OnVirtualSockMsg( CRelaySession*, xgc_lpvoid, xgc_size, xgc_uint32 );

///
/// \brief 虚拟套接字事件
/// \author albert.xu
/// \date 2017/07/18
///
xgc_ulong OnVirtualSockEvt( CRelaySession*, xgc_uint32, xgc_uint64 );

#endif // _GATE_MESSAGE_H_