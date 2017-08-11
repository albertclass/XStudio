#pragma once

#ifndef _CLIENT_SESSION_H_ 
#define _CLIENT_SESSION_H_ 

class CClientSession : public net::INetworkSession
{
protected:
	/// ������
	net::network_t handle_;
	/// �����ӳ�
	xgc_ulong pingpong_;
	/// ���һ�ν���ping��Ϣ��ʱ��
	xgc_time64 pinglast_;
	/// �û�����
	xgc_uint32 chat_id_;
public:
	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	CClientSession();

	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual ~CClientSession();

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
	/// \brief �û���֤
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onUserAuth( xgc_lpvoid ptr, int len );

	///
	/// \brief �����û���Ϣ
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onUserInfoReq( xgc_lpvoid ptr, int len );

	///
	/// \brief �������Ƶ��
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onEnterChannelReq( xgc_lpvoid ptr, int len );

	///
	/// \brief �����뿪Ƶ��
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onLeaveChannelReq( xgc_lpvoid ptr, int len );

	///
	/// \brief �û�˽������
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onUserChatReq( xgc_lpvoid ptr, int len );

	///
	/// \brief �û�Ƶ����������
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onChannelChatReq( xgc_lpvoid ptr, int len );

	///
	/// \brief �������ݵ��ͻ���
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Send2Client( xgc_uint16 msgid, ::google::protobuf::Message& msg );

};

#endif // _NETSESSION_H_ 

