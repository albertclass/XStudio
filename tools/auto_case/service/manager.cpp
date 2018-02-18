#include "stdafx.h"
#include <io.h>
#include <fcntl.h>
#include <process.h>

#include "manager.h"
#include "message_handle.h"
#include "command_handle.h"
#include "proto_def.h"

#include "PythonScript.h"

#define PIPE_R 0
#define PIPE_W 1

// 外部引用脚本存储路径
extern fs::path g_loader_path;
extern fs::path g_module_path;

using xgc::message::g_session;

case_executer::case_executer( const xgc_string &username )
	: process_h( INVALID_HANDLE_VALUE )
	, name( username )
	, buffer_bytes( 0 )
	, t( INVALID_TIMER_HANDLE )
{
	t = get_timer().insert( std::bind( &case_executer::update, this ), datetime::now(), DURATION_FOREVER, "intv:1000" );

	int ret = 0;
	ret = _pipe( fd_r, 1024 * 4, _O_BINARY );
	XGC_ASSERT( ret != -1 );

	ret = _pipe( fd_w, 1024 * 4, _O_BINARY );
	XGC_ASSERT( ret != -1 );
}

case_executer::~case_executer()
{
	get_timer().remove( t );
	t = INVALID_TIMER_HANDLE;

	TerminateProcess( process_h, (UINT)-1 );
	CloseHandle( process_h );
	process_h = INVALID_HANDLE_VALUE;

	_close( fd_r[0] );
	_close( fd_r[1] );
	_close( fd_w[0] );
	_close( fd_w[1] );
}

///
/// \brief 获取执行文件完全路径
///
/// \author albert.xu
/// \date 2016/04/12 15:04
///
static xgc_lpstr get_executable_path( xgc_lpcstr file )
{
	xgc_char * pfs = (xgc_char*) calloc( _MAX_PATH, 1 );
	xgc_char * env, * cur = nullptr;
	xgc_size siz;
	if( 0 != _dupenv_s( &env, &siz, "PATH" ) )
		return nullptr;

	xgc_lpstr tok = strtok_s( env, ";", &cur );

	while( tok )
	{
		auto len = strlen( tok );
		if( tok[len-1] == '\\' || tok[len-1] == '/' )
			sprintf_s( pfs, _msize( pfs ), "%s%s", tok, file );
		else
			sprintf_s( pfs, _msize( pfs ), "%s\\%s", tok, file );

		if( _access_s( pfs, 0 ) == 0 )
			break;

		strcat_s( pfs, _msize( pfs ), ".exe" );
		if( _access_s( pfs, 0 ) == 0 )
			break;

		tok = strtok_s( nullptr, ";", &cur );
	}

	free( env );
	
	if( tok )
		return pfs;

	free( pfs );
	return nullptr;
}

xgc_bool case_executer::start( xgc_size pipe_size, xgc_lpcstr path, xgc_lpcstr cmdline )
{
	int ret = 0;
	
	// 检查执行目录是否存在
	if( _access_s( path, 0 ) != 0 )
		return false;

	/* Info for spawning the child. */
	STARTUPINFOA StartupInfo;  /* Info for spawning a child */
	BOOL childstatus = 0;
	PROCESS_INFORMATION ProcessInfo; /* child process information */

	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);

	// 文件描述符的继承
	StartupInfo.cbReserved2 =
		(unsigned short)(sizeof(int) + (7 * (sizeof(char) + sizeof(intptr_t))));

	StartupInfo.lpReserved2 = (LPBYTE)calloc( 1, StartupInfo.cbReserved2 );

	*((UNALIGNED int *)(StartupInfo.lpReserved2)) = 7;

	int i;
	char *posfile;
	UNALIGNED intptr_t *posfhnd;

	posfile = (char *)(StartupInfo.lpReserved2 + sizeof(int));
	posfhnd = (UNALIGNED intptr_t *)(StartupInfo.lpReserved2 + sizeof(int) + (7 * sizeof(char)));

	for( i = 0; i < 3; i++, posfile++, posfhnd++ )
	{
		*posfile = 0;
		*posfhnd = (intptr_t)INVALID_HANDLE_VALUE;
	}

	*posfile++ = 0x09;
	*posfhnd++ = _get_osfhandle( fd_w[0] );

	*posfile++ = 0x09;
	*posfhnd++ = _get_osfhandle( fd_r[1] );

	memset(&ProcessInfo, 0, sizeof(ProcessInfo));

	// 复制命令行参数
	xgc_lpstr cmd = (xgc_lpstr) calloc( _MAX_PATH, 1 );
	xgc_lpstr oth = _strdup( cmdline );

	xgc_lpstr chr = strchr( oth, ' ' );
	if( chr ) 
		strncpy_s( cmd, _msize(cmd), oth, chr - oth );

	// 查找可用的路径
	if( _access_s( cmd, 0 ) != 0 )
	{
		xgc_lpstr pfs = cmd;
		cmd = get_executable_path( pfs );
		free( pfs );
	}

	// 如果有可用的路径
	if( cmd )
	{
		// 创建进程
		childstatus = CreateProcessA( cmd,
										oth,
										NULL,
										NULL,
										TRUE,
										0, //CREATE_NO_WINDOW,
										NULL,
										path,
										&StartupInfo,
										&ProcessInfo );
		if( childstatus )
		{
			CloseHandle( ProcessInfo.hThread );
			process_h = ProcessInfo.hProcess;

			pid = ProcessInfo.dwProcessId;
			tid = ProcessInfo.dwThreadId;
		}
	}

	free( StartupInfo.lpReserved2 );
	free( cmd );
	free( oth );
	return childstatus != 0;
}

xgc_size case_executer::recv()
{
	// 从标准输出中读取内容并解析
	if( _eof( fd_r[0] ) == 0 )
	{
		int bytes = _read( fd_r[0], buffer + buffer_bytes, int(sizeof( buffer ) - buffer_bytes) );
		if( bytes < 0 )
			return -1;

		buffer_bytes += bytes;
		return bytes;
	}

	return 0;
}

xgc_size case_executer::send( xgc_lpcstr buffer, xgc_size buffer_size )
{
	xgc_size bytes = buffer_size;
	while( bytes )
	{
		int ret = _write( fd_w[1], buffer, (int)bytes );
		if( ret == -1 )
			return buffer_size - bytes;
	}

	return buffer_size;
}

xgc_void case_executer::update()
{
	if( status() == alive )
	{
		recv();

		xgc_size end = 0;
		xgc_size cur = 0;
		while( cur < buffer_bytes )
		{
			if( buffer[cur] == 0 )
			{
				DebugCommand::Process( name.c_str(), this, buffer + end );
				end = cur + 1;
			}

			++cur;
		}

		memmove( buffer, buffer + cur, buffer_bytes - cur );
	}
}

xgc_long case_executer::status()
{
	DWORD dwExitCode = 0;
	if( !GetExitCodeProcess( (HANDLE) process_h, &dwExitCode ) )
	{
		SYS_ERROR( "last error = %u", GetLastError() );
		return (xgc_long)terminate;
	}

	if( dwExitCode != STILL_ACTIVE )
		return exit;

	return (xgc_long) alive;
}

xgc_bool case_executer::exitcode( xgc_long* code )
{
	DWORD dwExitCode = 0;
	if( !GetExitCodeProcess( (HANDLE) process_h, &dwExitCode ) )
	{
		SYS_ERROR( "last error = %u", GetLastError() );
		return false;
	}

	if( dwExitCode != STILL_ACTIVE )
	{
		if( code )
			*code = (xgc_long) dwExitCode;
		return true;
	}

	return false;
}

xgc_ulong case_executer::get_pid() const
{
	return pid;
}

xgc_ulong case_executer::get_tid() const
{
	return tid;
}

//////////////////////////////////////////////////////////////////////////
manager::manager()
	: session( INVALID_NETWORK_HANDLE )
{
}

manager::~manager()
{
}

xgc_long manager::start( const std::string &username, 
						 const std::string &password, 
						 const std::string &module, 
						 const std::string &loader_version, 
						 const std::string &module_version , 
						 const std::string &address )
{
	if( module.empty() )
		return false;

	xgc_string version;

	auto loader_path = g_loader_path.string();
	if( _access( loader_path.c_str(), 0 ) != 0 )
	{
		// 路径不存在，需要更新脚本文件
		request_loader();
		return -1;
	}

	// 检查loader版本
	if( false == getScript().GetLoaderVersion( version ) )
	{
		request_loader();
		return -1;
	}

	if( version != loader_version )
	{
		request_loader();
		return -1;
	}

	auto module_path = g_module_path.string();
	if( _access( module_path.c_str(), 0 ) != 0 )
	{
		// 路径不存在，需要更新脚本文件
		request_module( module );
		return -2;
	}

	if( false == getScript().GetModuleVersion( module, version ) )
	{
		request_module( module );
		return -2;
	}

	if( version != module_version )
	{
		request_module( module );
		return -2;
	}

	if( false == add( username ) )
		return -3;
	
	auto p = get( username );
	if( xgc_nullptr == p )
		return -3;

	xgc_char cmdline[1024] = { 0 };
	sprintf_s( cmdline, "python autocase.py -u %s -p %s -m %s%s -h=%s 3 4", 
			   username.c_str(), 
			   password.c_str(), 
			   module_path.c_str(),
			   module.c_str(), 
			   address.c_str() );

	if( false == p->start( 1024 * 4, loader_path.c_str(), cmdline ) )
	{
		del( username );
		return -4;
	}

	ServerStartAck stAck;
	stAck.username = username;
	stAck.pid = p->get_pid();
	stAck.tid = p->get_tid();

	Send( g_session, stAck );
	return 0;
}

void manager::stop( const std::string &username )
{
	del( username );
}

void manager::request_loader()
{
	ClientFilesReq stReq;
	stReq.root = 0;
	stReq.relative = "loader";

	Send( xgc::message::g_session, stReq );
}

///
/// \brief 请求发送脚本文件
///
/// \author albert.xu
/// \date 2016/04/11 11:16
///
void manager::request_module( const std::string &module_name )
{
	ClientFilesReq stReq;
	stReq.root = 1;
	stReq.relative = module_name;

	Send( xgc::message::g_session, stReq );
}

///
/// \brief 添加测试进程到管理器
///
/// \author albert.xu
/// \date 2016/03/14 16:14
///
xgc_bool manager::add( const xgc_string & username )
{
	auto ib = case_map.insert( std::make_pair( username, XGC_NEW case_executer( username ) ) );
	return ib.second;
}

///
/// \brief 删除测试进程从管理器
///
/// \author albert.xu
/// \date 2016/03/14 16:15
///
xgc_bool manager::del( const xgc_string & username )
{
	auto it = case_map.find( username );
	if( it != case_map.end() )
	{
		SAFE_DELETE( it->second );
		case_map.erase( it );

		return true;
	}

	return false;
}

case_executer * manager::get( const xgc_string & username )
{
	auto it = case_map.find( username );
	if( it != case_map.end() )
		return it->second;

	return nullptr;
}

manager &get_case_manager()
{
	static manager inst;
	return inst;
}

timer &get_timer()
{
	static timer inst;
	return inst;
}