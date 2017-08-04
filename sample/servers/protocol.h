#pragma once
#ifndef _PROTOCOL_H_ 
#define _PROTOCOL_H_ 

#define NET_GATE_MESSAGE 0
#define NET_GAME_MESSAGE 1

#pragma pack(1)

/// 消息头定义
struct MessageHeader
{
	/// 消息包总长度
	xgc_uint16	length;
	/// 消息类型
	xgc_uint16	type : 3; // 0 ~ 7
	/// 消息编号
	xgc_uint16	code : 13;
};

/// Ping消息
struct MessagePing
{
	xgc_time64 timestamp;
};

#pragma pack()

#endif // _PROTOCOL_H_ 