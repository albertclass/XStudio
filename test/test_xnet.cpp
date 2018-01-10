#include "frame.h"
#include "xnet.h"
#include "xutility.h"
#include <thread>
#if defined( _WINDOWS )
#include <WinSock2.h>
#elif defined( _LINUX )
#include <sys/socket.h>
#endif

using namespace xgc;
using namespace xgc::common;

bool running = true;
void server( const char* host, int port )
{
	char err[XNET_ERR_LEN];
	SOCKET server = tcp_server( err, port, "0.0.0.0", 10 );

	if( XNET_ERR == net_nonblock( err, server ) )
	{
		printf( "nonblock set error %s\n", err );
		return;
	}

	char peer_addr[64];
	int  peer_port;

	struct client
	{
		SOCKET s;
		int send;
		int recv;
		char send_buffer[1024 * 4];
		char recv_buffer[1024 * 4];
	};

	client* cli[128];
	int cli_count = 0;
	while( running )
	{
		SOCKET s = tcp_accept( err, server, peer_addr, sizeof( peer_addr), &peer_port );
		if( s != -1 )
		{
			printf( "new client accept %s:%d\n", peer_addr, peer_port );

			cli[cli_count] = new client;
			cli[cli_count]->s = s;
			cli[cli_count]->send = 0;
			cli[cli_count]->recv = 0;

			cli_count++;
		}

		for( int i = 0; i < cli_count; ++i )
		{
			client &c = *cli[i];
			int r = recv( c.s, c.recv_buffer + c.recv, sizeof( c.recv_buffer ) - c.recv, 0 );
			if( r > 0 )
			{
				c.recv += r;

				if( c.recv > sizeof( int ) )
				{
					// 解包
					auto packet_len = *(int*)c.recv_buffer;
					if( c.recv >= packet_len )
					{
						if( sizeof( c.send_buffer ) - c.send >= packet_len )
						{
							// 回发消息
							memcpy( c.send_buffer + c.send, c.recv_buffer, packet_len );
							memmove( c.recv_buffer, c.recv_buffer + packet_len, packet_len );
							c.recv = c.recv - packet_len;
							c.send += packet_len;
						}
					}
				}
			}

			if( c.send )
			{
				int w = send( c.s, c.send_buffer, c.send, 0 );
				if( w > 0 )
				{
					memmove( c.send_buffer, c.send_buffer + w, w );
					c.send -= w;
				}
			}
		}
	}
}

void client( const char* host, int port )
{
	char err[XNET_ERR_LEN];

	SOCKET s = tcp_connect( err, host, port, 0 );
	if( s == XNET_ERR )
	{
		printf( "create connection error %s\n", err );
		return;
	}

	char hex[] = "0123456789abcdef";
	char buf1[128];
	char buf2[128];

	while( running )
	{
		*(int*)buf1 = 128;

		for( int i = 4; i < sizeof( buf1 ) - 1; ++i )
		{
			auto idx = random_range( 0, 15 );
			buf1[i] = hex[idx];
		}

		buf1[127] = 0;

		net_send( s, buf1, sizeof( buf2 ) );

		int r = net_recv( s, buf2, sizeof( int ) );
		if( r == -1 )
			break;
		
		r = net_recv( s, buf2 + r, *(int*)buf2 - sizeof(int) );
		if( r == -1 )
			break;

		int cmp = strcasecmp( buf1 + 4, buf2 + 4 );
		printf( "%s\n%s %d\n", buf1 + 4, buf2 + 4, cmp );
	}

	puts( "client exit.\n" );
}

static int testmain()
{
	std::thread srv( server, "0.0.0.0", 20000 );
	std::thread cli( client, "127.0.0.1", 20000 );

	getch();
	running = false;

	srv.join();
	cli.join();

	return 0;
}

UNIT_TEST( "xnet", "test xnet", testmain );
