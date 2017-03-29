#include "header.h"
#include "server_files.h"
extern xgc_char root_path[XGC_MAX_PATH];

CServerFiles::CServerFiles()
	: sequence_( 0x80000000 )
{
}

CServerFiles::~CServerFiles()
{
}

xgc_void CServerFiles::GenFileList(xgc_lpcstr root)
{
	
}

xgc_uint32 CServerFiles::GetFileInfo( xgc_lpcstr path, xgc_lpcstr name, xgc_uint64 *length )
{
	xgc_char pathname[1024];
	sprintf_s( pathname, "%s/%s", path, name );

	auto it1 = files_.find( pathname );
	if( it1 != files_.end() )
	{
		return it1->second;
	}

	if( 0 != _access( pathname, 04 ) )
	{
		return 1;
	}

	++sequence_;

	auto sequence = XGC_ADD_FLAGS( sequence_, 0x80000000 );
	auto it2 = files_seq_.find( sequence );
	if( it2 == files_seq_.end() )
	{
		return 2;
	}

	// open file

	struct stat s;
	memset( &s, 0, sizeof(s) );

	if( 0 != stat( pathname, &s ) )
		return 3;

	int fd = _open( pathname, O_RDONLY, S_IREAD );

	if( fd == -1 )
	{
		return 4;
	}

	fileinfo* info = xgc_nullptr;

	if( s.st_size <= 1024 * 1024 )
	{
		info = (fileinfo*) malloc( s.st_size + sizeof( fileinfo ) );
		if( xgc_nullptr == info )
			return 5;

		memset( info, 0, memsize( info ) );

		// file size less 1M, cache it
		info->fd = -1;

		int read_total = 0;
		int read_try = 5;
		while( read_total < s.st_size )
		{
			auto read_bytes = _read( fd, info->file + read_total, (unsigned int) info->size - read_total );
			if( read_bytes < 0 )
			{
				if( --read_try == 0 )
					break;

				std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
				continue;
			}
			else
			{
				read_total += read_bytes;
			}
		}

		_close( fd );

		if( read_try == 0 )
		{
			free( info );
			return 6;
		}
	}
	else
	{
		info = (fileinfo*) malloc( sizeof( fileinfo ) );
		if( xgc_nullptr == info )
			return 5;

		memset( info, 0, memsize( info ) );

		// file greater 1M, use file descriptor,
		info->fd = fd;
	}

	strcpy_s( info->path, path );
	strcpy_s( info->name, name );

	info->sequence = sequence;
	info->size = s.st_size;

	auto ib1 = files_.insert( std::make_pair( pathname, sequence ) );
	if( false == ib1.second )
	{
		free( info );
		return 7;
	}

	auto ib2 = files_seq_.insert( std::make_pair( sequence, info ) );
	if( false == ib2.second )
	{
		free( info );
		return 8;
	}

	if( length )
	{
		*length = info->size;
	}

	// 打开文件
	return sequence;
}

xgc_int32 CServerFiles::GetFileData( xgc_uint32 sequence, xgc_size offset, xgc_lpvoid buffer, xgc_long length )
{
	auto it = files_seq_.find( sequence );
	if( it == files_seq_.end() )
		return -1;

	auto info = it->second;
	XGC_ASSERT_RETURN( info, -2 );
	XGC_ASSERT_RETURN( offset < info->size, 0 );

	auto bytes = XGC_MIN( (xgc_int32)(info->size - offset), length );
	if( info->fd == -1 )
	{
		memcpy( buffer, info->file + offset, bytes );
	}
	else
	{
		_lseek( info->fd, (long)offset, SEEK_SET );
		bytes = _read( info->fd, buffer, bytes );
	}

	return bytes;
}

CServerFiles & getServerFiles()
{
	static CServerFiles inst;
	return inst;
}
