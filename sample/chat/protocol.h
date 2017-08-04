#pragma once
#ifndef _PROTOCOL_H_ 
#define _PROTOCOL_H_ 
#pragma pack(1)

/// ��Ϣͷ����
struct MessageHeader
{
	/// ��Ϣ���ܳ���
	xgc_uint16	length;
	/// ��Ϣ����
	xgc_uint16	message;
};

/// Ping��Ϣ
struct MessagePing
{
	xgc_time64 timestamp;
};

#pragma pack()

#endif // _PROTOCOL_H_ 