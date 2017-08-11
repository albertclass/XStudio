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
	/// �û�����
	xgc_lpvoid userdata_;
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
	virtual xgc_void OnError( xgc_uint32 error_code ) override;

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
	/// \brief ��ɫ��½��Ϸ��֪ͨ���������
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onUserLoginReq( xgc_lpvoid ptr, int len );

	///
	/// \brief ��ɫ�ǳ���Ϸ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onUserLogoutReq( xgc_lpvoid ptr, int len );

	///
	/// \brief ��Ϸ����������Ƶ��
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onCreateChannelReq( xgc_lpvoid ptr, int len );

	///
	/// \brief ����������ϵͳ��Ϣ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onSystemNoteReq( xgc_lpvoid ptr, int len );

	///
	/// \brief �����û�����
	///
	/// \author albert.xu
	/// \date 2017/03/27 11:35
	///
	xgc_void SetUserdata( xgc_lpvoid userdata )
	{
		userdata_ = userdata;
	}

	///
	/// \brief ��ȡ�û�����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_lpvoid GetUserdata()
	{
		return userdata_;
	}

	///
	/// \brief �������ݵ���Ϸ������
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Send2GameServer( xgc_uint16 msgid, ::google::protobuf::Message& msg );

};

#endif // _SERVER_SESSION_H_ 

