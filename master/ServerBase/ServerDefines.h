#ifndef _SERVER_DEFINES_H_
#define _SERVER_DEFINES_H_

#define SESSION_EVENT_CONNECTED		(0U)
#define SESSION_EVENT_DISCONNECT	(1U)
#define SESSION_EVENT_ERROR			(2U)

typedef class CPipeSession CPipeSession;
typedef class CGameSession CGameSession;
typedef class CGateSession CGateSession;

///
/// 消息回调
/// [11/27/2014] create by albert.xu
///
typedef xgc_void( *PipeMsgHandler )( CPipeSession *, xgc_uint16, xgc_lpcstr, xgc_size, xgc_uint32 );
typedef xgc_void( *GameMsgHandler )( CGameSession *, xgc_uint16, xgc_lpcstr, xgc_size, xgc_uint32 );
typedef xgc_void( *GateMsgHandler )( CGateSession *, xgc_uint16, xgc_lpcstr, xgc_size, xgc_uint32 );

///
/// 事件回调
/// [11/27/2014] create by albert.xu
///
typedef xgc_void( *PipeEvtHandler )( CPipeSession *, xgc_uint16, xgc_uint32 );
typedef xgc_void( *GameEvtHandler )( CGameSession *, xgc_uint16, xgc_uint32 );
typedef xgc_void( *GateEvtHandler )( CGateSession *, xgc_uint16, xgc_uint32 );


#endif _SERVER_DEFINES_H_