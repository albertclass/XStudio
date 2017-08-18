#pragma once

#ifndef _GATE_SRV_SESSION_H_ 
#define _GATE_SRV_SESSION_H_ 

class CChatSrvSession;
class CGameSrvSession : public net::INetworkSession
{
protected:
	/// ������
	net::network_t handle_;
	/// �����ӳ�
	xgc_ulong pingpong_;
	/// ���һ�ν���ping��Ϣ��ʱ��
	xgc_time64 pinglast_;
	/// ������������ӻỰ
	CChatSrvSession* mChatSrvSession;
	/// �û���
	xgc_string mUsername;
	/// ����
	xgc_string mPassword;
public:
	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	CGameSrvSession( xgc_lpcstr username, xgc_lpcstr password );

	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual ~CGameSrvSession();

	///
	/// \brief ��ȡ������
	///
	/// \author albert.xu
	/// \date 2017/08/05
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
	/// \date 2017/08/05
	///
	virtual xgc_void OnAccept( net::network_t handle ) override;

	///
	/// \brief ���ӽ���
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnConnect( net::network_t handle ) override;

	///
	/// \brief ���Ӵ���
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnError( xgc_int16 error_type, xgc_int16 error_code ) override;

	///
	/// \brief ���ӹر�
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnClose() override;

	///
	/// \brief ���籣���¼�
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnAlive() override;

	///
	/// \brief ��������
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size ) override;

	///
	/// \brief �����������������
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_bool OnChatMsg( xgc_uint16 msgid, xgc_lpcstr ptr, xgc_long len )
	{
		return false;
	}

	///
	/// \brief ������������ӽ���
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnChatConnect( network_t handle );

	///
	/// \brief ������������ӽ���
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnChatError( xgc_int16 error_type, xgc_int16 error_code )
	{
		if( error_type == NET_ETYPE_CONNECT )
			CloseLink( handle_ );
	}

	///
	/// \brief ������������ӶϿ�
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnChatClose();

	///
	/// \brief �������ݵ���Ϸ������
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Send2GameSrv( xgc_uint16 msgid, ::google::protobuf::Message& msg );

	///
	/// \brief �������ݵ���Ϸ������
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Send2ChatSrv( xgc_uint16 msgid, ::google::protobuf::Message& msg );

	///
	/// \brief �Ͽ���Ϸ����������
	///
	/// \author albert.xu
	/// \date 2017/08/09
	///
	xgc_void Disconnect();
};

#endif // _GATE_SRV_SESSION_H_ 

