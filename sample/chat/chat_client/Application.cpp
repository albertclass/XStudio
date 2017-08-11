#include "header.h"
#include "Application.h"
extern xgc_void test_0( int argc, char *argv[] );
extern xgc_void test_1( int argc, char *argv[] );
extern xgc_void test_2( int argc, char *argv[] );
extern xgc_void test_3( int argc, char *argv[] );

CApplication theApp;

static void __cdecl finialize()
{
	theApp.Clear();
}

CApplication::CApplication()
{
}

CApplication::~CApplication()
{
}

xgc_bool CApplication::Setup( xgc_lpcstr lpConfigFile )
{
	char conf_file[XGC_MAX_FNAME] = { 0 };

	if( xgc_nullptr == get_absolute_path( conf_file, lpConfigFile ) )
	{
		fprintf( stderr, "format conf path error %s", conf_file );
		return false;
	}

	ini_reader ini;
	if( false == ini.load( conf_file ) )
	{
		fprintf( stderr, "conf load error %s", conf_file );
		return false;
	}

	struct net::Param_SetBufferSize param1;
	param1.recv_buffer_size = 64 * 1024;
	param1.send_buffer_size = 64 * 1024;

	if( 0 != net::ExecuteState( Operator_SetBufferSize, &param1 ) )
	{
		return false;
	}

	struct net::Param_SetPacketSize param2;
	param2.recv_packet_size = 64 * 1024;
	param2.send_packet_size = 64 * 1024;

	if( 0 != net::ExecuteState( Operator_SetPacketSize, &param2 ) )
	{
		return false;
	}

	auto addr = ini.get_item_value( "Client", "GameAddr", "0.0.0.0" );
	strcpy_s( mGameAddr, addr );
	auto port = ini.get_item_value( "GameServer", "Port", 40001 );
	mGamePort = port;

	addr = ini.get_item_value( "Client", "ChatAddr", "127.0.0.1" );
	strcpy_s( mChatAddr, addr );
	port = ini.get_item_value( "ChatServer", "CliPort", 50001 );
	mChatPort = port;

	return init_logger( ini );
}

xgc_void CApplication::Run( int argc, char *argv[] )
{
	struct {
		xgc_lpcstr tips;
		xgc_void( *call )( int argc, char *argv[] );
	} menu [] = {
		{ "连接测试", test_0 },
		{ "连接压力测试", test_1 },
		{ "频道压力测试", test_2 },
		{ "聊天压力测试", test_3 },
	};

	int idx = 0;
	for( auto item : menu )
		printf( "%d. %s\n", ++idx, item.tips );

	printf( "q. exit\n" );

	printf( "choice:" );

	int choice = getch() - '1';
	if( choice < XGC_COUNTOF( menu ) )
	{
		menu[choice].call( argc, argv );
	}
}

///
/// \brief 清理服务器
///
/// \author albert.xu
/// \date 2017/08/10
///
xgc_void CApplication::Clear()
{
	fini_logger();
	google::protobuf::ShutdownProtobufLibrary();
}

int main( int argc, char* argv[] )
{
	atexit( finialize );

	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	auto pLocale = setlocale( LC_ALL, "chs" );
	if( pLocale )
		printf( "locale set with %s\n", pLocale );
	else
		printf( "locale set failed\n" );

	auto pConfigFile = "chat-server.ini";
	if( argc > 1 )
		pConfigFile = argv[1];

	if( false == theApp.Setup( pConfigFile ) )
		return -1;

	theApp.Run( argc, argv );

	return 0;
}
