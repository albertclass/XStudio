#pragma once

#ifndef _GATE_SRV_SESSION_H_ 
#define _GATE_SRV_SESSION_H_ 

class CGameSrvSession : public net::INetworkSession
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
	CGameSrvSession();

	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual ~CGameSrvSession();

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
	/// \brief �����û�����
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	xgc_void setUserdata( xgc_lpvoid userdata )
	{
		userdata_= userdata;
	}

	///
	/// \brief ��ȡ�û�����
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	xgc_lpvoid getUserdata()
	{
		return userdata_;
	}

	///
	/// \brief �������ݵ���Ϸ������
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	xgc_void Send2GateSrv( xgc_uint16 msgid, ::google::protobuf::Message& msg );
};

#endif // _GATE_SRV_SESSION_H_ 

