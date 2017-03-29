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
	xgc_byte	type;
	/// 消息编号
	xgc_byte	code;
};

/// Ping消息
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

/// 文件请求
struct MessageFileInfoReq
{
	xgc_char filepath[XGC_MAX_PATH];
	xgc_char filename[XGC_MAX_FNAME];
};

/// 文件数据请求
struct MessageFileStreamReq
{
	xgc_uint32 file_sequence;
	xgc_uint32 file_offset;
};

/// 文件回应
struct MessageFileInfoAck
{
	xgc_uint8 error;
	xgc_char filepath[XGC_MAX_PATH];
	xgc_char filename[XGC_MAX_FNAME];

	xgc_uint32 file_sequence;
	xgc_uint64 file_length;
};

/// 文件流
struct MessageFileStreamAck
{
	xgc_uint32 file_sequence;
	xgc_uint32 file_offset;

	xgc_long file_size;
	xgc_char file[1];
};
#pragma pack()

#endif // _PROTOCOL_H_ 