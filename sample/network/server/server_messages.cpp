#include "../header.h"
#include "../protocol.h"
#include "../session.h"

#include "server_messages.h"
#include "server_files.h"

xgc_void OnMessage( CNetSession* net, xgc_uint8 type, xgc_uint8 code, xgc_lpvoid data, xgc_size size )
{
	switch( type )
	{
	case SERVER_MESSAGE_TYPE:
		OnServerMessage( net, code, data, size );
		break;
	}
}

xgc_void OnEvent( CNetSession* net, xgc_uint32 event, xgc_uint32 code )
{
}

xgc_void OnServerMessage( CNetSession* net, xgc_uint8 code, xgc_lpvoid data, xgc_size size )
{
	switch( code )
	{
	case FILE_INFO_REQ:
		OnFileInfoReq( net, data, size );
		break;
	case FILE_STREAM_REQ:
		OnFileStreamReq( net, data, size );
		break;
	}
}

xgc_void OnFileInfoReq( CNetSession * net, xgc_lpvoid data, xgc_size size )
{
	XGC_ASSERT_RETURN( size >= sizeof( MessageFileInfoReq ), XGC_NONE );
	MessageFileInfoReq &req = *(MessageFileInfoReq*) data;

	xgc_uint64 length = 0;
	auto sequence = getServerFiles().GetFileInfo( req.filepath, req.filename, &length );

	MessageHeader header;
	header.length = htons( (xgc_uint16) (sizeof( MessageHeader ) + sizeof( MessageFileInfoAck )) );
	header.type = SERVER_MESSAGE_TYPE;
	header.code = FILE_INFO_ACK;

	MessageFileInfoAck ack;
	if( sequence < 0x80000000U )
	{
		ack.error = sequence;
	}
	else
	{
		ack.error = 0;

		strcpy_s( ack.filepath, req.filepath );
		strcpy_s( ack.filename, req.filename );

		ack.file_sequence = htonl( sequence );
		ack.file_length = htonll( length );
	}

	net->SendPacket( SERVER_MESSAGE_TYPE, FILE_INFO_ACK, &ack, sizeof( ack ) );
}

xgc_void OnFileStreamReq( CNetSession * net, xgc_lpvoid data, xgc_size size )
{
	XGC_ASSERT_RETURN( size >= sizeof( MessageFileStreamReq ), XGC_NONE );
	MessageFileStreamReq &req = *(MessageFileStreamReq*) data;

	xgc_char ack_buffer[1024 * 16];
	MessageFileStreamAck &ack = *(MessageFileStreamAck*)ack_buffer;

	ack.file_sequence = htonl( req.file_sequence );
	ack.file_offset = htonl( req.file_offset );

	auto length = getServerFiles().GetFileData( req.file_sequence, req.file_offset, ack.file, ack.file_size );
	ack.file_size = htonl( length );

	net->SendPacket( SERVER_MESSAGE_TYPE, FILE_STREAM_ACK, &ack, sizeof(ack) + ack.file_size );
}
