#include "frame.h"
#include "Netbase.h"
#include <thread>
#include <chrono>
#include "xbuffer.h"
#include "serialization.h"

using namespace xgc::net;
using namespace xgc::common;

static void server()
{
	MessageQueuePtr message_queue;
	auto srv = StartServer( "0.0.0.0", 25000, 1000, message_queue );
	INetPacket* packet = xgc_nullptr;
	while( true )
	{
		if( message_queue->PopMessage( &packet ) )
		{
			MessageHeader* header = (MessageHeader*)packet->header();

			if( header->type == SYSTEM_MESSAGE_TYPE )
			{
				switch( header->code )
				{
				case EVENT_ACCEPT:
					break;
				case EVENT_CLOSE:
					break;
				case EVENT_ERROR:
					break;
				}
			}
			else
			{
				SendPacket( packet->handle(), (xgc_lpvoid)packet->header(), packet->length() );
			}
		}
		else
		{
			std::this_thread::sleep_for( std::chrono::milliseconds(1) );
		}
	}
	CloseServer( srv );
}

static void client()
{
	MessageQueuePtr queue;
	if( Connect( "127.0.0.1", 25000, 0, queue ) )
	{
		while(true )
		{
			INetPacket *packet = xgc_nullptr;
			if( queue->PopMessage( &packet ) )
			{
				MessageHeader* header = (MessageHeader*)packet->header();

				if( header->type == SYSTEM_MESSAGE_TYPE )
				{
					linear_buffer< static_buffer< 256 > > buffer;
					switch( header->code )
					{
					case EVENT_CONNECT:
						buffer << xgc_uint16(0) << xgc_uint8(1) << xgc_uint8(0) << "bairi1shanjin";
						*(xgc_uint16*)buffer.base() = (xgc_uint16)buffer.wd();
						SendPacket( packet->handle(), buffer.base(), buffer.wd() );
						break;
					case EVENT_CLOSE:
						break;
					case EVENT_ERROR:
						break;
					}
				}
			}
		}
	}
}

static int testmain( int agrc, char * argv[] )
{
	CreateNetwork( asio );
	InitNetwork(16);

	std::thread tsvr( server );
	std::thread tcli( client );

	tcli.join();
	tsvr.join();

	FiniNetwork();
	DestroyNetwork();
	return 0;
}

UNIT_TEST( "network", "test network", testmain );
