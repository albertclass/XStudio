#pragma once
#include "NetBase.h"

namespace XGC
{
	namespace net
	{

		class CClientSession	:	public INetworkSession
		{
		public:
			CClientSession( IMessageQueue *pMessageQueue, const PacketProtocal* pProtocal );
			~CClientSession(xgc_void);

		private:
			///function	:	连接关闭时的处理函数。重载此函数时需在做完应用需要做的事情后将连接删除或重置
			///param	:	无
			///return	:	0	成功	-1 失败
			virtual int OnClose(xgc_void);

			///function	:	新的远端连接成功时的处理函数
			///param	:	new_handle	新连接的套接字
			///return	:	0	成功	-1 失败
			virtual int OnAccept( network_t new_handle, xgc_uintptr from );

			///function	:	成功接受到网络数据包后提交到iocp上层应用处理的函数
			///param	:	data	接受到的数据	在中len范围内可靠有效
			///param	:	len		接受到数据的长度	此数据由iocp保证其真实性。
			///return	:	0	成功	-1 失败
			virtual int OnRecv( const void *data, size_t size );

			///function	:	成功发送网络数据包后给iocp上层处理的函数，目前暂时未使用
			///param	:	data	成功发送的数据
			///param	:	len		成功发送的数据长度
			///return	:	0	成功	-1 失败
			virtual int OnSend( const void *data, size_t size );

			///function	:	iocp捕获到错误并提供给应用的处理机会
			///param	:	error	错误代码
			///return	:	0	成功	-1 失败
			virtual int OnError( int error );

		private:
			IMessageQueue	*m_pMessageQueue;
			const
			PacketProtocal	*m_pProtocal;
		};

		INetworkSession* create_client_handler( xgc_uintptr lParam );
	}
}