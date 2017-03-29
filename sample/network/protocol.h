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
	xgc_byte	type;
	/// ��Ϣ���
	xgc_byte	code;
};

/// Ping��Ϣ
struct MessagePing
{
	xgc_time64 timestamp;
};

#define SERVER_MESSAGE_TYPE (0)
#define CLIENT_MESSAGE_TYPE (1)

#define FILE_INFO_REQ (0)
#define FILE_INFO_ACK (1)
#define FILE_STREAM_REQ (2)
#define FILE_STREAM_ACK (3)

/// �ļ�����
struct MessageFileInfoReq
{
	xgc_char filepath[XGC_MAX_PATH];
	xgc_char filename[XGC_MAX_FNAME];
};

/// �ļ���������
struct MessageFileStreamReq
{
	xgc_uint32 file_sequence;
	xgc_uint32 file_offset;
};

/// �ļ���Ӧ
struct MessageFileInfoAck
{
	xgc_uint8 error;
	xgc_char filepath[XGC_MAX_PATH];
	xgc_char filename[XGC_MAX_FNAME];

	xgc_uint32 file_sequence;
	xgc_uint64 file_length;
};

/// �ļ���
struct MessageFileStreamAck
{
	xgc_uint32 file_sequence;
	xgc_uint32 file_offset;

	xgc_long file_size;
	xgc_char file[1];
};
#pragma pack()

#endif // _PROTOCOL_H_ 