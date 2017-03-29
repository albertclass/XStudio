#include "../header.h"
#include "../protocol.h"
#include "../session.h"

#include "client_messages.h"
// 存储目录
extern char root_path[XGC_MAX_PATH];

#define DOWNLOAD_MAX 64
struct stFileInfo
{
	/// 文件描述符
	int fd;
	/// 文件路径
	xgc_char path[XGC_MAX_PATH];
	/// 文件名
	xgc_char name[XGC_MAX_FNAME];
	/// 文件长度
	xgc_size file_length;
	/// 文件已写字节
	xgc_size file_offset;

	/// 文件序列号
	xgc_uint32 file_sequence;
};

struct stUserInfo
{
	stFileInfo files[DOWNLOAD_MAX];

	xgc_size busy;
	xgc_size idle[DOWNLOAD_MAX];
};

xgc_void OnMessage( CNetSession* net, xgc_uint8 type, xgc_uint8 code, xgc_lpvoid data, xgc_size size )
{
	switch( type )
	{
	case CLIENT_MESSAGE_TYPE:
		OnClientMessage( net, code, data, size );
		break;
	}
}

xgc_void OnEvent( CNetSession* net, xgc_uint32 event, xgc_uint32 code )
{
	switch( event )
	{
	case EVENT_CONNECT:
		{
			auto user = (stUserInfo*)malloc( sizeof(stUserInfo) );
			memset( user, 0, memsize(user) );

			// 初始化空闲列表
			for( xgc_size i = 0; i < XGC_COUNTOF( user->idle ); ++i )
				user->idle[i] = i;

			// 设置用户数据
			net->SetUserdata( user );

			MessageFileInfoReq req;
			strcpy_s( req.filepath, "/" );
			strcpy_s( req.filename, "filelist" );

			net->SendPacket( SERVER_MESSAGE_TYPE, FILE_INFO_REQ, &req, sizeof( req ) );
		}
		break;
	case EVENT_CLOSE:
		{
			auto user = (stUserInfo*) net->GetUserdata();
			if( user )
			{
				for( xgc_size i = 0; i < XGC_COUNTOF( user->files ); ++i )
				{
					stFileInfo &file = user->files[i];
					if( file.fd != -1 )
					{
						_close(file.fd);
						file.fd = -1;
					}
				}

				free( user );
			}
		}
		break;
	}
}

xgc_void OnClientMessage( CNetSession* net, xgc_uint8 code, xgc_lpvoid data, xgc_size size )
{
	switch( code )
	{
	case FILE_INFO_ACK:
		OnFileInfoAck( net, data, size );
		break;
	case FILE_STREAM_ACK:
		OnFileStreamAck( net, data, size );
		break;
	}
}

xgc_void OnFileInfoAck( CNetSession * net, xgc_lpvoid data, xgc_size size )
{
	XGC_ASSERT_RETURN( size >= sizeof( MessageFileInfoAck ), XGC_NONE );

	MessageFileInfoAck &ack = *(MessageFileInfoAck*)data;
	auto file_length = ntohll( ack.file_length );
	auto file_sequence = ntohl( ack.file_sequence );

	xgc_char pathname[1024];
	get_normal_path( pathname, "%s/%s/%s", root_path, ack.filepath, ack.filename );
	
	if( _access( pathname, 6 ) != 0 )
	{
		fprintf( stderr, "file %s cannot open.", pathname );
		return;
	}

	auto user = (stUserInfo*) net->GetUserdata();
	if( user->busy == XGC_COUNTOF( user->idle ) )
	{
		fprintf( stderr, "file download thread was full." );
		return;
	}

	for( xgc_size i = 0; i < user->busy; ++i )
	{
		auto &file = user->files[user->idle[i]];
		if( file.file_sequence == ack.file_sequence )
		{
			fprintf( stderr, "file is in download queue. %s/%s - %u", file.path, file.name, file.file_sequence );
		}
	}

	int fd = _open( pathname, O_WRONLY, S_IWRITE );
	if( fd == -1 )
	{
		fprintf( stderr, "file open failed. err = %d", errno );
		return;
	}

	auto idle = user->idle[user->busy];
	stFileInfo &file = user->files[idle];

	file.fd = fd;
	
	file.file_length = file_length;
	file.file_offset = 0;
	file.file_sequence = file_sequence;

	strcpy_s( file.path, ack.filepath );
	strcpy_s( file.name, ack.filename );

	++user->busy;
}

xgc_void OnFileStreamAck( CNetSession* net, xgc_lpvoid data, xgc_size size )
{
	XGC_ASSERT_RETURN( size >= sizeof( MessageFileStreamAck ), XGC_NONE );

	MessageFileStreamAck &ack = *(MessageFileStreamAck*) data;
	auto file_sequence = htonl( ack.file_sequence );
	auto file_offset = htonl( ack.file_offset );
	auto file_size = htonl( ack.file_size );
	auto user = (stUserInfo*)net->GetUserdata();
	for( xgc_size i = 0; i < user->busy; ++i )
	{
		auto &file = user->files[user->idle[i]];
		if( file.file_sequence != ack.file_sequence )
			continue;
		
		XGC_ASSERT_RETURN( file.file_offset == ack.file_offset, XGC_NONE );

		xgc_long bytes = 0;
		while( bytes < ack.file_size )
		{
			int ret = _write( file.fd, ack.file, ack.file_size );
			if( ret < 0 )
			{
				// file download error
				fprintf( stderr, "write file error." );
				_close( file.fd );

				file.fd = -1;
				file.file_length = 0;
				file.file_offset = 0;
				file.file_sequence = 0;
				file.path[0] = '\0';
				file.name[0] = '\0';

				--user->busy;
				std::swap( user->idle[i], user->idle[user->busy] );
				return;
			}

			bytes += ret;
		}

		file.file_offset += bytes;
		if( file.file_offset == file.file_length )
		{
			// file download finished
			fprintf( stdout, "file %s/%s - %u download successful!", file.path, file.name, file.file_sequence );
			_close( file.fd );

			file.fd = -1;
			file.file_length = 0;
			file.file_offset = 0;
			file.file_sequence = 0;
			file.path[0] = '\0';
			file.name[0] = '\0';

			--user->busy;
			std::swap( user->idle[i], user->idle[user->busy] );
		}
	}
}
