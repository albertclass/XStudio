#include "header.h"
#include "gate.pb.h"

#include "GameSrvSession.h"
#include "ChatSrvSession.h"

CGameSrvSession::CGameSrvSession( xgc_lpcstr username, xgc_lpcstr password )
	: handle_( INVALID_NETWORK_HANDLE )
	, pingpong_( 0 )
	, pinglast_( ticks< std::chrono::milliseconds >() )
	, mChatSrvSession( xgc_nullptr )
	, mUsername( username )
	, mPassword( password )
{
}

CGameSrvSession::~CGameSrvSession()
{
}

int CGameSrvSession::OnParsePacket( const void * data, xgc_size size )
{
	if( size < sizeof( MessageHeader ) )
		return 0;

	xgc_uint16 length = ntohs( *(xgc_uint16*) data );
	if( size < length )
		return 0;

	return length;
}

xgc_void CGameSrvSession::OnAccept( net::network_t handle )
{
	handle_ = handle;
	//fprintf( stdout, "net session %u accepted\r\n", handle_ );
}

xgc_void CGameSrvSession::OnConnect( net::network_t handle )
{
	handle_ = handle;
	//fprintf( stdout, "net session %u connected\r\n", handle_ );

	gate::login_req req;
	req.set_username( mUsername );
	req.set_password( mPassword );

	Send2GameSrv( gate::GATE_MSG_LOGIN_REQ, req );
}

xgc_void CGameSrvSession::OnError( xgc_uint32 error_code )
{
	fprintf( stderr, "net session %0X error, code = %0X\r\n", handle_, error_code );
}

xgc_void CGameSrvSession::OnClose()
{
	//fprintf( stderr, "net session %u closed\r\n", handle_ );
	handle_ = INVALID_NETWORK_HANDLE;

	if( mChatSrvSession )
	{
		mChatSrvSession->Disconnect();
	}
	else
	{
		delete this;
	}
}

xgc_void CGameSrvSession::OnAlive()
{
}

xgc_void CGameSrvSession::OnRecv( xgc_lpvoid data, xgc_size size )
{
	MessageHeader &header = *(MessageHeader*)data;
	auto length = htons( header.length );
	auto message = htons( header.message );

	auto ptr = (xgc_lpstr)data + sizeof( MessageHeader );
	auto len = (int)( size - sizeof( MessageHeader ) );

	switch( message )
	{
		case gate::GATE_MSG_LOGIN_ACK:
		{
			gate::login_ack ack;
			ack.ParseFromArray( ptr, len );

			if( ack.result() >= 0 )
			{
				mChatSrvSession = XGC_NEW CChatSrvSession( this, ack.user_id(), ack.chat_token() );
				if( mChatSrvSession )
				{
					connect_options options_c;
					memset( &options_c, 0, sizeof( options_c ) );

					options_c.timeout = 1000;
					options_c.is_async = true;

					auto hNet = Connect(
						"127.0.0.1",
						50001,
						mChatSrvSession, 
						&options_c );

					if( hNet == INVALID_NETWORK_HANDLE )
					{
						SAFE_DELETE( mChatSrvSession );
					}
				}
			}
			else
			{
				CloseLink( handle_ );
			}
		}
		break;
	}
}

///
/// \brief 聊天服务器连接建立
///
/// \author albert.xu
/// \date 2017/08/05
///

xgc_void CGameSrvSession::OnChatConnect( network_t handle )
{
	mChatSrvSession->ChatUserAuth();
}


///
/// \brief 聊天服务器连接断开
///
/// \author albert.xu
/// \date 2017/08/05
///

xgc_void CGameSrvSession::OnChatClose()
{
	mChatSrvSession = xgc_nullptr;
	if( handle_ == INVALID_NETWORK_HANDLE )
		delete this;
}

xgc_void CGameSrvSession::Send2GameSrv( xgc_uint16 msgid, ::google::protobuf::Message& msg )
{
#pragma pack(1)
	struct
	{
		MessageHeader h;
		char b[1024 * 4];
	} m;
#pragma pack()

	xgc_uint16 data_size = msg.ByteSize();
	xgc_uint16 pack_size = sizeof( MessageHeader ) + data_size;

	m.h.length = htons( pack_size );
	m.h.message = htons( msgid );

	msg.SerializeToArray( m.b, sizeof( m.b ) );

	SendPacket( handle_, &m, pack_size );
}

///
/// \brief 发送数据到游戏服务器
///
/// \author albert.xu
/// \date 2017/08/5
///
xgc_void CGameSrvSession::Send2ChatSrv( xgc_uint16 msgid, ::google::protobuf::Message & msg )
{
	if( mChatSrvSession )
		mChatSrvSession->Send2ChatSrv( msgid, msg );
}

///
/// \brief 断开聊天服务器连接
///
/// \author albert.xu
/// \date 2017/08/09
///
xgc_void CGameSrvSession::Disconnect()
{
	CloseLink( handle_ );
	if( mChatSrvSession )
		mChatSrvSession->Disconnect();
}
