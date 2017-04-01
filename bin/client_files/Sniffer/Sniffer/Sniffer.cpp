// Sniffer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "iphdr.h"

typedef void( *prase_fn )( const char* packet, size_t size );
typedef int(*pfn_writelog)(const char* fmt, ...);

void prase_packet( const char* packet, size_t size );
void prase_ip( const char* packet, size_t size );
void prase_tcp( const char* packet, size_t size );
void prase_pkg( const char* packet, size_t size, prase_fn decode );
void prase_pipe( const char* packet, size_t size );
void prase_msg( const char* packet, size_t size );

TCP_PEER	src_peer;
TCP_PEER	dst_peer;

char src_addr[32];
char dst_addr[32];

IPV4_HDR	ip4;
TCP_HDR		tcp;

char *	msgbuf = nullptr;
size_t	msglen = 0;

struct msginfo
{
	u_long count;
	u_long total_byte;
};

static msginfo msgvec[0xffff] = { 0 };
static time_t msgsav = time(nullptr);

std::unordered_map< TCP_CONNECTION, PTCP_SESSION, std::_Bitwise_hash< TCP_CONNECTION > > sessions;

PTCP_SESSION session = nullptr; // 当前的Session对象

struct value_string
{
	char*	string;
	int		value;
};

value_string _Protocal[] =
{
	{ "HOPOPTS", 0 },// IPv6 Hop-by-Hop options
	{ "ICMP", 1 },
	{ "IGMP", 2 },
	{ "GGP", 3 },
	{ "IPV4", 4 },
	{ "ST", 5 },
	{ "TCP", 6 },
	{ "CBT", 7 },
	{ "EGP", 8 },
	{ "IGP", 9 },
	{ "PUP", 12 },
	{ "UDP", 17 },
	{ "IDP", 22 },
	{ "RDP", 27 },
	{ "IPV6", 41 }, // IPv6 header
	{ "ROUTING", 43 }, // IPv6 Routing header
	{ "FRAGMENT", 44 }, // IPv6 fragmentation header
	{ "ESP", 50 }, // encapsulating security payload
	{ "AH", 51 }, // authentication header
	{ "ICMPV6", 58 }, // ICMPv6
	{ "NONE", 59 }, // IPv6 no next header
	{ "DSTOPTS", 60 }, // IPv6 Destination options
	{ "ND", 77 },
	{ "ICLFXBM", 78 },
	{ "PIM", 103 },
	{ "PGM", 113 },
	{ "L2TP", 115 },
	{ "SCTP", 132 },
	{ "RAW", 255 },
	{ "MAX", 256 },
	{ nullptr, 0 },
};

const char* search_string( int value, value_string* table )
{
	for( int i = 0; table && table[i].string; ++i )
	{
		if( table[i].value == value )
			return table[i].string;
	}

	return "";
}

static unsigned char szOutput[1024 * 1024 * 128];
static unsigned char *pOutput = nullptr;
int Output( const char* fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	int cnt = _vsnprintf_s( (char*) pOutput, szOutput + sizeof(szOutput) -pOutput, szOutput + sizeof(szOutput) -pOutput, fmt, ap );
	if( cnt > 0 )
		pOutput += cnt;

	return cnt;
}

void ClearOutput()
{
	szOutput[0] = 0;
}

void PrintRawDataLine( const unsigned char *data, int length, int width, pfn_writelog writelog )
{
	for( int i = 0; i < width; ++i )
	{
		if( i < length )
			writelog( "%02X ", data[i] );
		else
			writelog( "   " );
	}

	writelog( "[" );

	for( int i = 0; i < width; ++i )
	{
		if( i < length )
			writelog( "%c", isprint( data[i] ) ? data[i] : '.' );
		else
			writelog( " " );
	}

	writelog( "]\n" );
}

/// 显示原始数据包

void PrintRawData( const unsigned char *data, int length, int width, pfn_writelog writelog )
{
	writelog( "-------------Data Begins-------------\n" );
	while( length > 0 )
	{
		PrintRawDataLine( data, min( length, width ), width, writelog );
		if( length < width )
		{
			data += length;
			length = 0;
		}
		else
		{
			data += width;
			length -= width;
		}

	}
}

void getInterfaces( std::vector< std::string >& lst )
{
	try
	{
		char     hostname[MAX_PATH];
		HOSTENT *hostaddr;
		int      adapter_idx = 0;
		struct sockaddr_in   address;

		int ret = gethostname(hostname, sizeof(hostname));
		if (ret != 0)
			return;

		hostaddr = gethostbyname(hostname);
		if (hostaddr == nullptr)
			return;

		while (hostaddr->h_addr_list[adapter_idx])
		{
			memcpy(&address.sin_addr, hostaddr->h_addr_list[adapter_idx], hostaddr->h_length);

			lst.push_back(inet_ntoa(address.sin_addr));
			adapter_idx++;
		}
	}
	catch (...)
	{
		puts("get interface failed.");
	}

	return;
}

int main( int argc, char* argv[] )
{
	SOCKET			s = INVALID_SOCKET;
	WSABUF			wbuf = { 0 };
	DWORD			dwBytesRet = 0, dwFlags = 0;
	char			rcvbuf[1024*16];
	int				rc = 0, err;
	int idx = 0;
	int sel = 0;

	memset( msgvec, 0, sizeof( msgvec ) );

	WSADATA wsaData;
	WSAStartup( MAKEWORD( 2, 2 ), &wsaData );

	if( argc > 1 )
		src_peer.port = (unsigned short)atoi( argv[1] );

	if( argc > 2 )
		src_peer.addr = inet_addr( argv[2] );

	std::vector< std::string > vec = { "localhost" };
	getInterfaces( vec );
	if( vec.size() > 1 )
	{
		puts( "choice one interface: " );
		for( auto iter = vec.begin(); iter != vec.end(); ++iter )
		{
			printf( "%d. %s\n", idx++, iter->c_str() );
		}
		puts( "q. exit" );
		do
		{
			sel = _getch();
			if( sel == 'q' || sel == 'Q' || sel == 27 )
				return -1;

			if( isdigit( sel ) )
			{
				sel -= '0';
				if( sel < (int)vec.size() )
					break;
			}

		} while( true );
	}
	//
	// Create a raw socket for receiving IP datagrams
	//
	s = socket( AF_INET, SOCK_RAW, IPPROTO_IP );
	if( s == INVALID_SOCKET )
	{
		printf( "WSASocket() failed: %d\n", WSAGetLastError() );
		return false;
	}

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr( vec[sel].c_str() );
	sa.sin_port = 0;

	rc = bind( s, (SOCKADDR *)&sa, sizeof( sa ) );
	if( rc == SOCKET_ERROR )
	{
		printf( "bind() failed: %d\n", WSAGetLastError() );
		if( INVALID_SOCKET != s )
		{
			closesocket( s );
			s = INVALID_SOCKET;
		}
		WSACleanup();
		return false;
	}

	//
	// Set the SIO_RCVALLxxx ioctl
	//
	u_long	optval = 1;
	rc = ioctlsocket( s, SIO_RCVALL, &optval );
	if( rc == SOCKET_ERROR )
	{
		printf( "WSAIotcl(0x%x) failed: %d\n", SIO_RCVALL, ( err = WSAGetLastError() ) );
		if( err == WSAEINVAL )
		{
			printf( "NOTE: IPv6 does not currently support the SIO_RCVALL* ioctls\n" );
		}

		if( INVALID_SOCKET != s )
		{
			closesocket( s );
			s = INVALID_SOCKET;
		}
		WSACleanup();
		return false;
	}

	msgbuf = (char*)malloc( 1024 * 1024 * 64 );

	puts("start working...");
	while( true )
	{
		wbuf.len = sizeof(rcvbuf);
		wbuf.buf = rcvbuf;
		dwFlags = 0;

		DWORD dwNumberOfLength = 0;
		rc = WSARecv( s, &wbuf, 1, &dwNumberOfLength, &dwFlags, NULL, NULL );
		if( rc < 0 )
		{
			printf( "WSARecv() failed: %d\n", WSAGetLastError() );
			break;
		}

		prase_ip( rcvbuf, rc );

		if( _kbhit() )
		{
			int ch = _getch();
			if( ch == 27 )
				break;
		}

		time_t now = time( nullptr );
		if( now - msgsav > 30 )
		{
			FILE* fp = nullptr;
			if( fopen_s( &fp, "sniffer.log", "a+" ) == 0 && fp )
			{
				u_long total = 0;
				char szDate[64], szTime[64];
				_strdate_s( szDate );
				_strtime_s( szTime );
				fprintf( fp, "=====================================================" );
				fprintf( fp, "%s %s report message status\n", szDate, szTime );

				for( size_t i = 0; i < _countof( msgvec ); ++i )
				{
					if( msgvec[i].count )
					{
						total += msgvec[i].total_byte;
						fprintf( fp, "msg : %8d, count : %6d, total_bytes : %8d\n", i, msgvec[i].count, msgvec[i].total_byte );
					}
				}

				fprintf( fp, "report finish total sent %lu\n", total );
				fclose( fp );
			}

			msgsav = now;
			memset( msgvec, 0, sizeof( msgvec ) );
		}
	}
	free( msgbuf );
	return 0;
}

void prase_ip( const char* packet, size_t size )
{
	// Check the IP version
	// Verify the buffer is large enough
	if (size  < sizeof(IPV4_HDR))
		return;

	ip4 = *(IPV4_HDR *)packet;

	if (ip4.version == 4)
	{
		if( src_peer.addr != 0 && src_peer.addr != ip4.src_addr && src_peer.addr != ip4.dst_addr )
		{
			ClearOutput();
			return;
		}

		switch( ip4.proto )
		{
		case IPPROTO_TCP:
			IN_ADDR src, dst;
			src.S_un.S_addr = ip4.src_addr;
			dst.S_un.S_addr = ip4.dst_addr;
			strcpy_s(src_addr, inet_ntoa(src));
			strcpy_s(dst_addr, inet_ntoa(dst));
			prase_tcp(packet + ip4.ihl * 4, size - ip4.ihl * 4);
			
			break;
		case IPPROTO_UDP:
			break;
		}
	}
}

void prase_tcp( const char* packet, size_t size )
{
	tcp = *(TCP_HDR*)packet;

	tcp.src_port = ntohs(tcp.src_port);
	tcp.dst_port = ntohs(tcp.dst_port);
	tcp.seq_num = ntohl(tcp.seq_num);
	tcp.ack_num = ntohl(tcp.ack_num);
	tcp.chk_sum = ntohs(tcp.chk_sum);

	size_t tcp_hdrlen = tcp.thl * 4;

	if (src_peer.port != 0 && tcp.src_port != src_peer.port && tcp.dst_port != src_peer.port)
	{
		return;
	}

	TCP_CONNECTION Conn;
	Conn.src.addr = ip4.src_addr;
	Conn.src.port = tcp.src_port;

	Conn.dst.addr = ip4.dst_addr;
	Conn.dst.port = tcp.dst_port;

	auto iter = sessions.find(Conn);
	if (iter == sessions.end())
	{
		session = (PTCP_SESSION)malloc( sizeof(TCP_SESSION) );
		memset(session, 0, _msize(session));
		sessions[Conn] = session;
	}
	else
	{
		session = iter->second;
	}

	if (tcp.flag & TH_PUSH )
	{
		if( tcp.seq_num > session->seq_num )
		{
			session->seq_num = tcp.seq_num;

			prase_pkg(packet + tcp_hdrlen, size - tcp_hdrlen, prase_msg);

			//PTCP_UNORDER cursor = session->unorder_list;
			//PTCP_UNORDER fefefe = session->unorder_list;
			//while (cursor != nullptr)
			//{
			//	if (cursor->tcp.seq_num != tcp.seq_num)
			//		break;

			//	++session->seq_num;
			//	prase_pkg(cursor->data, cursor->len, prase_msg);

			//	cursor = cursor->next;

			//	free(fefefe);
			//	fefefe = cursor;
			//}
		}
		//else
		//{
		//	PTCP_UNORDER cursor = session->unorder_list;

		//	// create new unorder packet
		//	size_t datalen = size - tcp_hdrlen;
		//	if (datalen > 0)
		//	{
		//		PTCP_UNORDER newest = (PTCP_UNORDER)malloc(sizeof(TCP_UNORDER)+datalen);
		//		newest->len = datalen;
		//		newest->next = nullptr;
		//		newest->tcp = tcp;
		//		memcpy(newest->data, packet + tcp_hdrlen, size - tcp_hdrlen);

		//		// insert order by asc
		//		if (cursor == nullptr)
		//		{
		//			session->unorder_list = newest;
		//		}
		//		else if (cursor->tcp.seq_num > newest->tcp.seq_num)
		//		{
		//			newest->next = cursor;
		//			session->unorder_list = newest;
		//		}
		//		else while (cursor)
		//		{
		//			if (cursor->next == nullptr || cursor->next->tcp.seq_num > newest->tcp.seq_num)
		//			{
		//				newest->next = cursor->next;
		//				cursor->next = newest;
		//				break;
		//			}
		//			cursor = cursor->next;
		//		}
		//	}
		//}
	}
}

void prase_pkg( const char* packet, size_t size, prase_fn decode )
{
	// 协议包搜索算法
	memcpy( session->msgbuf + session->msglen, packet, size);
	session->msglen += size;

	while (session->msglen >= sizeof(unsigned short))
	{
		unsigned short mark = *(unsigned short*)session->msgbuf;

		if(mark == 0xaaee && session->msglen >= sizeof(pkghead16))
		{
			// head16
			pkghead16 pkg = *(pkghead16*)packet;

			if( pkg.chk != ( ( pkg.len ^ 0xBBCC ) & 0x88AA ) )
			{
				memmove( session->msgbuf, session->msgbuf + 2, session->msglen - 2 );
				session->msglen -= 2;
				continue;
			}

			if( session->msglen < pkg.len + sizeof( pkghead16 ) )
				break;

			decode( packet + sizeof(pkghead16), pkg.len );
			memmove(session->msgbuf, session->msgbuf + pkg.len + sizeof(pkghead16), session->msglen - pkg.len - sizeof(pkghead16));
			session->msglen = session->msglen - pkg.len - sizeof(pkghead16);
		}
		else if (mark == 0xaaef && session->msglen >= sizeof(pkghead32))
		{
			// head32
			pkghead32 pkg	= *(pkghead32*)msgbuf;
			pkg.mark = ntohs(pkg.mark);
			pkg.len = ntohl(pkg.len);
			pkg.chk = ntohs(pkg.chk);

			if( pkg.chk != ( ( ( pkg.len >> 16 ) ^ ( pkg.len ) ^ 0xBBCC ) & 0x88AA ) )
			{
				memmove( session->msgbuf, session->msgbuf + 2, session->msglen - 2 );
				session->msglen -= 2;
				continue;
			}

			if (session->msglen >= sizeof(pkghead32)+pkg.len)
			{
				decode(session->msgbuf + sizeof(pkghead32), session->msglen - sizeof(pkghead32));
				memmove(session->msgbuf, session->msgbuf + pkg.len + sizeof(pkghead32), session->msglen - pkg.len - sizeof(pkghead32));
				session->msglen = session->msglen - pkg.len - sizeof(pkghead32);
			}
		}
		else
		{
			memmove( session->msgbuf, session->msgbuf + 2, session->msglen - 2 );
			session->msglen -= 2;
		}
	}
}

#define ID_CONNECT_REQ	1
#define ID_CONNECT_ACK	2
#define ID_DATA			10
#define ID_PING			11

#define DATA_MARK		0x66bb

void prase_pipe( const char* packet, size_t size )
{
	unsigned short msgid = htons( *(unsigned short*)packet );
	switch( msgid )
	{
	case ID_CONNECT_REQ:
		break;
	case ID_CONNECT_ACK:
		break;
	case ID_DATA:
		{
			if( size < sizeof( data_header ) )
				return;

			data_header head = *(data_header*)packet;
			head.business = ntohs( head.business );
			head.length = ntohs( head.length );
			head.mark = ntohs( head.mark );

			if( head.mark != DATA_MARK )
				return;

			if( head.length != size - sizeof( data_header ) )
				return;

			prase_msg( packet + sizeof( data_header ), size - sizeof( data_header ) );
			break;
		}
	case ID_PING:
		break;
	}
}

void prase_msg( const char* packet, size_t size )
{
	if( size < sizeof( pkghead ) + sizeof( msghead ) )
		return;

	packet += 3;
	size -= 3;
	pkghead pkg = *(pkghead*)( packet );
	msghead msg = *(msghead*)( packet + sizeof(pkg) );

	packet += sizeof(pkg)+sizeof(msg);
	size -= sizeof(pkg)+sizeof( msg );

	pkg._length = ntohs( pkg._length );

	msg._class = ntohs( msg._class );
	msg._message = ntohs( msg._message );

	msgvec[msg._message].count += 1;
	msgvec[msg._message].total_byte += pkg._length;

	printf( "sniffer message = %u, length = %u\n", msg._message, pkg._length );
}