#pragma once

#ifndef _CHAT_SRV_SESSION_H_ 
#define _CHAT_SRV_SESSION_H_ 

class CChatSrvSession : public net::INetworkSession
{
protected:
	/// ������
	net::network_t handle_;
	/// �����ӳ�
	xgc_ulong pingpong_;
	/// ���һ�ν���ping��Ϣ��ʱ��
	xgc_time64 pinglast_;
	/// �û���ʶ
	xgc_uint64 user_id_;
	/// �����ʶ
	xgc_uint32 chat_id_;
	/// ��½����
	xgc_string token_;
	/// ��Ϸ���������Ӿ��
	network_t gate_handle_;
public:
	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	CChatSrvSession( network_t gate_handle, xgc_uint64 user_id, xgc_uint32 chat_id, const xgc_string &token );

	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual ~CChatSrvSession();

	///
	/// \brief ��ȡ������
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	net::network_t GetHandle()const
	{
		return handle_;
	}

	///
	/// \brief ���ݰ��Ƿ�
	/// \return	0 ~ �ɹ�, -1 ~ ʧ��
	///
	virtual int OnParsePacket( const void* data, xgc_size size ) override;

	///
	/// \brief ���ӽ���
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual xgc_void OnAccept( net::network_t handle ) override;

	///
	/// \brief ���ӽ���
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual xgc_void OnConnect( net::network_t handle ) override;

	///
	/// \brief ���Ӵ���
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual xgc_void OnError( xgc_uint32 error_code ) override;

	///
	/// \brief ���ӹر�
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual xgc_void OnClose() override;

	///
	/// \brief ���籣���¼�
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual xgc_void OnAlive() override;

	///
	/// \brief ��������
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size ) override;

	///
	/// \brief �����������֤
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void ChatUserAuth();

	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void ChatTo( xgc_uint32 nChatID, xgc_lpcstr lpMessage );

	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Say( xgc_uint32 nChannelID, xgc_lpcstr lpMessage );

	///
	/// \brief �û���֤��Ӧ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onUserAuthAck( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief �ѽ���Ƶ����Ӧ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onChannelEnterNtf( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief �û���Ϣ��Ӧ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onUserInfoAck( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief ����Ƶ����Ӧ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onChannelEnterAck( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief �뿪Ƶ����Ӧ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onchannelLeaveAck( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief �û���������֪ͨ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onUserChatNtf( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief Ƶ������֪ͨ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onChannelChatNtf( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief �������֪ͨ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onChatErr( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief ������������ӶϿ�
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void ChatSrvClosed();

	///
	/// \brief ��Ϸ���������ӶϿ�
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void GateSrvClosed();

	///
	/// \brief ������Ϣ��Chat������
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Send2ChatSrv( xgc_uint16 msgid, ::google::protobuf::Message& msg );
};

#endif // _CHAT_SRV_SESSION_H_ 

