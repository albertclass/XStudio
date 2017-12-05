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
		friend asio_SocketMgr& getSocketMgr();
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
			xgc_bool NewTimer( xgc_uint32 nTimerId, xgc_real64 fPeriod, xgc_real64 fAfter, const std::function< void() > &onTimer );

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
			/// \brief �����¼�
			///
			/// \author albert.xu
			/// \date 2017/03/01 14:46
			///
			xgc_void Push( xgc_lpvoid data, xgc_size size );

			///
			/// \brief �����¼�
			///
			/// \author albert.xu
			/// \date 2017/08/17
			///
			xgc_bool Kick( xgc_lpvoid &data, xgc_size &size );

			///
			/// \brief ִ���¼�
			///
			/// \author albert.xu
			/// \date 2017/03/01 14:47
			///
			xgc_long Exec( xgc_long nStep );

			///
			/// \brief ���ӽ���
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:07
			///
			xgc_bool LinkUp( asio_SocketPtr &pSocket );

			///
			/// \brief ���ӶϿ�
			///
			/// \author albert.xu
			/// \date 2016/02/24 18:07
			///
			xgc_void LinkDown( asio_SocketPtr &pSocket );

		private:
			///
			/// \brief ��ʱ����Ӧ
			///
			/// \author albert.xu
			/// \date 2017/03/14 17:32
			///
			xgc_void OnTimer( const asio::error_code &e, xgc_uint32 id, xgc_real64 period );
		private:
			typedef std::queue< network_t > CFreeHandleList;

			/// δ����ľ��
			CFreeHandleList	free_handles;
			/// δ�������
			CFreeHandleList free_groups;

			/// �����
			asio_SocketPtr	handles[0xffff];

			/// ���߳���
			std::mutex		lock_;

			/// ��Ϣ����
			std::queue< std::tuple< xgc_lpvoid, xgc_size > > event_queue_;

			/// ��Ϣ������
			std::mutex		lock_queue_;

			/// ���̶߳�ʱ����
			std::mutex		lock_timer_;

			struct timer_info
			{
				asio::steady_timer* timer;
				std::function< void() > on_timer;
			};

			/// ʱ���
			xgc_time64 exec_;
			/// �����¼���
			std::atomic_ullong exec_inc_;
			/// �����¼���
			std::atomic_ullong exec_dec_;

			/// ��ʱ��ӳ���
			xgc::unordered_map< xgc_uint32, timer_info > mTimerMap;
		};

		///
		/// \brief �������Ƿ���Ч
		///
		/// \author albert.xu
		/// \date 2017/07/18 18:07
		///
		xgc_bool IsValidSocketMgr();
	}
}
#endif // _ASIO_SOCKETMGR_H_