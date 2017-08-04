#pragma once
#ifndef _PROTOCOL_H_ 
#define _PROTOCOL_H_ 

#define NET_GATE_MESSAGE 0
#define NET_GAME_MESSAGE 1

#pragma pack(1)

/// ��Ϣͷ����
struct MessageHeader
{
	/// ��Ϣ���ܳ���
	xgc_uint16	length;
	/// ��Ϣ����
	xgc_uint16	type : 3; // 0 ~ 7
	/// ��Ϣ���
	xgc_uint16	code : 13;
};

/// Ping��Ϣ
struct MessagePing
{
	xgc_time64 timestamp;
};

#pragma pack()

#endif // _PROTOCOL_H_ 