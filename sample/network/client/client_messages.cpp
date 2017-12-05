#include "../header.h"
#include "../protocol.h"
#include "../session.h"

#include "client_messages.h"
// 存储目录
extern char root_path[XGC_MAX_PATH];
extern bool running;

#define DOWNLOAD_MAX 64
struct stFileInfo
{
	/// 文件描述符
	int fd;
	/// 文件索引
	xgc_byte id;
	/// 文件路径
	xgc_char path[XGC_MAX_PATH];
	/// 文件名
	xgc_char name[XGC_MAX_FNAME];
	/// 校验码
	xgc_char md5[33];
	/// 文件长度
	xgc_uint64 file_length;
	/// 文件已写字节
	xgc_uint64 file_offset;

	/// 文件序列号
	xgc_uint32 file_sequence;
	/// 时间戳
	xgc_time64 file_timestamp;
	/// 下载完成后事件
	int (*action)( CNetSession* net );
};

typedef xgc_list< xgc_tuple< xgc_string, xgc_string > > download_queue;

struct stUserInfo
{
	stFileInfo files[DOWNLOAD_MAX];

	xgc_byte busy;
	xgc_byte idle[DOWNLOAD_MAX];

	download_queue *dl_queue;
};

///
/// \brief 开始下载文件
///
/// \author albert.xu
/// \date 2017/04/01 15:53
///
int download_start( CNetSession* net, xgc_lpcstr fullname, xgc_lpcstr md5, int (*action)( CNetSession* net ) )
{
	auto user = (stUserInfo*)net->GetUserdata();
	if( user->busy == XGC_COUNTOF( user->idle ) )
	{
		fprintf( stderr, "file download thread was full." );
		return -1;
	}

	// get a free file struct.
	auto idle = user->idle[user->busy];
	stFileInfo &file = user->files[idle];

	file.fd = -1;

	file.file_length = 0;
	file.file_offset = 0;
	file.file_sequence  = 0;
	file.file_timestamp = ticks< std::chrono::milliseconds >();
	
	if( md5 )
	{
		strcpy_s( file.md5, md5 );
	}
	else
	{
		file.md5[0] = '\0';
	}

	path_dirs( file.path, fullname );
	path_name( file.name, fullname );

	file.action = action;
	++user->busy;

	MessageFileInfoReq req;

	req.file_id = idle;
	strcpy_s( req.filepath, file.path );
	strcpy_s( req.filename, file.name );

	SendPacket( net, SERVER_MESSAGE_TYPE, FILE_INFO_REQ, req );

	return idle;
};

///
/// \brief 从文件创建下载队列
///
/// \author albert.xu
/// \date 2017/04/01 15:29
///
int download_build( CNetSession* net )
{
	auto user = (stUserInfo*)net->GetUserdata();

	xgc_char filelist[XGC_MAX_PATH] = { 0 };
	auto path = get_absolute_path( filelist, "%s/filelist", root_path );
	XGC_ASSERT_RETURN( path, -1 );

	#ifdef _WINDOWS
	int fd = _open( path, O_RDONLY | O_BINARY, S_IREAD );
	#endif

	#ifdef _LINUX
	int fd = _open( path, O_RDONLY, 0 );
	#endif
	
	XGC_ASSERT_RETURN( fd != -1, -2 );

	struct _stat s;
	int rl = _fstat( fd, &s );
	XGC_ASSERT_RETURN( rl != -1, -3 );

	auto buffer = malloc( s.st_size );
	if( xgc_nullptr == buffer )
	{
		_close( fd );
		return -4;
	}

	int re = 5;
	int total_bytes = 0;
	while( total_bytes < s.st_size && re )
	{
		rl = _read( fd, (char*) buffer + total_bytes, s.st_size - total_bytes );

		if( rl < 0 )
			--re;
		else
			total_bytes += rl;
	}

	_close( fd );

	if( re == 0 )
		return -5;

	// read every line and split
	auto lines = string_split( (xgc_lpcstr) buffer, "\r\n" );
	for( auto line : lines )
	{
		auto tokens = string_split( line.c_str(), ":" );
		if( tokens.size() != 2 )
		{
			fprintf( stderr, "line is not valid! line - %s\n", line.c_str() );
			continue;
		}

		user->dl_queue->push_back( std::make_tuple( tokens[0], tokens[1] ) );
	}

	return 0;
}

///
/// \brief 下载队列中的文件
///
/// \author albert.xu
/// \date 2017/04/01 15:30
///
int download_front( CNetSession* net )
{
	auto user = (stUserInfo*) net->GetUserdata();
	XGC_ASSERT_RETURN( user && user->dl_queue, -1 );

	if( user->dl_queue->empty() )
		return 1;

	auto file = user->dl_queue->front();
	auto text = std::get<0>( file );
	auto full = std::get<1>( file );

	download_start( net, full.c_str(), text.c_str(), xgc_nullptr );
	user->dl_queue->pop_front();
	return 0;
}

///
/// \brief 检查下载是否成功
///
/// \author albert.xu
/// \date 2017/04/01 15:48
///
int download_check( CNetSession* net, xgc_byte file_id )
{
	XGC_ASSERT_RETURN( file_id < XGC_COUNTOF( stUserInfo::files ), -1 );

	auto user = (stUserInfo*) net->GetUserdata();
	auto &file = user->files[file_id];

	if( file.md5[0] )
	{
		// check md5 code.
		xgc_char fullname[1024] = { 0 };
		auto path = get_absolute_path( fullname, "%s/%s/%s", root_path, file.path, file.name );
		XGC_ASSERT_RETURN( path, -1 );

		xgc_char md5[16];
		if( false == Encryption::Md5_file( path, md5 ) )
			return -2;

		xgc_char md5_text[33];
		Encryption::Md5_text( md5, md5_text );

		if( strcasecmp( file.md5, md5_text ) != 0 )
			return -3;
	}

	return file.action ? file.action( net ) : 0;
}

///
/// \brief 重新下载
///
/// \author albert.xu
/// \date 2017/04/01 15:48
///
int download_again( CNetSession* net, xgc_byte file_id )
{
	XGC_ASSERT_RETURN( file_id < XGC_COUNTOF( stUserInfo::files ), -1 );

	auto user = (stUserInfo*) net->GetUserdata();

	// get a free file struct.
	auto idle = user->idle[file_id];
	stFileInfo &file = user->files[idle];

	_close( file.fd );

	file.fd = -1;
	file.file_length = 0;
	file.file_offset = 0;
	file.file_sequence = 0;
	file.file_timestamp = ticks< std::chrono::milliseconds >();

	MessageFileInfoReq req;

	req.file_id = idle;
	strcpy_s( req.filepath, file.path );
	strcpy_s( req.filename, file.name );

	SendPacket( net, SERVER_MESSAGE_TYPE, FILE_INFO_REQ, req );

	return 0;
}

///
/// \brief 关闭下载
///
/// \author albert.xu
/// \date 2017/04/01 16:20
///
int download_close( CNetSession* net, xgc_byte file_id )
{
	XGC_ASSERT_RETURN( file_id < XGC_COUNTOF( stUserInfo::files ), -1 );

	auto user = (stUserInfo*) net->GetUserdata();
	auto &file = user->files[file_id];

	_close( file.fd );

	file.fd = -1;
	file.path[0] = '\0';
	file.name[0] = '\0';
	file.md5[0] = '\0';
	file.file_length = 0;
	file.file_offset = 0;
	file.file_sequence = 0;
	file.file_timestamp = 0;
	file.action = xgc_nullptr;

	--user->busy;

	for( decltype(user->busy) slot = 0; slot < user->busy; ++slot )
	{
		if( user->idle[slot] != file_id )
			continue;
		
		std::swap( user->idle[slot], user->idle[user->busy] );
		break;
	}

	return 0;
}

xgc_void OnMessage( CNetSession* net, xgc_lpvoid data, xgc_size size )
{
	MessageHeader* pHeader = (MessageHeader*)data;
	auto type = pHeader->type;
	auto code = pHeader->code;

	auto pstr = (char*)data;

	pstr += sizeof MessageHeader;
	size -= sizeof MessageHeader;

	switch( type )
	{
	case CLIENT_MESSAGE_TYPE:
		OnClientMessage( net, code, pstr, size );
		break;
	}
}

xgc_void OnEvent( CNetSession* net, xgc_uint32 event, xgc_uint64 code )
{
	switch( event )
	{
	case EVENT_CONNECT:
		{
			auto user = (stUserInfo*)malloc( sizeof(stUserInfo) );
			memset( user, 0, memsize(user) );

			// 初始化空闲列表
			for( xgc_size i = 0; i < XGC_COUNTOF( user->idle ); ++i )
			{
				user->files[i].id = user->idle[i] = (xgc_byte)i;
				user->files[i].fd = -1;
			}

			user->dl_queue = XGC_NEW download_queue();

			// 设置用户数据
			net->SetUserdata( user );

			download_start( net, "/filelist", "", download_build );
		}
		break;
	case EVENT_ERROR:
		{
			switch( code )
			{
			case NET_ERROR_TIMEOUT:
				SAFE_DELETE( net );
				break;
			}
		}
		break;
	case EVENT_CLOSE:
		{
			fprintf( stderr, "connection %u disconnect.\n", net->GetHandle() );
			auto user = (stUserInfo*) net->GetUserdata();
			if( user )
			{
				for( xgc_size i = 0; i < user->busy; ++i )
				{
					auto slot = user->idle[i];
					auto &file = user->files[slot];

					if( file.fd != -1 )
					{
						_close(file.fd);
						file.fd = -1;
					}
				}

				SAFE_DELETE( user->dl_queue );
				free( user );
			}

			SAFE_DELETE( net );
		}
		break;
	}
}

xgc_void OnClientMessage( CNetSession* net, xgc_uint16 code, xgc_lpvoid data, xgc_size size )
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
	if( ack.error != 0 )
	{
		fprintf( stderr, "request file %s%s error, code = %u\r\n", ack.filepath, ack.filename, ack.error );
		return;
	}

	auto file_length = ntohll( ack.file_length );
	auto file_sequence = ntohl( ack.file_sequence );

	XGC_ASSERT( file_length );

	xgc_char path[XGC_MAX_PATH] = { 0 };
	auto ret = get_absolute_path( path, "%s/%s", root_path, ack.filepath );
	XGC_ASSERT_RETURN( ret, XGC_NONE );
	
	if( _access( path, 6 ) != 0 )
	{
		make_dirs( path, true );
		if( _access( path, 6 ) != 0 )
		{
			fprintf( stderr, "file %s cannot open.\n", path );
			return;
		}
	}

	ret = get_absolute_path( path, "%s/%s/%s", root_path, ack.filepath, ack.filename );
	XGC_ASSERT_RETURN( ret, XGC_NONE );

	auto user = (stUserInfo*) net->GetUserdata();

	// check file is already in download status
	for( xgc_size i = 0; i < user->busy; ++i )
	{
		auto &file = user->files[user->idle[i]];
		if( file.file_sequence == ack.file_sequence )
		{
			fprintf( stderr, "file is in download queue. %s/%s - %u\n", file.path, file.name, file.file_sequence );
			download_close( net, ack.file_id );
			return;
		}
	}

	stFileInfo &file = user->files[ack.file_id];
	if( strcmp( file.path, ack.filepath ) != 0 )
	{
		fprintf( stderr, "file path mistake [%s != %s]\n", file.path, ack.filepath );
		download_close( net, ack.file_id );
		return;
	}

	if( strcmp( file.name, ack.filename ) != 0 )
	{
		fprintf( stderr, "file name mistake [%s != %s]\n", file.name, ack.filename );
		download_close( net, ack.file_id );
		return;
	}

	#ifdef _WINDOWS
	int fd = _open( path, O_RDWR | O_TRUNC | O_CREAT | O_BINARY, S_IWRITE | S_IREAD );
	#endif

	#ifdef _LINUX
	int fd = _open( path, O_RDWR | O_TRUNC | O_CREAT , 0666 );
	#endif

	if( fd == -1 )
	{
		fprintf( stderr, "file open failed. err = %d\n", errno );
		return;
	}

	file.fd = fd;
	
	file.file_length = file_length;
	file.file_offset = 0;
	file.file_sequence = file_sequence;
	file.file_timestamp = ticks< std::chrono::milliseconds >();

	MessageFileStreamReq req;
	req.file_id = ack.file_id;
	req.file_sequence = ack.file_sequence;
	req.file_offset = 0;

	SendPacket( net, SERVER_MESSAGE_TYPE, FILE_STREAM_REQ, req );
}

xgc_void OnFileStreamAck( CNetSession* net, xgc_lpvoid data, xgc_size size )
{
	XGC_ASSERT_RETURN( size >= sizeof( MessageFileStreamAck ), XGC_NONE );

	MessageFileStreamAck &ack = *(MessageFileStreamAck*) data;
	auto file_id       = ack.file_id;
	auto file_sequence = ntohl( ack.file_sequence );
	auto file_offset   = ntohll( ack.file_offset );
	auto data_size     = ntohl( ack.data_size );

	auto user = (stUserInfo*)net->GetUserdata();

	auto &file = user->files[ack.file_id];

	if( file.file_sequence != file_sequence )
	{
		fprintf( stderr, "error file sequence %08X != %08X\n", file.file_sequence, file_sequence );
		return;
	}

	if( file.file_offset != file_offset )
	{
		fprintf( stderr, "error file offset %llu != %llu\n", file.file_offset, file_offset );
		download_close( net, ack.file_id );
		return;
	}

	// update time stamp.
	file.file_timestamp = ticks< std::chrono::milliseconds >();

	// write data to file
	xgc_ulong bytes = 0;
	while( bytes < data_size )
	{
		int ret = _write( file.fd, ack.data, (int)data_size );
		if( ret < 0 )
		{
			// file download error
			fprintf( stderr, "write file error.\n" );
			download_close( net, ack.file_id );
			return;
		}

		bytes += ret;
	}

	// update file download bytes and progress
	file.file_offset += bytes;

	if( file.file_offset >= file.file_length )
	{
		// download finished
		fprintf( stdout, "file %s/%s - %u download successful!\n", file.path, file.name, file.file_sequence );

		// check file is correct.
		if( 0 > download_check( net, file.id ) )
		{
			download_again( net, file.id );
		}
		else
		{
			download_close( net, file.id );
			if( 1 == download_front( net ) )
				running = false;
		}
	}
	else
	{
		// download going on
		MessageFileStreamReq req;
		req.file_id = ack.file_id;
		req.file_offset = htonll( file.file_offset );
		req.file_sequence = ack.file_sequence;

		SendPacket( net, SERVER_MESSAGE_TYPE, FILE_STREAM_REQ, req );
	}
}