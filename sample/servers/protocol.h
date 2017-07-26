#pragma once
#ifndef _PROTOCOL_H_ 
#define _PROTOCOL_H_ 
#define NET_GATE_MESSAGE 1
#define NET_GAME_MESSAGE 2

#pragma pack(1)

/// ��Ϣͷ����
struct MessageHeader
{
	/// ��Ϣ���ܳ���
	xgc_uint16	length;
	/// ��Ϣ����
	xgc_byte	type;
	/// ��Ϣ���
	xgc_byte	code;
};

/// Ping��Ϣ
struct MessagePing
{
	xgc_time64 timestamp;
};

#pragma pack()

#endif // _PROTOCOL_H_ 