#ifndef __SESSION_H__
#define __SESSION_H__

class CSession : public INetworkSession
{
public:
	CSession();

	~CSession();
private:
	///
	/// \brief ���ݰ��Ƿ�
	/// \return	0 ~ �ɹ�, -1 ~ ʧ��
	///
	virtual int OnParsePacket( const void* data, xgc_size size );

	///
	/// \brief ���ӽ���
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnAccept( net::network_t handle );

	///
	/// \brief ���ӽ���
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnConnect( net::network_t handle );

	///
	/// \brief ���Ӵ���
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnError( xgc_int16 error_type, xgc_int16 error_code );

	///
	/// \brief ���ӹر�
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:10
	///
	virtual xgc_void OnClose();

	///
	/// \brief ��������
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:10
	///
	virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size );

	///
	/// \brief ���籣���¼�
	///
	/// \author albert.xu
	/// \date 2017/03/03 10:41
	///
	virtual xgc_void OnAlive();
};
#endif // __SESSION_H__