#pragma once
#ifndef _PROTOCOL_H_ 
#define _PROTOCOL_H_ 
#pragma pack(1)

/// 消息头定义
struct MessageHeader
{
	/// 消息包总长度
	xgc_uint16	length;
	/// 消息类型
	xgc_uint16	message;
};

/// Ping消息
struct MessagePing
{
	xgc_time64 timestamp;
};

#pragma pack()

#endif // _PROTOCOL_H_ 