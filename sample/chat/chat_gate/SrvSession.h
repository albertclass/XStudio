#pragma once

#ifndef _SERVER_SESSION_H_ 
#define _SERVER_SESSION_H_ 

class CServerSession : public net::INetworkSession
{
protected:
	/// ������
	net::network_t handle_;
	/// �����ӳ�
	xgc_ulong pingpong_;
	/// ���һ�ν���ping��Ϣ��ʱ��
	xgc_time64 pinglast_;
	/// ����Ƶ��
	xgc_map< xgc_uint32, xgc_string > channels_;
	/// �������ϵ��û�
	xgc_map< xgc_uint64, xgc_uint32 > users_;
public:
	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	CServerSession();

	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual ~CServerSession();

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
	/// \brief ����Ƶ����Ӧ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onCreateChannelAck( xgc_lpvoid ptr, int len );

	///
	/// \brief ��ɫ��½�����������Ӧ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onLoginAck( xgc_lpvoid ptr, int len );

	///
	/// \brief �û���½
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_long UserLogin( xgc_uint64 user_id, const xgc_string &nickname, xgc_uint32 session_id );

	///
	/// \brief �û��ǳ�
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void UserLogout( xgc_uint64 user_id );

	///
	/// \brief ����ϵͳ֪ͨ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void SendSystemNote( xgc_uint32 channel_id, xgc_string note, xgc_uint32 flags );

	///
	/// \brief ��������
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Send( xgc_uint16 msgid, ::google::protobuf::Message& msg );
};

#endif // _SERVER_SESSION_H_ 

