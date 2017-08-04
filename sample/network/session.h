#pragma once

#ifndef _NETSESSION_H_ 
#define _NETSESSION_H_ 

class CNetSession : public net::INetworkSession
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
	/// \date 2017/03/14 10:51
	///
	CNetSession();

	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2017/03/14 10:51
	///
	virtual ~CNetSession();

	///
	/// \brief ��ȡ������
	///
	/// \author albert.xu
	/// \date 2017/03/14 10:53
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
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnAccept( net::network_t handle ) override;

	///
	/// \brief ���ӽ���
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnConnect( net::network_t handle ) override;

	///
	/// \brief ���Ӵ���
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnError( xgc_uint32 error_code ) override;

	///
	/// \brief ���ӹر�
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:10
	///
	virtual xgc_void OnClose() override;

	///
	/// \brief ���籣���¼�
	///
	/// \author albert.xu
	/// \date 2017/03/03 10:41
	///
	virtual xgc_void OnAlive() override;

	///
	/// \brief ��������
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:10
	///
	virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size ) override;

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
	/// \date 2017/03/27 11:36
	///
	xgc_lpvoid GetUserdata()
	{
		return userdata_;
	}

	///
	/// \brief ������Ϣ
	///
	/// \author albert.xu
	/// \date 2017/03/24 17:28
	///
	xgc_void SendPacket( xgc_lpvoid data, xgc_size size );
};

#endif // _NETSESSION_H_ 

