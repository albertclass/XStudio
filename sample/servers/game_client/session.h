#ifndef __SESSION_H__
#define __SESSION_H__

class CSession : public INetworkSession
{
public:
	CSession();

	~CSession();
private:
	///
	/// \brief 数据包是否
	/// \return	0 ~ 成功, -1 ~ 失败
	///
	virtual int OnParsePacket( const void* data, xgc_size size );

	///
	/// \brief 连接建立
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnAccept( net::network_t handle );

	///
	/// \brief 连接建立
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnConnect( net::network_t handle );

	///
	/// \brief 连接错误
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnError( xgc_int16 error_type, xgc_int16 error_code );

	///
	/// \brief 连接关闭
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:10
	///
	virtual xgc_void OnClose();

	///
	/// \brief 接收数据
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:10
	///
	virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size );

	///
	/// \brief 网络保活事件
	///
	/// \author albert.xu
	/// \date 2017/03/03 10:41
	///
	virtual xgc_void OnAlive();
};
#endif // __SESSION_H__