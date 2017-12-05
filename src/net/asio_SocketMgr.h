///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file HandlerManager.h
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 套接字管理对象
///
///////////////////////////////////////////////////////////////
#pragma once
#ifndef _ASIO_SOCKETMGR_H_
#define _ASIO_SOCKETMGR_H_

namespace xgc
{
	namespace net
	{
		class asio_Socket;
		using asio_SocketPtr = std::shared_ptr< asio_Socket >;
		///
		/// \brief 套接字管理类
		///
		/// \author albert.xu
		/// \date 2016/02/26 14:22
		///
		class asio_SocketMgr
		{
		friend asio_SocketMgr& getSocketMgr();
		private:
			///
			/// \brief 构造
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:22
			///
			asio_SocketMgr();

			///
			/// \brief 析构
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:22
			///
			~asio_SocketMgr();

			///
			/// \brief 不允许复制
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:22
			///
			asio_SocketMgr( const asio_SocketMgr& ) = delete;

			///
			/// \brief 不允许赋值
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:22
			///
			asio_SocketMgr& operator =( const asio_SocketMgr& ) = delete;

		public:
			///
			/// \brief 初始化套接字管理器
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:22
			///
			xgc_void Initialize();

			///
			/// \brief 等待所有连接关闭
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:20
			///
			xgc_void Final( xgc_ulong timeout );

			///
			/// \brief 通过句柄获取套接字
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:16
			///
			asio_SocketPtr getSocket( network_t handle );

			///
			/// \brief 套接字统计
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:16
			///
			size_t SocketCount()const
			{
				return XGC_COUNTOF( handles ) - free_handles.size();
			}

			///
			/// \brief 设置一个定时器（并未进行完善）
			///
			/// \author albert.xu
			/// \date 2017/03/20 15:34
			///
			xgc_bool NewTimer( xgc_uint32 nTimerId, xgc_real64 fPeriod, xgc_real64 fAfter, const std::function< void() > &onTimer );

			///
			/// \brief 删除一个定时器
			///
			/// \author albert.xu
			/// \date 2017/03/14 17:24
			///
			xgc_bool DelTimer( xgc_uint32 nTimerId );

			///
			/// \brief 关闭所有套接字
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:16
			///
			xgc_void CloseAll( xgc_lpvoid from = 0 );

			///
			/// \brief 等待所有套接字关闭
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:17
			///
			xgc_bool WaitForClose( network_t handle, xgc_uint32 sleep = 100, xgc_ulong timeout = -1 );

			///
			/// \brief 推入事件
			///
			/// \author albert.xu
			/// \date 2017/03/01 14:46
			///
			xgc_void Push( xgc_lpvoid data, xgc_size size );

			///
			/// \brief 弹出事件
			///
			/// \author albert.xu
			/// \date 2017/08/17
			///
			xgc_bool Kick( xgc_lpvoid &data, xgc_size &size );

			///
			/// \brief 执行事件
			///
			/// \author albert.xu
			/// \date 2017/03/01 14:47
			///
			xgc_long Exec( xgc_long nStep );

			///
			/// \brief 连接建立
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:07
			///
			xgc_bool LinkUp( asio_SocketPtr &pSocket );

			///
			/// \brief 连接断开
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:07
			///
			xgc_void LinkDown( asio_SocketPtr &pSocket );

		private:
			///
			/// \brief 定时器响应
			///
			/// \author albert.xu
			/// \date 2017/03/14 17:32
			///
			xgc_void OnTimer( const asio::error_code &e, xgc_uint32 id, xgc_real64 period );
		private:
			typedef std::queue< network_t > CFreeHandleList;

			/// 未分配的句柄
			CFreeHandleList	free_handles;
			/// 未分配的组
			CFreeHandleList free_groups;

			/// 句柄池
			asio_SocketPtr	handles[0xffff];

			/// 多线程锁
			std::mutex		lock_;

			/// 消息队列
			std::queue< std::tuple< xgc_lpvoid, xgc_size > > event_queue_;

			/// 消息队列锁
			std::mutex		lock_queue_;

			/// 多线程定时器锁
			std::mutex		lock_timer_;

			struct timer_info
			{
				asio::steady_timer* timer;
				std::function< void() > on_timer;
			};

			/// 时间戳
			xgc_time64 exec_;
			/// 新增事件量
			std::atomic_ullong exec_inc_;
			/// 处理事件量
			std::atomic_ullong exec_dec_;

			/// 定时器映射表
			xgc::unordered_map< xgc_uint32, timer_info > mTimerMap;
		};

		///
		/// \brief 管理器是否有效
		///
		/// \author albert.xu
		/// \date 2017/07/18 18:07
		///
		xgc_bool IsValidSocketMgr();
	}
}
#endif // _ASIO_SOCKETMGR_H_