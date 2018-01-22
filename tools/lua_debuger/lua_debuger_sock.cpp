#include "lua_debuger_sock.h"

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
				// ���
				cli.ipkg = *(int*)cli.recv_buffer;
			}

			if( cli.recv >= cli.ipkg )
			{
				// ������ȫ
				if( sizeof( cli.send_buffer ) - cli.send >= cli.ipkg )
				{
					// �ȴ����������
					wait_signal();

					// �ƶ�����������
					memmove( cli.recv_buffer, cli.recv_buffer + cli.ipkg, cli.ipkg );

					// ���¼����ѽ������ݰ��ߴ�
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

/// ������Ϣ��������
void send( const void* data, int size )
{
	if( size + sizeof( int ) > sizeof( cli.send_buffer ) - cli.send )
		return;

	// ע�⣬send_buffer���ڶ��̻߳����£�����δ�����ݱ���
	memcpy( cli.send_buffer + cli.recv, &size, sizeof( size ) );
	cli.recv += sizeof( size );
	memcpy( cli.send_buffer + cli.recv, data, size );
	cli.recv += size;
}
