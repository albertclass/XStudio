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
	xgc_uint16	type : 4;
	/// ��Ϣ���
	xgc_uint16	code : 12;
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
	/// ��������
	xgc_byte file_id;
	/// �ļ�·��
	xgc_char filepath[XGC_MAX_PATH];
	/// �ļ���
	xgc_char filename[XGC_MAX_FNAME];
};

/// �ļ���������
struct MessageFileStreamReq
{
	/// ��������
	xgc_byte file_id;
	/// �ļ����к�
	xgc_uint32 file_sequence;
	/// �����ƫ����
	xgc_uint64 file_offset;
};

/// �ļ���Ӧ
struct MessageFileInfoAck
{
	/// ������
	xgc_uint8 error;
	/// ��������
	xgc_byte file_id;
	/// �ļ�·��
	xgc_char filepath[XGC_MAX_PATH];
	/// �ļ���
	xgc_char filename[XGC_MAX_FNAME];

	/// �ļ����к�
	xgc_uint32 file_sequence;
	/// �ļ���С
	xgc_uint64 file_length;
};

/// �ļ���
struct MessageFileStreamAck
{
	/// ��������
	xgc_byte file_id;
	/// �ļ����к�
	xgc_uint32 file_sequence;
	/// �����ƫ����
	xgc_uint64 file_offset;

	/// ���ݴ�С
	xgc_long data_size;
	xgc_char data[1];
};
#pragma pack()

#endif // _PROTOCOL_H_ 