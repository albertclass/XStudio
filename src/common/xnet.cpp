#include "defines.h"
#ifdef _WINDOWS
//#include "Win32_Interop/Win32_Portability.h"
//#include "Win32_Interop/win32_types.h"
//#include "Win32_Interop/win32fixes.h"
//#include "Win32_Interop/win32_wsiocp2.h"
#include <ws2tcpip.h>
#include <fcntl.h>
#include <Mstcpip.h>
#define	O_NONBLOCK	0x0004
#pragma comment( lib, "ws2_32.lib" )
#endif

#include <sys/types.h>
#ifdef _LINUX
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#endif
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#include "xnet.h"

namespace xgc
{
	namespace common
	{
#ifdef _WINDOWS
		struct __init_winsock
		{
			__init_winsock()
			{
				WSADATA t_wsa;
				WORD wVers;
				int iError;

				wVers = MAKEWORD( 2, 2 );
				iError = WSAStartup( wVers, &t_wsa );

				if( iError != NO_ERROR || LOBYTE( t_wsa.wVersion ) != 2 || HIBYTE( t_wsa.wVersion ) != 2 )
				{
					exit( 1 );
				}

				atexit( (void (*)() )WSACleanup );
			}
		};

		static __init_winsock __sock_init__;
#endif
		static void set_error( char *err, const char *fmt, ... )
		{
			va_list ap;

			if( !err ) return;
			va_start( ap, fmt );
			vsnprintf( err, XNET_ERR_LEN, fmt, ap );
			va_end( ap );
		}

		///
		/// \brief ÉèÖÃ sock ×èÈû×´Ì¬
		/// \date 1/8/2018
		/// \author albert.xu
		///
		static int set_block( char *err, SOCKET s, int non_block )
		{
#ifdef _LINUX
			int flags;

			if( ( flags = fcntl( s, F_GETFL, 0 ) ) == -1 )
			{
				set_error( err, "fcntl(F_GETFL): %s", strerror( errno ) );
				return XNET_ERR;
			}

			if( non_block )
				flags |= O_NONBLOCK;
			else
				flags &= ~O_NONBLOCK;

			if( fcntl( s, F_SETFL, flags ) == -1 )
			{
				set_error( err, "fcntl(F_SETFL,O_NONBLOCK): %s", strerror( errno ) );
				return XNET_ERR;
			}
#endif

#ifdef _WINDOWS
			u_long flags = (u_long)non_block;
			if( -1 == ioctlsocket( s, FIONBIO, &flags ) )
			{
				set_error( err, "ioctlsocket( s, FIONBIO, &flags ): %s", strerror( errno ) );
				return XNET_ERR;
			}
#endif

			return XNET_OK;
		}

		int net_nonblock( char *err, SOCKET s )
		{
			return set_block( err, s, 1 );
		}

		int net_block( char *err, SOCKET s )
		{
			return set_block( err, s, 0 );
		}

		/* Set TCP keep alive option to detect dead peers. The interval option
		 * is only used for Linux as we are using Linux-specific APIs to set
		 * the probe send time, interval, and count. */
		int net_keepalive( char *err, SOCKET s, int interval )
		{
			int val = 1;

#ifdef _WINDOWS    
			if( setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, (char*)&val, sizeof( val ) ) == -1 )
			{
				set_error( err, "setsockopt SO_KEEPALIVE: %s", strerror( errno ) );
				return XNET_ERR;
			}

			struct tcp_keepalive alive;
			DWORD dwBytesRet = 0;
			alive.onoff = TRUE;
			alive.keepalivetime = interval * 1000;
			/* According to
			 * http://msdn.microsoft.com/en-us/library/windows/desktop/ee470551(v=vs.85).aspx
			 * On Windows Vista and later, the number of keep-alive probes (data
			 * retransmissions) is set to 10 and cannot be changed.
			 * So we set the keep alive interval as interval/10, as 10 probes will
			 * be send before detecting an error */
			val = interval / 10;
			if( val == 0 ) val = 1;
			alive.keepaliveinterval = val * 1000;
			if( WSAIoctl( s, SIO_KEEPALIVE_VALS, &alive, sizeof( alive ), NULL, 0, &dwBytesRet, NULL, NULL ) == SOCKET_ERROR )
			{
				set_error( err,
					"WSAIotcl(SIO_KEEPALIVE_VALS) failed with error code %d\n",
					strerror( errno ) );
				return XNET_ERR;
			}
#endif

#ifdef _LINUX
			/* Default settings are more or less garbage, with the keepalive time
			 * set to 7200 by default on Linux. Modify settings to make the feature
			 * actually useful. */

			 /* Send first probe after interval. */
			val = interval;
			if( setsockopt( s, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof( val ) ) < 0 )
			{
				set_error( err, "setsockopt TCP_KEEPIDLE: %s\n", strerror( errno ) );
				return XNET_ERR;
			}

			/* Send next probes after the specified interval. Note that we set the
			 * delay as interval / 3, as we send three probes before detecting
			 * an error (see the next setsockopt call). */
			val = interval / 3;
			if( val == 0 ) val = 1;
			if( setsockopt( s, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof( val ) ) < 0 )
			{
				set_error( err, "setsockopt TCP_KEEPINTVL: %s\n", strerror( errno ) );
				return XNET_ERR;
			}

			/* Consider the socket in error state after three we send three ACK
			 * probes without getting a reply. */
			val = 3;
			if( setsockopt( s, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof( val ) ) < 0 )
			{
				set_error( err, "setsockopt TCP_KEEPCNT: %s\n", strerror( errno ) );
				return XNET_ERR;
			}
#else
			( (void)interval ); /* Avoid unused var warning for non Linux systems. */
#endif

			return XNET_OK;
		}

		int net_tcp_nodelay( char *err, SOCKET s, int val )
		{
			if( setsockopt( s, IPPROTO_TCP, TCP_NODELAY, (char*)&val, sizeof( val ) ) == -1 )
			{
				set_error( err, "setsockopt TCP_NODELAY: %s", strerror( errno ) );
				return XNET_ERR;
			}
			return XNET_OK;
		}

		int net_tcp_keepalive( char *err, SOCKET s )
		{
			int yes = 1;
			if( setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, (char*)&yes, sizeof( yes ) ) == -1 )
			{
				set_error( err, "setsockopt SO_KEEPALIVE: %s", strerror( errno ) );
				return XNET_ERR;
			}
			return XNET_OK;
		}

		int net_send_buffer( char *err, SOCKET s, int buffsize )
		{
			if( setsockopt( s, SOL_SOCKET, SO_SNDBUF, (char*)&buffsize, sizeof( buffsize ) ) == -1 )
			{
				set_error( err, "setsockopt SO_SNDBUF: %s", strerror( errno ) );
				return XNET_ERR;
			}
			return XNET_OK;
		}

		/* Set the socket send timeout (SO_SNDTIMEO socket option) to the specified
		 * number of milliseconds, or disable it if the 'ms' argument is zero. */
		int net_send_timeout( char *err, SOCKET s, int64_t ms )
		{
			struct timeval tv;

			tv.tv_sec = (int)ms / 1000;
			tv.tv_usec = ( ms % 1000 ) * 1000;

			if( setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof( tv ) ) == -1 )
			{
				set_error( err, "setsockopt SO_SNDTIMEO: %s", strerror( errno ) );
				return XNET_ERR;
			}
			return XNET_OK;
		}

		/* net_generic_resolve() is called by anetResolve() and anetResolveIP() to
		 * do the actual work. It resolves the hostname "host" and set the string
		 * representation of the IP address into the buffer pointed by "ipbuf".
		 *
		 * If flags is set to XNET_IP_ONLY the function only resolves hostnames
		 * that are actually already IPv4 or IPv6 addresses. This turns the function
		 * into a validating / normalizing function. */
		int net_generic_resolve( char *err, char *host, char *ipbuf, size_t ipbuf_len, int flags )
		{
			struct addrinfo hints, *info;
			int rv;

			memset( &hints, 0, sizeof( hints ) );

			hints.ai_flags    = ( flags & XNET_IP_ONLY ) ? AI_NUMERICHOST : 0;
			hints.ai_family   = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;  /* specify socktype to avoid dups */

			if( ( rv = getaddrinfo( host, NULL, &hints, &info ) ) != 0 )
			{
				set_error( err, "%s", gai_strerror( rv ) );
				return XNET_ERR;
			}
			if( info->ai_family == AF_INET )
			{
				struct sockaddr_in *sa = ( struct sockaddr_in * )info->ai_addr;
				inet_ntop( AF_INET, &( sa->sin_addr ), ipbuf, ipbuf_len );
			}
			else
			{
				struct sockaddr_in6 *sa = ( struct sockaddr_in6 * )info->ai_addr;
				inet_ntop( AF_INET6, &( sa->sin6_addr ), ipbuf, ipbuf_len );
			}

			freeaddrinfo( info );
			return XNET_OK;
		}

		int net_resolve( char *err, char *host, char *ipbuf, size_t ipbuf_len )
		{
			return net_generic_resolve( err, host, ipbuf, ipbuf_len, XNET_NONE );
		}

		int net_resolve_ip( char *err, char *host, char *ipbuf, size_t ipbuf_len )
		{
			return net_generic_resolve( err, host, ipbuf, ipbuf_len, XNET_IP_ONLY );
		}

		int net_reuse_addr( char *err, SOCKET s )
		{
			int yes = 1;
			/* Make sure connection-intensive things like the redis benckmark
			 * will be able to close/open sockets a zillion of times */
			if( setsockopt( s, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof( yes ) ) == -1 )
			{
				set_error( err, "setsockopt SO_REUSEADDR: %s", strerror( errno ) );
				return XNET_ERR;
			}
			return XNET_OK;
		}

		static SOCKET new_socket( char *err, int domain )
		{
			SOCKET s;
			if( ( s = socket( domain, SOCK_STREAM, IF_WINDOWS( IPPROTO_TCP, 0 ) ) ) == -1 )
			{
				set_error( err, "creating socket: %s", strerror( errno ) );
				return XNET_ERR;
			}

			/* Make sure connection-intensive things like the redis benchmark
			 * will be able to close/open sockets a zillion of times */
			if( net_reuse_addr( err, s ) == XNET_ERR )
			{
				closesocket( s );
				return XNET_ERR;
			}
			return s;
		}

		static SOCKET tcp_generic_connect( char *err, const char *addr, int port, char *source_addr, int nonblock )
		{
			SOCKET s = XNET_ERR;
			int rv;
			char portstr[6];  /* strlen("65535") + 1; */
			struct addrinfo hints, *servinfo, *bservinfo, *p, *b;

			snprintf( portstr, sizeof( portstr ), "%d", port );
			memset( &hints, 0, sizeof( hints ) );
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;

			if( ( rv = getaddrinfo( addr, portstr, &hints, &servinfo ) ) != 0 )
			{
				set_error( err, "%s", gai_strerror( rv ) );
				return XNET_ERR;
			}

			for( p = servinfo; p != NULL; p = p->ai_next )
			{
				/* Try to create the socket and to connect it.
				 * If we fail in the socket() call, or on connect(), we retry with
				 * the next entry in servinfo. */
				if( ( s = socket( p->ai_family, p->ai_socktype, p->ai_protocol ) ) == -1 )
					continue;

				if( net_reuse_addr( err, s ) == XNET_ERR )
					goto error;

				if( nonblock && net_nonblock( err, s ) != XNET_OK )
					goto error;

				if( source_addr )
				{
					int bound = 0;
					/* Using getaddrinfo saves us from self-determining IPv4 vs IPv6 */
					if( ( rv = getaddrinfo( source_addr, NULL, &hints, &bservinfo ) ) != 0 )
					{
						set_error( err, "%s", gai_strerror( rv ) );
						goto error;
					}
					for( b = bservinfo; b != NULL; b = b->ai_next )
					{
						if( bind( s, b->ai_addr, (int)b->ai_addrlen ) != -1 )
						{
							bound = 1;
							break;
						}
					}

					freeaddrinfo( bservinfo );
					if( !bound )
					{
						set_error( err, "bind: %s", strerror( errno ) );
						goto error;
					}
				}

				if( connect( s, p->ai_addr, (int)p->ai_addrlen ) == -1 )
				{
					/* If the socket is non-blocking, it is ok for connect() to
					 * return an EINPROGRESS error here. */
					int err = errno;
					if( err == IF_WINDOWS(0, EINPROGRESS) && nonblock )
						goto end;

					closesocket( s );
					s = XNET_ERR;
					continue;
				}

				/* If we ended an iteration of the for loop without errors, we
				 * have a connected socket. Let's return to the caller. */
				goto end;
			}
			if( p == NULL )
				set_error( err, "creating socket: %s", strerror( errno ) );

		error:
			if( s != XNET_ERR )
			{
				closesocket( s );
				s = XNET_ERR;
			}

		end:
			freeaddrinfo( servinfo );

			/* Handle best effort binding: if a binding address was used, but it is
			 * not possible to create a socket, try again without a binding address. */
			if( s == XNET_ERR && source_addr )
			{
				return tcp_generic_connect( err, addr, port, NULL, nonblock );
			}
			else
			{
				return s;
			}
		}

		SOCKET tcp_connect( char *err, const char *addr, int port, int nonblock )
		{
			return tcp_generic_connect( err, addr, port, NULL, nonblock );
		}

		SOCKET tcp_connect_bind( char *err, const char *addr, int port, char *source_addr, int nonblock )
		{
			return tcp_generic_connect( err, addr, port, source_addr, nonblock );
		}

		int net_recv( SOCKET s, char *buf, int count )
		{
			int nread, totlen = 0;
			while( totlen != count )
			{
				nread = recv( s, buf, count - totlen, 0 );

				if( nread == -1 )
				{
					if( errno != 0 )
					{
						fprintf( stderr, strerror( errno ) );
						return -1;
					}
					else
					{
						nread = 0;
					}
				}

				if( nread == 0 )
					return (int)totlen;

				totlen += nread;
				buf += nread;
			}

			return (int)totlen;
		}

		int net_send( SOCKET s, char *buf, int count )
		{
			int nwritten, totlen = 0;
			while( totlen != count )
			{
				nwritten = send( s, buf, count - totlen, 0 );
				if( nwritten == 0 )
					return (int)totlen;

				if( nwritten == -1 )
					return -1;

				totlen += nwritten;
				buf += nwritten;
			}

			return (int)totlen;
		}

		static int net_listen( char *err, SOCKET s, struct sockaddr *sa, socklen_t len, int backlog )
		{
			if( bind( s, sa, len ) == -1 )
			{
				set_error( err, "bind: %s", strerror( errno ) );
				closesocket( s );
				return XNET_ERR;
			}

			if( listen( s, backlog ) == -1 )
			{
				set_error( err, "listen: %s", strerror( errno ) );
				closesocket( s );
				return XNET_ERR;
			}
			return XNET_OK;
		}

		static int net_ipv6_only( char *err, SOCKET s )
		{
			int yes = 1;
			if( setsockopt( s, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&yes, sizeof( yes ) ) == -1 )
			{
				set_error( err, "setsockopt: %s", strerror( errno ) );
				closesocket( s );
				return XNET_ERR;
			}
			return XNET_OK;
		}

#ifdef _WIN32
		static int net_exclusive_addr( char *err, SOCKET s )
		{
			int yes = 1;
			/* Make sure connection-intensive things like the redis benchmark
			 * will be able to close/open sockets a zillion of times */
			if( setsockopt( s, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&yes, sizeof( yes ) ) == -1 )
			{
				set_error( err, "setsockopt SO_EXCLUSIVEADDRUSE: %s", strerror( errno ) );
				return XNET_ERR;
			}
			return XNET_OK;
		}
#endif

		static SOCKET net_tcp_server( char *err, int port, char *bindaddr, int af, int backlog )
		{
			SOCKET s;
			int rv;

			char _port[6];  /* strlen("65535") */
			struct addrinfo hints, *servinfo, *p;

			sprintf_s( _port, "%d", port );

			memset( &hints, 0, sizeof( hints ) );
			hints.ai_family = af;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_PASSIVE;    /* No effect if bindaddr != NULL */

			if( ( rv = getaddrinfo( bindaddr, _port, &hints, &servinfo ) ) != 0 )
			{
				set_error( err, "%s", gai_strerror( rv ) );
				return XNET_ERR;
			}

			for( p = servinfo; p != NULL; p = p->ai_next )
			{
				if( ( s = socket( p->ai_family, p->ai_socktype, p->ai_protocol ) ) == -1 )
					continue;

				if( af == AF_INET6 && net_ipv6_only( err, s ) == XNET_ERR )
					goto error;

				if( IF_WINDOWS( net_exclusive_addr, net_reuse_addr )( err, s ) == XNET_ERR )
					goto error;

				if( net_listen( err, s, p->ai_addr, (socklen_t)p->ai_addrlen, backlog ) == XNET_ERR )
					goto error;
				
				goto end;
			}

			if( p == NULL )
			{
				set_error( err, "unable to bind socket, errno: %d", errno );
				goto error;
			}

		error:
			s = XNET_ERR;

		end:
			freeaddrinfo( servinfo );
			return s;
		}

		SOCKET tcp_server( char *err, int port, char *bindaddr, int backlog )
		{
			return net_tcp_server( err, port, bindaddr, AF_INET, backlog );
		}

		SOCKET tcp_server6( char *err, int port, char *bindaddr, int backlog )
		{
			return net_tcp_server( err, port, bindaddr, AF_INET6, backlog );
		}

		static SOCKET net_generic_accept( char *err, SOCKET s, struct sockaddr *sa, socklen_t *len )
		{
			SOCKET sock;
			while( 1 )
			{
				sock = accept( s, sa, len );
				if( sock == -1 )
				{
					if( errno == EINTR )
						continue;
					else
					{
						set_error( err, "accept: %s", strerror( errno ) );
						return XNET_ERR;
					}
				}
				break;
			}
			return sock;
		}

		SOCKET tcp_accept( char *err, SOCKET s, char *ip, size_t ip_len, int *port )
		{
			SOCKET sock;
			struct sockaddr_storage sa;
			socklen_t salen = sizeof( sa );
			if( ( sock = net_generic_accept( err, s, ( struct sockaddr* )&sa, &salen ) ) == -1 )
				return XNET_ERR;

			if( sa.ss_family == AF_INET )
			{
				struct sockaddr_in *s = ( struct sockaddr_in * )&sa;
				if( ip )
					inet_ntop( AF_INET, ( void* )&( s->sin_addr ), ip, ip_len );
				if( port )
					*port = ntohs( s->sin_port );
			}
			else
			{
				struct sockaddr_in6 *s = ( struct sockaddr_in6 * )&sa;
				if( ip ) inet_ntop( AF_INET6, ( void* )&( s->sin6_addr ), ip, ip_len );
				if( port ) *port = ntohs( s->sin6_port );
			}
			return sock;
		}

		int net_peer2string( SOCKET s, char *ip, size_t ip_len, int *port )
		{
			struct sockaddr_storage sa;
			socklen_t salen = sizeof( sa );

			if( getpeername( s, ( struct sockaddr* )&sa, &salen ) == -1 )
				goto error;

			if( ip_len == 0 )
				goto error;

			if( sa.ss_family == AF_INET )
			{
				struct sockaddr_in *sa4 = ( struct sockaddr_in * )&sa;
				if( ip )
					inet_ntop( AF_INET, ( void* )&( sa4->sin_addr ), ip, ip_len );

				if( port )
					*port = ntohs( sa4->sin_port );
			}
			else if( sa.ss_family == AF_INET6 )
			{
				struct sockaddr_in6 *sa6 = ( struct sockaddr_in6 * )&sa;
				if( ip )
					inet_ntop( AF_INET6, ( void* )&( sa6->sin6_addr ), ip, ip_len );

				if( port )
					*port = ntohs( sa6->sin6_port );
			}
			else if( sa.ss_family == AF_UNIX )
			{
				if( ip )
					strncpy( ip, "/unixsocket", ip_len );

				if( port )
					*port = 0;
			}
			else
			{
				goto error;
			}
			return 0;

		error:
			if( ip )
			{
				if( ip_len >= 2 )
				{
					ip[0] = '?';
					ip[1] = '\0';
				}
				else if( ip_len == 1 )
				{
					ip[0] = '\0';
				}
			}

			if( port )
			{
				*port = 0;
			}

			return -1;
		}

		/* Format an IP,port pair into something easy to parse. If IP is IPv6
		 * (matches for ":"), the ip is surrounded by []. IP and port are just
		 * separated by colons. This the standard to display addresses within Redis. */
		int net_format_addr( char *buf, size_t buf_len, char *ip, int port )
		{
			return snprintf( buf, buf_len, strchr( ip, ':' ) ?
				"[%s]:%d" : "%s:%d", ip, port );
		}

		/* Like anetFormatAddr() but extract ip and port from the socket's peer. */
		int net_format_peer( SOCKET s, char *buf, size_t buf_len )
		{
			char ip[INET6_ADDRSTRLEN];
			int port;

			net_peer2string( s, ip, sizeof( ip ), &port );
			return net_format_addr( buf, buf_len, ip, port );
		}

		int net_sockname( SOCKET s, char *ip, size_t ip_len, int *port )
		{
			struct sockaddr_storage sa;
			socklen_t salen = sizeof( sa );

			if( getsockname( s, ( struct sockaddr* )&sa, &salen ) == -1 )
			{
				if( port ) *port = 0;
				ip[0] = '?';
				ip[1] = '\0';
				return -1;
			}
			if( sa.ss_family == AF_INET )
			{
				struct sockaddr_in *sa4 = ( struct sockaddr_in * )&sa;
				if( ip )
					inet_ntop( AF_INET, ( void* )&( sa4->sin_addr ), ip, ip_len );

				if( port )
					*port = ntohs( sa4->sin_port );
			}
			else
			{
				struct sockaddr_in6 *sa6 = ( struct sockaddr_in6 * )&sa;
				if( ip )
					inet_ntop( AF_INET6, ( void* )&( sa6->sin6_addr ), ip, ip_len );

				if( port )
					*port = ntohs( sa6->sin6_port );
			}
			return 0;
		}

		int net_format_sock( SOCKET s, char *fmt, size_t fmt_len )
		{
			char ip[INET6_ADDRSTRLEN];
			int port;

			net_sockname( s, ip, sizeof( ip ), &port );
			return net_format_addr( fmt, fmt_len, ip, port );
		}
		COMMON_API int net_close( SOCKET s )
		{
			return closesocket( s );
		}
	}
}
