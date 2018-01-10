#ifndef __XNET_H__
#define __XNET_H__

#include <sys/types.h>

#if defined(_WINDOWS)
#include <WinSock2.h>
#elif defined(_LINUX)
#include <sys/socket.h>
#endif

#define XNET_OK 0
#define XNET_ERR -1
#define XNET_ERR_LEN 256

/* Flags used with certain functions. */
#define XNET_NONE 0
#define XNET_IP_ONLY (1<<0)

#if defined(__sun) || defined(_AIX)
#define AF_LOCAL AF_UNIX
#endif

#ifdef _AIX
#undef ip_len
#endif

#define NET_CONNECT_NONE       0
#define NET_CONNECT_NONBLOCK   1
#define NET_CONNECT_BE_BINDING 2 /* Best effort binding. */

namespace xgc
{
	namespace common
	{
		COMMON_API SOCKET tcp_connect( char *err, const char *addr, int port, int nonblock );
		COMMON_API SOCKET tcp_connect_bind( char *err, const char *addr, int port, char *source_addr, int nonblock );
		COMMON_API SOCKET tcp_server( char *err, int port, char *bindaddr, int backlog );
		COMMON_API SOCKET tcp_server6( char *err, int port, char *bindaddr, int backlog );
		COMMON_API SOCKET tcp_accept( char *err, SOCKET s, char *ip, size_t ip_len, int *port );
		COMMON_API int net_recv( SOCKET s, char *buf, int count );
		COMMON_API int net_send( SOCKET s, char *buf, int count );
		COMMON_API int net_resolve( char *err, char *host, char *ipbuf, size_t ipbuf_len );
		COMMON_API int net_resolve_ip( char *err, char *host, char *ipbuf, size_t ipbuf_len );
		COMMON_API int net_nonblock( char *err, SOCKET s );
		COMMON_API int net_block( char *err, SOCKET s );
		COMMON_API int net_tcp_nodelay( char *err, SOCKET s, int val );
		COMMON_API int net_tcp_keepalive( char *err, SOCKET s );
		COMMON_API int net_send_buffer( char *err, SOCKET s, int buffsize );
		COMMON_API int net_send_timeout( char *err, SOCKET s, int64_t ms );
		COMMON_API int net_peer2string( SOCKET s, char *ip, size_t ip_len, int *port );
		COMMON_API int net_keepalive( char *err, SOCKET s, int interval );
		COMMON_API int net_sockname( SOCKET s, char *ip, size_t ip_len, int *port );
		COMMON_API int net_format_addr( char *fmt, size_t fmt_len, char *ip, int port );
		COMMON_API int net_format_peer( SOCKET s, char *fmt, size_t fmt_len );
		COMMON_API int net_format_sock( SOCKET s, char *fmt, size_t fmt_len );
	}
}
#endif // __XNET_H__
