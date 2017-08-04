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
	xgc_uint16	type : 4;
	/// 消息编号
	xgc_uint16	code : 12;
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
	/// 下载索引
	xgc_byte file_id;
	/// 文件路径
	xgc_char filepath[XGC_MAX_PATH];
	/// 文件名
	xgc_char filename[XGC_MAX_FNAME];
};

/// 文件数据请求
struct MessageFileStreamReq
{
	/// 下载索引
	xgc_byte file_id;
	/// 文件序列号
	xgc_uint32 file_sequence;
	/// 请求的偏移量
	xgc_uint64 file_offset;
};

/// 文件回应
struct MessageFileInfoAck
{
	/// 错误码
	xgc_uint8 error;
	/// 下载索引
	xgc_byte file_id;
	/// 文件路径
	xgc_char filepath[XGC_MAX_PATH];
	/// 文件名
	xgc_char filename[XGC_MAX_FNAME];

	/// 文件序列号
	xgc_uint32 file_sequence;
	/// 文件大小
	xgc_uint64 file_length;
};

/// 文件流
struct MessageFileStreamAck
{
	/// 下载索引
	xgc_byte file_id;
	/// 文件序列号
	xgc_uint32 file_sequence;
	/// 请求的偏移量
	xgc_uint64 file_offset;

	/// 数据大小
	xgc_long data_size;
	xgc_char data[1];
};
#pragma pack()

#endif // _PROTOCOL_H_ 