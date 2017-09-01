#include "frame.h"
#include "net.h"
#include <thread>
#include <chrono>
#include "xbuffer.h"
#include "serialization.h"

using namespace xgc::net;
using namespace xgc::common;

#if defined(_LINUX)
#	include <arpa/inet.h>
#	include <endian.h>
#endif

#if defined(_WINDOWS)
#	include <WinSock2.h>
#	pragma comment( lib, "ws2_32.lib" )
#endif

class CNetworkSession : public INetworkSession
{
public:
	CNetworkSession()
	{

	}

	///
	/// \brief 数据包是否
	/// \return	0 ~ 成功, -1 ~ 失败
	///
	virtual int OnParsePacket( const void* data, xgc_size size )
	{
		if( size > 2 )
		{
			int size = ntohs( *(const xgc_uint16*) data );
			return size;
		}

		return 0;
	}

	///
	/// \brief 连接建立
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnAccept( network_t handle )
	{

	}

	///
	/// \brief 连接建立
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnConnect( network_t handle )
	{

	}

	///
	/// \brief 连接错误
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnError( xgc_int16 error_type, xgc_int16 error_code )
	{

	}

	///
	/// \brief 连接关闭
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:10
	///
	virtual xgc_void OnClose()
	{

	}

	///
	/// \brief 接收数据
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:10
	///
	virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size )
	{

	}

	///
	/// \brief 网络保活事件
	///
	/// \author albert.xu
	/// \date 2017/03/03 10:41
	///
	virtual xgc_void OnAlive()
	{

	}
};

INetworkSession* CreateServerSession()
{
	return XGC_NEW CNetworkSession();
}

static void server()
{
	server_options options;
	memset( &options, 0, sizeof options );

	auto srv = StartServer( "0.0.0.0", 25000, &options, CreateServerSession );

	while( true )
	{
		if( ProcessNetEvent( 10 ) == 10 )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds(1) );
		}
	}

	CloseServer( srv );
}

static void client()
{
	connect_options options;
	memset( &options, 0, sizeof options );

	if( Connect( "127.0.0.1", 25000, XGC_NEW CNetworkSession(), &options ) )
	{
		while(true )
		{
			ProcessNetEvent( 10 );
		}
	}
}

static int testmain( int agrc, char * argv[] )
{
	CreateNetwork( 16 );

	std::thread tsvr( server );
	std::thread tcli( client );

	tcli.join();
	tsvr.join();

	DestroyNetwork();
	return 0;
}

UNIT_TEST( "network", "test network", testmain );
