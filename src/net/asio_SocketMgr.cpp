#include "Header.h"
#include "asio_SocketMgr.h"

#include <thread>
#include <chrono>

namespace xgc
{
	namespace net
	{
		asio_SocketMgr::asio_SocketMgr()
		{
		}

		asio_SocketMgr::~asio_SocketMgr()
		{
		}

		xgc_void asio_SocketMgr::Initialize()
		{
			for( xgc_uint16 i = 0; i < XGC_COUNTOF( handles ); ++i )
			{
				free_handles.push( i );
			}

			for( xgc_uint16 i = 0; i < XGC_COUNTOF( handles ); ++i )
			{
				free_groups.push( i );
			}
		}

		///
		/// \brief �ȴ��������ӹر�
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:20
		///

		xgc_void asio_SocketMgr::Final( xgc_ulong timeout )
		{
			auto start = tick();
			auto counter = 0;

			do
			{
				std::this_thread::sleep_for( std::chrono::milliseconds(1000) );

				counter = 0;
				for( network_t i = 0; i < XGC_COUNTOF( handles ); ++i )
				{
					asio_SocketPtr pSocket = getSocket( i );
					if( pSocket )
					{
						pSocket->close();

						counter++;
					}
				}

			} while( counter > 0 && (tick() - start < timeout) );

			// execute socket close event.
			Exec( -1 );
		}

		asio_SocketPtr asio_SocketMgr::getSocket( network_t handle )
		{
			XGC_ASSERT_RETURN( handle < XGC_COUNTOF( handles ), xgc_nullptr );

			return handles[handle];
		}

		xgc_bool asio_SocketMgr::SetTimer( network_t hHandle, xgc_uint32 nTimerId, xgc_real64 fPeriod, xgc_real64 fAfter )
		{
			std::lock_guard< std::mutex > _lock( lock_timer_ );

			auto it = mTimerMap.find( nTimerId );
			if( it == mTimerMap.end() )
			{
				auto timer = XGC_NEW asio::steady_timer( getNetwork().Ref() );
				timer->expires_after( std::chrono::milliseconds( (xgc_uint32)(fAfter * 1000) ) );
				timer->async_wait( std::bind( &asio_SocketMgr::OnTimer, this, std::placeholders::_1, hHandle, nTimerId, fPeriod ) );

				mTimerMap[nTimerId] = timer;
				return true;
			}

			return false;
		}

		xgc_bool asio_SocketMgr::DelTimer( xgc_uint32 nTimerId )
		{
			std::lock_guard< std::mutex > _lock( lock_timer_ );

			auto it = mTimerMap.find( nTimerId );
			if( it != mTimerMap.end() )
			{
				auto timer = it->second;
				asio::error_code ec;
				if( timer->cancel( ec ) )
				{
					mTimerMap.erase( it );
					SAFE_DELETE( timer );
				}
			}

			return true;
		}

		xgc_void asio_SocketMgr::CloseAll( xgc_lpvoid from /*= 0 */ )
		{
			for( network_t i = 0; i < XGC_COUNTOF( handles ); ++i )
			{
				asio_SocketPtr pSocket = getSocket( i );
				if( pSocket && (from == 0 || pSocket->belong( from )) )
				{
					pSocket->close();
				}
			}
		}

		///
		/// \brief �ȴ������׽��ֹر�
		///
		/// \author albert.xu
		/// \date 2016/02/26 14:17
		///

		xgc_bool asio_SocketMgr::WaitForClose( network_t handle, xgc_uint32 sleep, xgc_ulong timeout )
		{
			if( handle >= XGC_COUNTOF( handles ) )
				return true;

			xgc_time64 start = tick();
			while( start < start + timeout )
			{
				asio_SocketPtr pSocket = getSocket( handle );
				if( xgc_nullptr == pSocket )
					return true;

				pSocket->close();
				std::this_thread::sleep_for( std::chrono::milliseconds(sleep) );
			}

			return false;
		}

		///
		/// \brief �½���
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:04
		///

		group_t asio_SocketMgr::NewGroup( network_t self_handle )
		{
			std::lock_guard< std::mutex > _l( lock_group_ );
			if( free_groups.empty() )
			{
				return INVALID_NETWORK_HANDLE;
			}
			else
			{
				group_t group = free_groups.front();
				auto ret = group_map.insert( CGroupMap::value_type( group, XGC_NEW CSocketGroup( self_handle ) ) );
				if( ret.second == false )
				{
					return INVALID_NETWORK_HANDLE;
				}
				free_groups.pop();
				return group;
			}

			return INVALID_NETWORK_HANDLE;
		}

		///
		/// \brief ������
		///
		/// \param handle GroupParam{ xgc_uint32 handle; xgc_uint32 group; } *(xgc_uint32*)param, *(((xgc_uint32*)param)+1)
		/// \author albert.xu
		/// \reutrn >0 �ɹ� -1 ָ�������Ѳ����� -2 �������ѱ���� -3 ��Ӿ������ʧ��
		/// \date 2016/02/24 18:04
		///

		xgc_long asio_SocketMgr::EnterGroup( group_t group, network_t handle )
		{
			CSocketGroup* pGroup = FetchHandleGroup( group );
			if( pGroup )
			{
				if( pGroup->find( handle ) == pGroup->end() )
				{
					auto ret = pGroup->insert( handle );
					auto size = (xgc_long) pGroup->size();
					FreeHandleGroup( pGroup );
					return ret.second ? size : -3;
				}
				else
				{
					FreeHandleGroup( pGroup );
					return -2;
				}
			}

			return -1;
		}

		///
		/// \brief �뿪��
		///
		/// \param handle ������
		/// \return >0 �ɹ����ص�ǰ���еľ������, -1 ָ�������Ѳ����� -2 ������������
		/// \author albert.xu
		/// \date 2016/02/24 18:05
		///

		xgc_long asio_SocketMgr::LeaveGroup( group_t group, network_t handle )
		{
			CSocketGroup* pGroup = FetchHandleGroup( group );
			if( pGroup )
			{
				if( pGroup->erase( handle ) == 0 )
				{
					FreeHandleGroup( pGroup );
					return -2;
				}

				auto size = (xgc_long) pGroup->size();
				FreeHandleGroup( pGroup );
				return size;
			}

			return -1;
		}

		///
		/// \brief ɾ����
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:06
		///

		xgc_long asio_SocketMgr::RemoveGroup( group_t group )
		{
			std::lock_guard< std::mutex > _l( lock_group_ );
			CGroupMap::iterator i = group_map.find( group );
			if( i != group_map.end() )
			{
				CSocketGroup* pGroup = i->second;
				group_map.erase( i );
				delete pGroup;
				return 0;
			}

			return -1;
		}

		///
		/// \brief ��ȡ�����
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:06
		///

		asio_SocketMgr::CSocketGroup * asio_SocketMgr::FetchHandleGroup( group_t group )
		{
			std::lock_guard< std::mutex > _l( lock_group_ );
			CGroupMap::iterator i = group_map.find( group );
			if( i != group_map.end() )
			{
				CSocketGroup* pGroup = i->second;
				if( pGroup )
				{
					pGroup->lock_handle();
					return pGroup;
				}
			}
			return xgc_nullptr;
		}

		///
		/// \brief �黹�����
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:07
		///

		xgc_void asio_SocketMgr::FreeHandleGroup( CSocketGroup * pGroup ) const
		{
			pGroup->free_handle();
		}

		xgc_void asio_SocketMgr::Push( INetPacket * pEvt )
		{
			mEvtQueue.Push( pEvt );
		}

		xgc_long asio_SocketMgr::Exec( xgc_long nStep )
		{
			INetPacket *pEvt = xgc_nullptr;

			while( nStep && mEvtQueue.Kick( &pEvt ) )
			{
				auto hNet = pEvt->handle();
				auto pHeader = (EventHeader*)pEvt->data();

				do 
				{
					switch( pHeader->event )
					{
					case EVENT_HANGUP:
						{
							auto pSocket = getSocket( hNet );
							if( xgc_nullptr == pSocket )
								break;

							auto srv = (asio_ServerBase*) pSocket->get_from();
							auto session = srv->CreateSession();
							if( !session )
							{
								pSocket->close();
							}
							else
							{
								pSocket->set_userdata( session );
								pSocket->accept( EVENT_ACCEPT );
							}
						}
						break;
					case EVENT_ACCEPT:
						{
							auto pSession = (INetworkSession*)pHeader->bring;
							XGC_ASSERT_BREAK( pSession );

							pSession->OnAccept( hNet );
						}
						break;
					case EVENT_CONNECT:
						{
							auto pSession = (INetworkSession*) pHeader->bring;
							XGC_ASSERT_BREAK( pSession );
							pSession->OnConnect( hNet );
						}
						break;
					case EVENT_CLOSE:
						{
							auto pSession = (INetworkSession*) pHeader->bring;
							XGC_ASSERT_BREAK( pSession );

							pSession->OnClose();
						}
						break;
					case EVENT_ERROR:
						{
							auto pSession = (INetworkSession*) pHeader->bring;
							XGC_ASSERT_BREAK( pSession );

							pSession->OnError( (xgc_uint32) pHeader->error );
						}
						break;
					case EVENT_DATA:
						{
							auto pSession = (INetworkSession*) pHeader->bring;
							XGC_ASSERT_BREAK( pSession );

							pSession->OnRecv( pHeader + 1, pHeader->error );
						}
						break;
					case EVENT_PING:
						{
							auto pSession = (INetworkSession*) pHeader->bring;
							XGC_ASSERT_BREAK( pSession );

							pSession->OnAlive();
						}
						break;
					}
				} while (false);

				pEvt->freedom();
				--nStep;
			}

			return nStep;
		}

		///
		/// \brief ���ӽ���
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:07
		///

		xgc_bool asio_SocketMgr::LinkUp( asio_SocketPtr pSocket )
		{
			std::lock_guard< std::mutex > _guard( lock_ );
			if( free_handles.empty() )
				return false;

			auto handler = free_handles.front();
			pSocket->set_handle( handler );
			handles[handler] = pSocket->shared_from_this();

			free_handles.pop();

			return true;
		}

		///
		/// \brief ���ӶϿ�
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:07
		///

		xgc_void asio_SocketMgr::LinkDown( asio_SocketPtr pSocket )
		{
			// Ϊ��ֹ ����˳���µ��������⣬���ﲻ���ùؼ�����������
			auto handle = pSocket->get_handle();
			if( handles[handle] == pSocket )
			{
				handles[handle] = xgc_nullptr;

				std::lock_guard< std::mutex > _guard( lock_ );
				free_handles.push( handle );
			}
		}

		xgc_void asio_SocketMgr::OnTimer( const asio::error_code & e, network_t handle, xgc_uint32 id, xgc_real64 period )
		{
			if( e == asio::error::operation_aborted )
				return;

			EventHeader evt;
			evt.handle = handle;
			evt.event = EVENT_TIMER;
			evt.error = id;
			evt.bring = xgc_nullptr;

			Push( CNetworkPacket::allocate( &evt, sizeof(evt), handle ) );

			std::lock_guard< std::mutex > _lock( lock_timer_ );
			auto it = mTimerMap.find( id );
			if( it != mTimerMap.end() )
			{
				auto timer = it->second;
				timer->expires_after( std::chrono::milliseconds( (xgc_uint32) (period * 1000) ) );
				timer->async_wait( std::bind( &asio_SocketMgr::OnTimer, this, std::placeholders::_1, handle, id, period ) );
			}
		}
	}
}
