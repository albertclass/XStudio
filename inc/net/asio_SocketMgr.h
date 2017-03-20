///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file HandlerManager.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// �׽��ֹ������
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
		/// \brief �׽��ֹ�����
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
			/// \brief ����
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:22
			///
			asio_SocketMgr();

			///
			/// \brief ����
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:22
			///
			~asio_SocketMgr();

			///
			/// \brief ��������
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:22
			///
			asio_SocketMgr( const asio_SocketMgr& ) = delete;

			///
			/// \brief ������ֵ
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:22
			///
			asio_SocketMgr& operator =( const asio_SocketMgr& ) = delete;

		public:
			///
			/// \brief ��ʼ���׽��ֹ�����
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:22
			///
			xgc_void Initialize();

			///
			/// \brief �ȴ��������ӹر�
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:20
			///
			xgc_void Final( xgc_ulong timeout );

			///
			/// \brief ͨ�������ȡ�׽���
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:16
			///
			asio_SocketPtr getSocket( network_t handle );

			///
			/// \brief �׽���ͳ��
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:16
			///
			size_t SocketCount()const
			{
				return XGC_COUNTOF( handles ) - free_handles.size();
			}

			///
			/// \brief ����һ����ʱ������δ�������ƣ�
			///
			/// \author albert.xu
			/// \date 2017/03/20 15:34
			///
			xgc_bool SetTimer( network_t hHandle, xgc_uint32 nTimerId, xgc_real64 fPeriod, xgc_real64 fAfter );

			///
			/// \brief ɾ��һ����ʱ��
			///
			/// \author albert.xu
			/// \date 2017/03/14 17:24
			///
			xgc_bool DelTimer( xgc_uint32 nTimerId );

			///
			/// \brief �ر������׽���
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:16
			///
			xgc_void CloseAll( xgc_lpvoid from = 0 );

			///
			/// \brief �ȴ������׽��ֹر�
			///
			/// \author albert.xu
			/// \date 2016/02/26 14:17
			///
			xgc_bool WaitForClose( network_t handle, xgc_uint32 sleep = 100, xgc_ulong timeout = -1 );

			///
			/// \brief �½���
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:04
			///
			group_t NewGroup( network_t self_handle );

			///
			/// \brief ������
			///
			/// \param handle GroupParam{ xgc_uint32 handle; xgc_uint32 group; } *(xgc_uint32*)param, *(((xgc_uint32*)param)+1)
			/// \author albert.xu
			/// \reutrn >0 �ɹ� -1 ָ�������Ѳ����� -2 �������ѱ���� -3 ��Ӿ������ʧ��
			/// \date 2016/02/24 18:04
			///
			xgc_long EnterGroup( group_t group, network_t handle );

			///
			/// \brief �뿪��
			///
			/// \param handle ������
			/// \return >0 �ɹ����ص�ǰ���еľ������, -1 ָ�������Ѳ����� -2 ������������
			/// \author albert.xu
			/// \date 2016/02/24 18:05
			///
			xgc_long LeaveGroup( group_t group, network_t handle );

			///
			/// \brief ɾ����
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:06
			///
			xgc_long RemoveGroup( group_t group );

			///
			/// \brief ��ȡ�����
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:06
			///
			CSocketGroup* FetchHandleGroup( group_t group );

			///
			/// \brief �黹�����
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:07
			///
			xgc_void FreeHandleGroup( CSocketGroup* pGroup )const;

			///
			/// \brief �����¼�
			///
			/// \author albert.xu
			/// \date 2017/03/01 14:46
			///
			xgc_void Push( INetPacket *pEvt );

			///
			/// \brief �����¼�
			///
			/// \author albert.xu
			/// \date 2017/03/01 14:47
			///
			xgc_long Exec( xgc_long nStep );
		protected:
			///
			/// \brief ���ӽ���
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:07
			///
			xgc_bool LinkUp( asio_SocketPtr pSocket );

			///
			/// \brief ���ӶϿ�
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:07
			///
			xgc_void LinkDown( asio_SocketPtr pSocket );

			///
			/// \brief ��ʱ����Ӧ
			///
			/// \author albert.xu
			/// \date 2017/03/14 17:32
			///
			xgc_void OnTimer( const asio::error_code &e, network_t handle, xgc_uint32 id, xgc_real64 period );
		private:
			typedef std::queue< network_t > CFreeHandleList;
			typedef std::unordered_map< group_t, CSocketGroup* > CGroupMap;

			/// δ����ľ��
			CFreeHandleList	free_handles;
			/// δ�������
			CFreeHandleList free_groups;

			/// �����
			asio_SocketPtr	handles[0xffff];

			/// ��ӳ���
			CGroupMap		group_map;
			/// ���߳���
			std::mutex		lock_;
			/// ���߳�����
			std::mutex		lock_group_;

			/// ��Ϣ����
			CNetEventQueue	mEvtQueue;

			/// ���̶߳�ʱ����
			std::mutex		lock_timer_;
			/// ��ʱ��ӳ���
			xgc_unordered_map< xgc_uint32, asio::steady_timer* > mTimerMap;
		};
	}
}
#endif // _ASIO_SOCKETMGR_H_