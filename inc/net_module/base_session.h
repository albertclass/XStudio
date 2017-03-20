#pragma once
#ifndef _BASE_SESSION_H_
#define _BASE_SESSION_H_

namespace net_module
{
	#pragma pack(1)
	struct MessageHeader
	{
		xgc_uint16 length;
		xgc_uint8  type;
		xgc_uint8  code;
	};
	#pragma pack()

	class CBaseSession : public net::INetworkSession
	{
	protected:
		/// ������
		net::network_t handle_;

	protected:
		///
		/// \brief ����
		///
		/// \author albert.xu
		/// \date 2017/03/14 10:51
		///
		CBaseSession();

	public:
		///
		/// \brief ����
		///
		/// \author albert.xu
		/// \date 2017/03/14 10:51
		///
		virtual ~CBaseSession();

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
		/// \brief �¼�֪ͨ
		///
		/// \author albert.xu
		/// \date 2017/03/03 11:10
		///
		virtual xgc_ulong EvtNotify( xgc_uint32 event, xgc_uint32 result ) = 0;

		///
		/// \brief ��Ϣ֪ͨ
		///
		/// \author albert.xu
		/// \date 2017/03/03 11:12
		///
		virtual xgc_ulong MsgNotify( xgc_lpvoid data, xgc_size size ) = 0;

		///
		/// \brief ��������
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:11
		///
		virtual xgc_void Send( xgc_lpvoid data, xgc_size size ) = 0;
	};
}

#endif // _BASE_SESSION_H_