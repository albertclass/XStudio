#include "header.h"
#include "gate.pb.h"
#include "Server.h"
#include "CliSession.h"

CClientSession::CClientSession()
	: handle_( INVALID_NETWORK_HANDLE )
	, pingpong_( 0 )
	, pinglast_( ticks< std::chrono::milliseconds >() )
	, user_id_( -1 )
{
}

CClientSession::~CClientSession()
{
}

int CClientSession::OnParsePacket( const void * data, xgc_size size )
{
	if( size < sizeof( MessageHeader ) )
		return 0;

	xgc_uint16 length = ntohs( *(xgc_uint16*) data );
	if( size < length )
		return 0;

	return length;
}

xgc_void CClientSession::OnAccept( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u accepted\r\n", handle_ );
}

xgc_void CClientSession::OnConnect( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u connected\r\n", handle_ );
}

xgc_void CClientSession::OnError( xgc_uint32 error_code )
{
	fprintf( stderr, "net session %u error, code = %u\r\n", handle_, error_code );
}

xgc_void CClientSession::OnClose()
{
	fprintf( stderr, "net session %u closed\r\n", handle_ );
	user_id_ = -1;
	handle_ = INVALID_NETWORK_HANDLE;

	delete this;
}

xgc_void CClientSession::OnAlive()
{
}

xgc_void CClientSession::OnRecv( xgc_lpvoid data, xgc_size size )
{
	MessageHeader &header = *(MessageHeader*)data;
	auto length = htons( header.length );
	auto message = htons( header.message );

	auto ptr = (xgc_lpstr)data + sizeof( MessageHeader );
	auto len = (xgc_int32)( size - sizeof( MessageHeader ) );

	switch( message )
	{
		case gate::GATE_MSG_LOGIN_REQ:
		{
			gate::login_req req;
			req.ParseFromArray( ptr, len );

			auto ret = theServer.VerificationUser( req.username(), req.password(), user_id_ );
			if( ret > 0 )
			{
				theServer.UserLogin( user_id_, req.username() );
			}
			else
			{
				gate::login_ack ack;
				ack.set_result( ret );

				Send( gate::GATE_MSG_LOGIN_ACK, ack );
			}
		}
		break;

		case gate::GATE_MSG_LOGOUT_REQ:
		{
			gate::logout_req req;
			req.ParseFromArray( ptr, len );

			theServer.UserLogout( user_id_ );

			CloseLink( handle_ );
		}
		break;
	}
}

///
/// \brief ·¢ËÍÊý¾Ý
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CClientSession::Send( xgc_uint16 msgid, ::google::protobuf::Message& msg )
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
