// TestServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Client.h"
#include <conio.h>
#include <time.h>
#include <fstream>
#include "json/json.h"

VOID ProcessPacket( net::INetPacket* pPacket );
extern void ConsoleTest();

void Print( const Json::Value& node, char *deep, char *deep_cur )
{
	switch( node.type() )
	{
	case Json::stringValue:
		printf( "%s\n", node.asCString() );
		break;
	case Json::booleanValue:
		printf( "%s\n", node.asBool()?"true":"false" );
		break;
	case Json::intValue:
		printf( "%d\n", node.asInt() );
		break;
	case Json::uintValue:
		printf( "%u\n", node.asUInt() );
		break;
	case Json::realValue:
		printf( "%lf\n", node.asDouble() );
		break;
	case Json::arrayValue:
		putch('\n');
		for( Json::ValueConstIterator i = node.begin(); i != node.end(); ++i )
		{
			deep_cur[0] = '|';
			deep_cur[1] = '-';
			printf( "%s[%d] : ", deep, i.index() );

			deep_cur[1] = ' ';
			Print( *i, deep, deep_cur+2 );
			*deep_cur = 0;
		}
		break;
	case Json::objectValue:
		putch('\n');
		for( Json::ValueConstIterator i = node.begin(); i != node.end(); ++i )
		{
			deep_cur[0] = '|';
			deep_cur[1] = '-';
			printf( "%s%s : ", deep, i.key().asCString() );

			deep_cur[1] = ' ';
			Print( *i, deep, deep_cur+2 );
			*deep_cur = 0;
		}
		break;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::ifstream ifs( "Config.Json" );
	if( ifs.is_open() )
	{
		Json::Value mRoot;
		Json::Reader mReader;
		if( mReader.parse( ifs, mRoot ) == false )
		{
			std::cout 
				<< "parse config file 'config.json' error" << std::endl 
				<< mReader.getFormatedErrorMessages().c_str() << std::endl;
			mRoot.clear();
		}

		char deep[256] = { ' ' };
		Print( mRoot, deep, deep );
		Json::Value ServerProxyName = 
			mRoot["LoginServer"]["ServerProxy"];
		ifs.close();
	}

	//initialize_console_library();
	//text_t t = text( 40, 10 );
	//window_t w = window( 10, 10, 20, 10, t );
	//draw_window( w );

	//for( int i = 0; i < 1000; ++i )
	//{
	//	wprintf_ascii( w, "%04d:1234567890123456789012345678901234567890\n", i );
	//	Sleep(1000);
	//}

	//free( w );
	//finialize_console_library();

	net::CreateNetwork( net::asio );
	net::InitNetwork( 10 );

	net::IMessageQueue* mQueue = xgc_nullptr;
	xgc_uintptr hServer = net::StartServer( "0.0.0.0", 30120, net::ProtocalDefault, 0, 0, 10, &mQueue );
	if( hServer && mQueue )
	{
		net::INetPacket* pPacket = xgc_nullptr;
		clock_t now = clock();
		while( clock() - now < 60000 * 10 )
		{
			if( mQueue->PopMessage( &pPacket ) )
			{
				ProcessPacket( pPacket );
				pPacket->release();
			}
			else
			{
				Sleep(1);
			}
		}

		net::CloseServer( hServer );
	}
	net::FiniNetwork();
	net::DestroyNetwork();

	SAFE_RELEASE( mQueue );
	return 0;
}

void ProcessAccept( net::network_t h )
{
	net::Param_SetUserdata _set;
	_set.handle = h;
	_set.userdata_ptr = reinterpret_cast<xgc_lpvoid>( new CClient() );
	net::ExecuteState( Operator_SetUserdata, (xgc_uintptr) &_set );
}

void ProcessClose( net::network_t h, xgc_lpvoid userdata )
{
	CClient* pClient = reinterpret_cast< CClient* >( userdata );
	SAFE_DELETE(pClient);
}

void ProcessError( net::network_t h, int e )
{
	net::CloseLink( h );
}

VOID ProcessPacket( net::INetPacket* pPacket )
{
	serialization buf( (xgc_lpvoid)pPacket->data(), pPacket->size() );
	xgc_uint16 length;
	xgc_byte type, code;
	buf >> length >> type >> code;
	switch( type )
	{
	case SYSTEM_MESSAGE_TYPE:
		switch( code )
		{
		case EVENT_ACCEPT:
			ProcessAccept( pPacket->handle() );
			break;
		case EVENT_CLOSE:
			ProcessClose( pPacket->handle(), pPacket->userdata() );
			break;
		case EVENT_ERROR:
			ProcessError( pPacket->handle(), *(int*)buf.rd_ptr() );
			break;
		}
		break;
	default:
		{
			net::SendPacket( pPacket->handle(), pPacket->data(), pPacket->size() );
		}
	}
}