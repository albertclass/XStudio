#include "header.h"
#include "ChatSrvSession.h"

CChatSrvSession::CChatSrvSession( network_t gate_handle, xgc_uint64 user_id, xgc_uint32 chat_id, const xgc_string &token )
	: user_id_( user_id )
	, chat_id_( chat_id )
	, token_( token )
	, handle_( INVALID_NETWORK_HANDLE )
	, gate_handle_( gate_handle )
	, pingpong_( 0 )
	, pinglast_( ticks< std::chrono::milliseconds >() )
{
}

CChatSrvSession::~CChatSrvSession()
{
}

int CChatSrvSession::OnParsePacket( const void * data, xgc_size size )
{
	if( size < sizeof( MessageHeader ) )
		return 0;

	xgc_uint16 length = ntohs( *(xgc_uint16*) data );
	if( size < length )
		return 0;

	return length;
}

xgc_void CChatSrvSession::OnAccept( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u accepted\r\n", handle_ );

	ChatUserAuth();
}

xgc_void CChatSrvSession::OnConnect( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u connected\r\n", handle_ );
}

xgc_void CChatSrvSession::OnError( xgc_uint32 error_code )
{
	fprintf( stderr, "net session %0X error, code = %0X\r\n", handle_, error_code );
}

xgc_void CChatSrvSession::OnClose()
{
	fprintf( stderr, "net session %u closed\r\n", handle_ );
	ChatSrvClosed();
}

xgc_void CChatSrvSession::OnAlive()
{
}

xgc_void CChatSrvSession::OnRecv( xgc_lpvoid data, xgc_size size )
{
	MessageHeader &header = *(MessageHeader*) data;
	auto length  = htons( header.length );
	auto message = htons( header.message );

	auto ptr = (xgc_lpstr)data + sizeof( MessageHeader );
	auto len = (int)( size - sizeof( MessageHeader ) );

	switch( message )
	{
		case chat::MSG_USERAUTH_ACK:
		onUserAuthAck( ptr, len );
		break;
		case chat::MSG_CHANNEL_ENTER_NTF:
		onChannelEnterNtf( ptr, len );
		break;
		case chat::MSG_USERINFO_ACK:
		onUserInfoAck( ptr, len );
		break;
		case chat::MSG_CHANNEL_ENTER_ACK:
		onChannelEnterAck( ptr, len );
		break;
		case chat::MSG_CHANNEL_LEAVE_ACK:
		onchannelLeaveAck( ptr, len );
		break;
		case chat::MSG_USER_CHAT_NTF:
		onUserChatNtf( ptr, len );
		break;
		case chat::MSG_CHANNEL_CHAT_NTF:
		onChannelChatNtf( ptr, len );
		break;
		case chat::MSG_CHAT_ERR:
		onChatErr( ptr, len );
		break;
	}
}

///
/// \brief �����������֤
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::ChatUserAuth()
{
	chat::user_auth_req req;
	req.set_token( token_ );
	req.set_user_id( user_id_ );

	Send2ChatSrv( chat::MSG_USERAUTH_REQ, req );
}

///
/// \brief ����
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::ChatTo( xgc_uint32 nChatID, xgc_lpcstr lpMessage )
{

}

///
/// \brief ����
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::Say( xgc_uint32 nChannelID, xgc_lpcstr lpMessage )
{

}

///
/// \brief �û���֤��Ӧ
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onUserAuthAck( xgc_lpcstr ptr, xgc_size len )
{

}

///
/// \brief �ѽ���Ƶ����Ӧ
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onChannelEnterNtf( xgc_lpcstr ptr, xgc_size len )
{

}

///
/// \brief �û���Ϣ��Ӧ
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onUserInfoAck( xgc_lpcstr ptr, xgc_size len )
{

}

///
/// \brief ����Ƶ����Ӧ
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onChannelEnterAck( xgc_lpcstr ptr, xgc_size len )
{

}

///
/// \brief �뿪Ƶ����Ӧ
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onchannelLeaveAck( xgc_lpcstr ptr, xgc_size len )
{

}

///
/// \brief �û���������֪ͨ
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onUserChatNtf( xgc_lpcstr ptr, xgc_size len )
{

}

///
/// \brief Ƶ������֪ͨ
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onChannelChatNtf( xgc_lpcstr ptr, xgc_size len )
{

}

///
/// \brief �������֪ͨ
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onChatErr( xgc_lpcstr ptr, xgc_size len )
{

}

///
/// \brief ������������ӶϿ�
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::ChatSrvClosed()
{
	handle_ = INVALID_NETWORK_HANDLE;

	if( gate_handle_ == INVALID_NETWORK_HANDLE )
		delete this;
}

///
/// \brief ��Ϸ���������ӶϿ�
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::GateSrvClosed()
{
	gate_handle_ = INVALID_NETWORK_HANDLE;

	if( handle_ == INVALID_NETWORK_HANDLE )
		delete this;
}

///
/// \brief ������Ϣ��Chat������
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::Send2ChatSrv( xgc_uint16 msgid, ::google::protobuf::Message& msg )
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