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
#include "NetEventQueue.h"

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
		friend class asio_Socket;
		friend asio_SocketMgr& getSocketMgr();
		public:
			struct CSocketGroup : public std::unordered_set< group_t >
			{
				std::mutex		group_guard_;
				network_t		network_;
				CSocketGroup( network_t handle )
					: network_( handle )
				{
				}

				~CSocketGroup()
				{
				}

				xgc_void lock_handle()
				{
					group_guard_.lock();
				}

				xgc_void free_handle()
				{
					group_guard_.unlock();
				}
			};

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
			xgc_bool SetTimer( network_t hHandle, xgc_uint32 nTimerId, xgc_real64 fPeriod, xgc_real64 fAfter );

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
			/// \brief 新建组
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:04
			///
			group_t NewGroup( network_t self_handle );

			///
			/// \brief 进入组
			///
			/// \param handle GroupParam{ xgc_uint32 handle; xgc_uint32 group; } *(xgc_uint32*)param, *(((xgc_uint32*)param)+1)
			/// \author albert.xu
			/// \reutrn >0 成功 -1 指定的组已不存在 -2 网络句柄已被添加 -3 添加句柄到组失败
			/// \date 2016/02/24 18:04
			///
			xgc_long EnterGroup( group_t group, network_t handle );

			///
			/// \brief 离开组
			///
			/// \param handle 网络句柄
			/// \return >0 成功返回当前组中的句柄个数, -1 指定的组已不存在 -2 网络句柄不存在
			/// \author albert.xu
			/// \date 2016/02/24 18:05
			///
			xgc_long LeaveGroup( group_t group, network_t handle );

			///
			/// \brief 删除组
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:06
			///
			xgc_long RemoveGroup( group_t group );

			///
			/// \brief 获取组对象
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:06
			///
			CSocketGroup* FetchHandleGroup( group_t group );

			///
			/// \brief 归还组对象
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:07
			///
			xgc_void FreeHandleGroup( CSocketGroup* pGroup )const;

			///
			/// \brief 推入事件
			///
			/// \author albert.xu
			/// \date 2017/03/01 14:46
			///
			xgc_void Push( INetPacket *pEvt );

			///
			/// \brief 弹出事件
			///
			/// \author albert.xu
			/// \date 2017/03/01 14:47
			///
			xgc_long Exec( xgc_long nStep );
		protected:
			///
			/// \brief 连接建立
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:07
			///
			xgc_bool LinkUp( asio_SocketPtr pSocket );

			///
			/// \brief 连接断开
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:07
			///
			xgc_void LinkDown( asio_SocketPtr pSocket );

			///
			/// \brief 定时器响应
			///
			/// \author albert.xu
			/// \date 2017/03/14 17:32
			///
			xgc_void OnTimer( const asio::error_code &e, network_t handle, xgc_uint32 id, xgc_real64 period );
		private:
			typedef std::queue< network_t > CFreeHandleList;
			typedef std::unordered_map< group_t, CSocketGroup* > CGroupMap;

			/// 未分配的句柄
			CFreeHandleList	free_handles;
			/// 未分配的组
			CFreeHandleList free_groups;

			/// 句柄池
			asio_SocketPtr	handles[0xffff];

			/// 组映射表
			CGroupMap		group_map;
			/// 多线程锁
			std::mutex		lock_;
			/// 多线程组锁
			std::mutex		lock_group_;

			/// 消息队列
			CNetEventQueue	mEvtQueue;

			/// 多线程定时器锁
			std::mutex		lock_timer_;
			/// 定时器映射表
			xgc_unordered_map< xgc_uint32, asio::steady_timer* > mTimerMap;
		};
	}
}
#endif // _ASIO_SOCKETMGR_H_