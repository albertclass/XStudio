#include "header.h"
#include "session.h"
#include "server_files.h"

#include "curses.h"
#include "panel.h"

/// 是否运行中
xgc_bool running = true;

/// 根路径
xgc_char root_path[XGC_MAX_PATH] = { 0 };

int main( int argc, char* argv[] )
{
	initscr();

	WINDOW *win_err = newwin( 15, 40, 0, 0 );
	PANEL  *pel_err = new_panel(win_err);
	set_panel_userptr( pel_err, "err" );

	box( win_err, 0, 0 );
	mvwprintw( win_err, 1, 1, "error output window" );

	WINDOW *win_files = newwin( 40, 25, 41, 0 );
	PANEL  *pel_files = new_panel(win_err);
	set_panel_userptr( pel_files, "files" );

	box( win_files, 0, 0 );

	update_panels();
	doupdate();

	char conf_path[1024] = { 0 };
	if( xgc_nullptr == get_absolute_path( conf_path, "../server.ini" ) )
	{
		fprintf( stderr, "format conf path error %s", conf_path );
		return -1;
	}

	ini_reader ini;
	if( false == ini.load( conf_path ) )
	{
		fprintf( stderr, "conf load error %s", conf_path );
		return -1;
	}

	auto addr = ini.get_item_value( "Server", "Listen", "0.0.0.0" );
	auto port = ini.get_item_value( "Server", "Port", 50001 );

	auto path = ini.get_item_value( "Server", "Path", xgc_nullptr );
	XGC_ASSERT_RETURN( path, -1 );

	get_absolute_path( root_path, "%s", path );

	auto exist = _access( root_path, 6 );
	XGC_ASSERT_RETURN( exist == 0, -1 );

	getServerFiles().GenIgnoreList( root_path );
	getServerFiles().GenFileList( root_path );

	if( false == net::CreateNetwork( 1 ) )
		return -1;

	struct net::Param_SetBufferSize param1;
	param1.recv_buffer_size = 64 * 1024;
	param1.send_buffer_size = 64 * 1024;

	if( 0 != net::ExecuteState( Operator_SetBufferSize, &param1 ) )
	{
		net::DestroyNetwork();
		return -1;
	}

	struct net::Param_SetPacketSize param2;
	param2.recv_packet_size = 64 * 1024;
	param2.send_packet_size = 64 * 1024;

	if( 0 != net::ExecuteState( Operator_SetPacketSize, &param2 ) )
	{
		net::DestroyNetwork();
		return -1;
	}

	auto srv = net::StartServer( addr, port, 0, [](){ return XGC_NEW CNetSession(); } );
	while( running )
	{
		if( net::ProcessNetEvent( 100 ) == 100 )
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}

	net::CloseServer( srv );
	net::DestroyNetwork();

	endwin();
}