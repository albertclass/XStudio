#pragma once
#ifndef _MESSAGE_H_
#define _MESSAGE_H_

class CNetSession;

///
/// gate message dispatcher.
///
xgc_void OnGateMessage( CNetSession* net, xgc_uint8 code, xgc_lpvoid data, xgc_size size );

///
/// game message dispatcher.
///
xgc_void OnGameMessage( CNetSession* net, xgc_uint8 code, xgc_lpvoid data, xgc_size size );

#endif // _MESSAGE_H_