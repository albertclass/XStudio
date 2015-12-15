// TestClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <time.h>
#include "serialization.h"

VOID ProcessPacket( net::INetPacket* pPacket );
typedef std::list< net::network_t > HandleList;
HandleList handleList;

int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	net::CreateNetwork( net::asio );
	net::InitNetwork( 4 );

	net::IMessageQueue* mQueue = xgc_nullptr;
	for( int i = 0; i < 1000; ++i )
	{
		net::ConnectServerAsync( "127.0.0.1", 30120, net::ProtocalDefault, 1000, &mQueue );
	}

	clock_t now = clock();
	while( clock() - now < 60000 )
	{
		net::INetPacket* pPacket = xgc_nullptr;

		if( mQueue )
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
	_set.userdata_ptr = xgc_nullptr;
	net::ExecuteState( Operator_SetUserdata, (xgc_uintptr) &_set );

	handleList.push_back( h );

	printf( "accept link %u \n", h );
}

void ProcessClose( net::network_t h, xgc_lpvoid userdata )
{
	HandleList::iterator i = std::find( handleList.begin(), handleList.end(), h );
	if( i != handleList.end() )
		handleList.erase( i );

	SAFE_DELETE( userdata );
	printf( "close link %u \n", h );
}

void ProcessError( net::network_t h, int e )
{
	printf( "link %u error = %d\n", h, e );
}

VOID ProcessPacket( net::INetPacket* pPacket )
{
	serialization buf( (xgc_lpvoid)pPacket->header(), pPacket->size() );
	xgc_uint16 length;
	xgc_byte type, code;
	buf >> length >> type >> code;
	switch( type )
	{
	case SYSTEM_MESSAGE_TYPE:
		switch( code )
		{
		case EVENT_CONNECT:
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
			net::SendPacket( pPacket->handle(), pPacket->header(), pPacket->size() );
		}
	}
}