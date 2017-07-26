#pragma once
#ifndef _PROTOCOL_H_ 
#define _PROTOCOL_H_ 
#define NET_GATE_MESSAGE 1
#define NET_GAME_MESSAGE 2

#pragma pack(1)

/// 消息头定义
struct MessageHeader
{
	/// 消息包总长度
	xgc_uint16	length;
	/// 消息类型
	xgc_byte	type;
	/// 消息编号
	xgc_byte	code;
};

/// Ping消息
struct MessagePing
{
	xgc_time64 timestamp;
};

#pragma pack()

#endif // _PROTOCOL_H_ 