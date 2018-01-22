#include "lua_debuger_sock.h"

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

void client( int port )
{
	char err[XNET_ERR_LEN];
	SOCKET client = tcp_connect( err, "127.0.0.1", port, 0 );

	if( XNET_ERR == net_nonblock( err, client ) )
	{
		printf( "nonblock set error %s\n", err );
		return;
	}

	char peer_addr[64];
	int  peer_port;

	cli.signal.notify_all();

	while( !cli.exit )
	{
		int r = net_recv( cli.sock, cli.recv_buffer + cli.recv, sizeof( cli.recv_buffer ) - cli.recv );
		if( r > 0 )
		{
			cli.recv += r;

			if( cli.recv > sizeof( int ) )
			{
				// 解包
				cli.ipkg = *(int*)cli.recv_buffer;
			}

			if( cli.recv >= cli.ipkg )
			{
				// 包已收全
				if( sizeof( cli.send_buffer ) - cli.send >= cli.ipkg )
				{
					// 等待处理包数据
					wait_signal();

					// 移动后续的数据
					memmove( cli.recv_buffer, cli.recv_buffer + cli.ipkg, cli.ipkg );

					// 重新计算已接收数据包尺寸
					cli.recv -= cli.ipkg;
					cli.ipkg = 0;
				}
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
void send( const void* data, int size )
{
	if( size + sizeof( int ) > sizeof( cli.send_buffer ) - cli.send )
		return;

	// 注意，send_buffer处于多线程环境下，这里未做数据保护
	memcpy( cli.send_buffer + cli.recv, &size, sizeof( size ) );
	cli.recv += sizeof( size );
	memcpy( cli.send_buffer + cli.recv, data, size );
	cli.recv += size;
}
