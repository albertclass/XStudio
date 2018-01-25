#include "lua_debuger_sock.h"

/// ���Կͻ���
debuger cli;

void wait_signal()
{
	if( !cli.exit )
	{
		// ����fini�˳�ʱ�����ܵ���server�м�������wait_signal
		// ��ʱ�ȴ��ź���������fini�е�thread.join���뻥�����ʴ˴����ж���
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
			// �ȴ���������
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
				// ���
				cli.ipkg = *(int*)cli.recv_buffer + sizeof( int );

				if( cli.recv < cli.ipkg )
					break;
				
				// ������ȫ
				parse_request();

				// �ƶ�����������
				memmove( cli.recv_buffer, cli.recv_buffer + cli.ipkg, cli.ipkg );

				// ���¼����ѽ������ݰ��ߴ�
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

/// ������Ϣ��������
void request( const void* data, int size )
{
	if( size + sizeof( int ) > sizeof( cli.send_buffer ) - cli.send )
		return;

	// ע�⣬send_buffer���ڶ��̻߳����£�����δ�����ݱ���
	memcpy( cli.send_buffer + cli.send, &size, sizeof( size ) );
	cli.send += sizeof( size );
	memcpy( cli.send_buffer + cli.send, data, size );
	cli.send += size;
}
