// CaseService.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "service.h"
#include "screen.h"

#include "message_handle.h"
#include "manager.h"

#include "PythonScript.h"
using namespace xgc::message;
using namespace std::chrono;
/// 是否退出中
xgc_bool IsConsoleQuit = false;

/// 服务器连接地址
xgc_string g_addr = "127.0.0.1";
/// 服务器连接端口
xgc_uint16 g_port = 20375;
/// 脚本存储路径
fs::path g_loader_path;
fs::path g_module_path;

///
/// \brief 处理键盘输入
///
/// \author albert.xu
/// \date 2016/02/24 14:24
///
xgc_int32 ProcessConsoleInput( xgc_lpcstr pszCommandLine )
{
	if( _stricmp( pszCommandLine, "exit" ) == 0 )
		IsConsoleQuit = true;

	return 0;
}

/// 程序入口
int ServiceMain( int argc, char *argv[] )
{
	xgc_lpcstr lpConfigPath = "Config.ini";
	if( argc > 1 )
	{
		lpConfigPath = argv[1];
	}

	// 创建网络
	if( !CreateNetwork( eNetLibrary::asio ) )
		return -1;

	// 初始化网络
	if( !InitNetwork( 1 ) )
		return -1;

	if( !getScript().Init() )
		return -1;

	// 初始化消息分派表
	xgc::message::init_deliver_table();

	// 声明消息队列
	MessageQueuePtr pQueue = xgc_nullptr;

	try
	{
		xgc_char szConfigPath[_MAX_PATH] = { 0 };
		GetNormalPath( szConfigPath, sizeof( szConfigPath ), lpConfigPath );

		// 读取配置文件
		ini_reader ini;
		ini.load( szConfigPath );

		// 删除关闭按钮
		if( !IsService() )
		{
			HMENU hSysMenu = GetSystemMenu( GetConsoleWindow(), FALSE );
			if( hSysMenu != 0 )
			{
				RemoveMenu( hSysMenu, SC_CLOSE, MF_BYCOMMAND );
			}
		}

		// 获取连接服务器地址
		g_addr = ini.get_item_value( "ServerInfo", "Addr", "127.0.0.1" );
		g_port = ini.get_item_value( "ServerInfo", "Port", 20375 );
		// 获取框架的存储目录
		xgc_char szPath[_MAX_PATH] = { 0 };

		auto module_path = ini.get_item_value( "Config", "ModuleRoot", "./Script/" );
		g_module_path = GetNormalPath( szPath, sizeof( szPath ), "%s", module_path );
		// 获取脚本的存储目录
		auto loader_path = ini.get_item_value( "Config", "LoaderPath", "./Script/Robot" );
		g_loader_path = GetNormalPath( szPath, sizeof( szPath ), "%s", loader_path );

		xgc_char szDateTime[64] = { 0 };

		//get_case_manager().start( "robot000", "123456", "R000", "1.0.0.1" );

		auto tick = system_clock::now();
		// 开始逻辑主循环
		while( !IsServiceStoped() && !IsConsoleQuit )
		{
			// 重连设置
			if( system_clock::now() >= tick && is_disconnect() )
			{
				datetime::now().to_string( szDateTime );
				printf( "%s - connect server %s : %u\n", szDateTime, g_addr.c_str(), g_port );
				ConnectServerAsync( g_addr.c_str(), g_port, ProtocalDefault, 10000, pQueue );

				tick = system_clock::now() + seconds( 5 );
			}

			get_timer().step();

			// 服务暂停
			if( IsServicePaused() )
			{
				Sleep( 1 );
			}
			// 处理消息
			else if( 0 == DeliverMessage( pQueue, 100 ) )
			{
				Sleep( 1 );
			}

			// 处理调试命令行
			if( !IsService() && _kbhit() )
			{
				auto ch = _getch();
				if( ch == 27 ) // esc
				{
					printf( "#" );
					xgc_char cmdline[64];
					scanf_s( "%s", cmdline, (int)_countof(cmdline) );

					// 判定退出指令
					if( _stricmp( cmdline, "exit" ) == 0 )
						IsConsoleQuit = true;
				}
			}
		}
	}
	catch( std::exception err )
	{
		puts( err.what() );
	}

	if( !IsService() )
	{
		FinializeConsole();
	}

	getScript().Fini();
	FiniNetwork();
	DestroyNetwork();

    return 0;
}

