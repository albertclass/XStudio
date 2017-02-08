///////////////////////////////////////////////////////////////
/// \file ClientSession.h
/// \brief 文件简介
/// \author xufeng04
/// \date 三月 2016
///
/// 客户端会话对象
///
///////////////////////////////////////////////////////////////
#pragma once
#ifndef _CLIENTSESSION_H_
#define _CLIENTSESSION_H_

namespace xgc
{
	namespace net
	{
		class CSession : public INetworkSession, public ProtocalDefault
		{
		protected:
			/// 网络ping值
			xgc_ulong		ping_;
			/// 网络ping值
			xgc_ulong		pingfailed_;
			/// Socket句柄
			network_t		handle_;
			/// 用户数据
			xgc_lpvoid		userdata_;
			/// 消息队列
			MessageQueuePtr queue_;
		public:
			CSession( MessageQueuePtr Queue );

			~CSession();
		private:
			///
			/// \brief 数据包是否
			/// \return	0 ~ 成功, -1 ~ 失败
			///
			virtual int OnParsePacket( const void* data, xgc_size size ) override;

			///
			/// \brief 连接关闭时的处理函数。重载此函数时需在做完应用需要做的事情后将连接删除或重置
			/// \return	0 ~ 成功, -1 ~ 失败
			///
			virtual int OnClose() override;

			///
			/// \brief 成功接受到网络数据包后提交到iocp上层应用处理的函数
			/// \param	data 接受到的数据,在中len范围内可靠有效
			/// \param	len	接受到数据的长度,此数据由iocp保证其真实性。
			/// \return	0 ~ 成功, -1 ~ 失败
			///
			virtual int OnRecv( const void *data, xgc_size size ) override;

			///
			/// \brief 成功发送网络数据包后给iocp上层处理的函数，目前暂时未使用
			/// \param data 成功发送的数据
			/// \param len 成功发送的数据长度
			/// \return	0 ~ 成功, -1 ~ 失败
			///
			virtual int OnSend( const void *data, xgc_size size ) override;

			///
			/// \brief iocp捕获到错误并提供给应用的处理机会
			/// \param error 错误代码
			/// \return 0 ~ 成功, -1 ~ 失败
			virtual int OnError( int error ) override;

			///
			/// \brief 保活定时器，用于定时发送保活消息
			/// \return 0 ~ 成功, -1 ~ 失败
			virtual int OnAlive() override;

			///
			/// \brief 获取用户数据
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:23
			///
			virtual int GetPing()const override
			{
				return ping_;
			}

			///
			/// \brief 获取用户数据
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:23
			///
			virtual xgc_lpvoid GetUserdata()const override
			{
				return userdata_;
			}

			///
			/// \brief 设置用户数据
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:23
			///
			virtual xgc_void SetUserdata( xgc_lpvoid userdata ) override
			{
				userdata_ = userdata;
			}
		};

		class CClientSession : public CSession
		{
		public:
			CClientSession( MessageQueuePtr QueuePtr );
			~CClientSession();

		private:
			/// \brief  新的远端连接成功时的处理函数
			/// \param	new_handle	新连接的套接字
			/// \return	0	成功	-1 失败
			virtual int OnAccept( network_t new_handle, xgc_lpvoid from );
		};

		class CServerSession : public CSession
		{
		public:
			CServerSession( MessageQueuePtr QueuePtr );
			~CServerSession();

		private:
			/// \brief  新的远端连接成功时的处理函数
			/// \param	new_handle	新连接的套接字
			/// \return	0	成功	-1 失败
			virtual int OnAccept( network_t new_handle, xgc_lpvoid from );
		};

	}
}
#endif // _CLIENTSESSION_H_