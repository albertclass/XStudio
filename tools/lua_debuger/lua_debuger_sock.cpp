#include "lua_debuger_sock.h"

/// 调试客户端
debuger cli;

void wait_signal()
{
	if( !cli.exit )
	{
		// 调用fini退出时，可能导致server中即将进入wait_signal
		// 此时等待信号量将会与fini中的thread.join进入互锁，故此处需判定。
		cli.signal.notify_one();

		std::unique_lock< std::mutex > lock( cli.mtx );
		cli.signal.wait( lock );
	}
}

void parse_request()
{
	char* cur = cli.recv_buffer + sizeof( int );

	auto split = string_split( cur, "\n" );
	for( auto &str : split )
	{
		cli.response.push_back( str );

		if( str.empty() )
		{
			// 等待处理请求
			wait_signal();
			cli.response.clear();
		}
	}
}

void client( const char* host, int port )
{
	char err[XNET_ERR_LEN];
	cli.sock = tcp_connect( err, host, port, 0 );
	if( cli.sock == -1 )
	{
		printf( "connect server %s:%d error = %s\n", host, port, err );
		cli.exit = true;
		cli.signal.notify_one();
		return;
	}

	if( XNET_ERR == net_nonblock( err, cli.sock ) )
	{
		printf( "nonblock set error %s\n", err );
		cli.exit = true;
		cli.signal.notify_one();
		return;
	}

	cli.signal.notify_one();

	while( !cli.exit )
	{
		int r = net_recv( cli.sock, cli.recv_buffer + cli.recv, sizeof( cli.recv_buffer ) - cli.recv );
		if( r > 0 )
		{
			cli.recv += r;

			while( cli.recv > sizeof( int ) )
			{
				// 解包
				cli.ipkg = *(int*)cli.recv_buffer + sizeof( int );

				if( cli.recv < cli.ipkg )
					break;
				
				// 包已收全
				parse_request();

				// 移动后续的数据
				memmove( cli.recv_buffer, cli.recv_buffer + cli.ipkg, cli.ipkg );

				// 重新计算已接收数据包尺寸
				cli.recv -= cli.ipkg;
				cli.ipkg = 0;
			}
		}

		if( cli.send )
		{
			int w = net_send( cli.sock, cli.send_buffer, cli.send );
			if( w > 0 )
			{
				memmove( cli.send_buffer, cli.send_buffer + w, w );
				cli.send -= w;
			}
		}
	}

	if( cli.sock != -1 )
	{
		net_close( cli.sock );
		cli.sock = -1;
	}
}

/// 发送消息给调试器
void request( const void* data, int size )
{
	if( size + sizeof( int ) > sizeof( cli.send_buffer ) - cli.send )
		return;

	// 注意，send_buffer处于多线程环境下，这里未做数据保护
	memcpy( cli.send_buffer + cli.send, &size, sizeof( size ) );
	cli.send += sizeof( size );
	memcpy( cli.send_buffer + cli.send, data, size );
	cli.send += size;
}
