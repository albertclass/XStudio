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

xgc_long CServerFiles::GenIgnoreList( xgc_lpcstr root )
{
	ignore_files.push_back( "/ignores" );
	ignore_files.push_back( "/filelist" );

	xgc_char absolute[XGC_MAX_PATH] = { 0 };
	get_absolute_path( absolute, "%s/ignores", root );

	FILE *fp = fopen( absolute, "r" );
	if( fp == xgc_nullptr )
		return -1;

	char line[XGC_MAX_PATH + 64];
	while( !feof( fp ) )
	{
		line[0] = 0;
		fgets( line, XGC_COUNTOF(line), fp );
		string_trim_left( line, " " );

		// # is comment
		if( line[0] == '#' )
			continue;

		if( line[0] == '\0' )
			continue;

		string_trim_right( line, "\r\n" );
		ignore_files.push_back( line );
	}

	fclose( fp );
	return 0;
}

xgc_bool CServerFiles::IsIgnoreFile( xgc_lpcstr path, xgc_lpcstr name )
{
	xgc_char relative[XGC_MAX_PATH] = { 0 };

	int cpy = 0;
	if( name )
		cpy = sprintf_s( relative, "%s/%s", path, name );
	else
		cpy = sprintf_s( relative, "%s", path );

	XGC_ASSERT_RETURN( cpy < XGC_COUNTOF(relative), true );

	std::transform( relative, relative + strlen(relative), relative, []( int _Ch ){
		return _Ch == '\\' ? '/' : _Ch;
	} );

	for( auto &str : ignore_files )
	{
		xgc_lpcstr pattern = str.c_str();
		if( string_match( pattern, relative, 1 ) )
			return true;
	}

	return false;
}

xgc_long CServerFiles::GenFileList(xgc_lpcstr root)
{
	xgc_char absolute[XGC_MAX_PATH] = { 0 };
	get_absolute_path( absolute, "%s/filelist", root );

	FILE *fp = fopen( absolute, "w" );
	if( fp == xgc_nullptr )
		return -1;

	list_directory( root, [fp, this]( xgc_lpcstr absolute, xgc_lpcstr relative, xgc_lpcstr name ){

		// file
		if( IsIgnoreFile( relative, name ) )
		{
			if( name )
				fprintf( stdout, "ignore file %s%c%s\n", relative, SLASH, name );
			else
				fprintf( stdout, "ignore directory %s\n", relative );

			return false;
		}

		if( name )
		{
			fprintf( stdout, "generate file %s%c%s", relative, SLASH, name );

			xgc_time64 tick_1 = ticks< std::chrono::microseconds >();

			xgc_char path[XGC_MAX_PATH] = { 0 };
			sprintf_s( path, "%s%c%s", absolute, SLASH, name );

			xgc_char md5[16];
			xgc_char md5_text[33];
			if( encrypt::md5_file( path, md5 ) )
			{
				encrypt::md5_text( md5, md5_text );
				fprintf( fp, "%s:%s%c%s\n", md5_text, relative, SLASH, name );
			}

			xgc_time64 tick_2 = ticks< std::chrono::microseconds >();
			fprintf( stdout, " using %llu microseconds\n", tick_2 - tick_1 );

			return false;
		}

		return true;
	}, 16 );

	fprintf( stdout, "generate file complete!\n" );

	fclose( fp );

	return 0;
}

xgc_uint32 CServerFiles::GetFileInfo( xgc_lpcstr path, xgc_lpcstr name, xgc_uint64 *length )
{
	xgc_char pathname[1024];
	sprintf_s( pathname, "%s/%s", path, name );

	auto it1 = files_.find( pathname );
	if( it1 != files_.end() )
	{
		if( length )
		{
			auto it2 = files_seq_.find( it1->second );
			if( it2 != files_seq_.end() && it2->second )
			{
				*length = it2->second->size;
			}
		}

		return it1->second;
	}

	xgc_char absolute[1024];
	get_absolute_path( absolute, "%s/%s", root_path, pathname );
	if( 0 != _access( absolute, 04 ) )
	{
		return 1;
	}

	++sequence_;

	auto sequence = XGC_ADD_FLAGS( sequence_, 0x80000000 );
	auto it2 = files_seq_.find( sequence );
	if( it2 != files_seq_.end() )
	{
		return 2;
	}

	// open file
	#ifdef _WINDOWS
	int fd = _open( absolute, O_RDONLY | O_BINARY, S_IREAD );
	#endif

	#ifdef _LINUX
	int fd = _open( absolute, O_RDONLY, S_IREAD );
	#endif
	if( fd == -1 )
		return 3;

	struct _stat s;
	memset( &s, 0, sizeof( s ) );

	if( 0 != _fstat( fd, &s ) )
		return 4;

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
			auto read_bytes = _read( fd, info->file + read_total, (unsigned int) s.st_size - read_total );
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

xgc_int32 CServerFiles::GetFileData( xgc_uint32 sequence, xgc_int64 offset, xgc_lpvoid buffer, xgc_int32 length )
{
	auto it = files_seq_.find( sequence );
	if( it == files_seq_.end() )
		return -1;

	auto info = it->second;
	XGC_ASSERT_RETURN( info, -2 );
	XGC_ASSERT_RETURN( offset <= (xgc_int64)info->size, 0 );

	xgc_int32 bytes = XGC_MIN( (xgc_int32)(info->size - offset), length );
	if( info->fd == -1 )
	{
		memcpy( buffer, info->file + offset, bytes );
	}
	else
	{
		if( _tell( info->fd ) != offset )
		{
			_lseek( info->fd, (xgc_long)offset, SEEK_SET );
		}

		bytes = _read( info->fd, buffer, (int)bytes );
	}

	return bytes;
}

CServerFiles & getServerFiles()
{
	static CServerFiles inst;
	return inst;
}
