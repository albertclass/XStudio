#include "Header.h"
#include "asio_SocketMgr.h"
#include "asio_Network.h"

#include <thread>
#include <chrono>

namespace xgc
{
	namespace net
	{
		using xgc::common::current_milliseconds;
		const int handle_count = 0xffff;

		/// SocketMgr 对象是否激活
		static volatile std::atomic_long SocketMgrAlive = -1;

		asio_SocketMgr::asio_SocketMgr()
			: exec_( current_milliseconds() )
			, exec_inc_( 0 )
			, exec_dec_( 0 )
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

			// 设置状态为已初始化
			SocketMgrAlive = 0;
		}

		///
		/// \brief 等待所有连接关闭
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:20
		///

		xgc_void asio_SocketMgr::Final( xgc_ulong timeout )
		{
			auto start = tick();
			auto counter = 0;

			xgc_long exp = 0;
			// 设置状态为等待关闭
			if( SocketMgrAlive.compare_exchange_strong( exp, 1 ) )
			{
				do
				{
					std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

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

					// 处理网络事件
					Exec( -1 );
				} while( counter > 0 && ( tick() - start < timeout ) );

				// execute socket close event.
				while( -1 != Exec( -1 ) )
				{
					std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
				}

				// 设置状态为已关闭
				SocketMgrAlive = 2;
			}
		}

		asio_SocketPtr asio_SocketMgr::getSocket( network_t handle )
		{
			XGC_ASSERT_RETURN( handle < XGC_COUNTOF( handles ), xgc_nullptr );

			return handles[handle];
		}

		xgc_bool asio_SocketMgr::NewTimer( xgc_uint32 nTimerId, xgc_real64 fPeriod, xgc_real64 fAfter, const std::function< void() > &onTimer )
		{
			std::lock_guard< std::mutex > _lock( lock_timer_ );

			auto it = mTimerMap.find( nTimerId );
			if( it == mTimerMap.end() )
			{
				timer_info &info = mTimerMap[nTimerId];
				info.on_timer = onTimer;

				info.timer = XGC_NEW asio::steady_timer( getNetwork().Ref() );
				info.timer->expires_after( std::chrono::milliseconds( (xgc_uint32)(fAfter * 1000) ) );
				info.timer->async_wait( std::bind( &asio_SocketMgr::OnTimer, this, std::placeholders::_1, nTimerId, fPeriod ) );

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
				timer_info &info = it->second;
				asio::error_code ec;
				if( info.timer->cancel( ec ) )
				{
					SAFE_DELETE( info.timer );
					mTimerMap.erase( it );
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
		/// \brief 等待所有套接字关闭
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
		/// \brief 新建组
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
		/// \brief 进入组
		///
		/// \param handle GroupParam{ xgc_uint32 handle; xgc_uint32 group; } *(xgc_uint32*)param, *(((xgc_uint32*)param)+1)
		/// \author albert.xu
		/// \reutrn >0 成功 -1 指定的组已不存在 -2 网络句柄已被添加 -3 添加句柄到组失败
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
		/// \brief 离开组
		///
		/// \param handle 网络句柄
		/// \return >0 成功返回当前组中的句柄个数, -1 指定的组已不存在 -2 网络句柄不存在
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
		/// \brief 删除组
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
		/// \brief 获取组对象
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
		/// \brief 归还组对象
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:07
		///

		xgc_void asio_SocketMgr::FreeHandleGroup( CSocketGroup * pGroup ) const
		{
			pGroup->free_handle();
		}

		xgc_void asio_SocketMgr::Push( xgc_lpvoid data, xgc_size size )
		{
			++exec_inc_;
			std::lock_guard< std::mutex > _lock( lock_queue_ );

			event_queue_.push( { data, size } );
		}

		xgc_bool asio_SocketMgr::Kick( xgc_lpvoid &data, xgc_size &size )
		{
			std::lock_guard< std::mutex > _lock( lock_queue_ );
			if( event_queue_.empty() )
				return false;

			auto pkg = event_queue_.front();
			data = std::get<0>( pkg );
			size = std::get<1>( pkg );

			event_queue_.pop();

			++exec_dec_;
			return true;
		}

		xgc_long asio_SocketMgr::Exec( xgc_long nStep )
		{
			xgc_lpvoid data = xgc_nullptr;
			
			xgc_size size = 0;

			for( ;nStep && Kick( data, size ); --nStep )
			{
				xgc_time64 diff = current_milliseconds() - exec_;
				if( diff >= 1000 )
				{
					NET_INFO( "circle %llu ms inc %llu dec %llu", diff, exec_inc_.load(), exec_dec_.load() );
					exec_ = current_milliseconds();
					exec_inc_ = exec_dec_ = 0;
				}

				XGC_ASSERT_CONTINUE( size >= NET_EVENT_SIZE );

				auto pHeader = (NetEvent*)data;

				do 
				{
					// NET_INFO( "handle=%u, event=%u, error=%u, bring=%p", pHeader->handle, pHeader->event, pHeader->error, pHeader->session );
					switch( pHeader->event )
					{
					case EVENT_PENDING:
						{
							auto pSocket = getSocket( pHeader->handle );
							if( xgc_nullptr == pSocket )
								break;

							auto srv = (asio_Server*) pSocket->get_from();
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
							auto pSession = (INetworkSession*)pHeader->session;
							XGC_ASSERT_BREAK( pSession );

							pSession->OnAccept( pHeader->handle );
						}
						break;
					case EVENT_CONNECT:
						{
							auto pSession = (INetworkSession*) pHeader->session;
							XGC_ASSERT_BREAK( pSession );
							pSession->OnConnect( pHeader->handle );
						}
						break;
					case EVENT_CLOSE:
						{
							auto pSession = (INetworkSession*) pHeader->session;
							XGC_ASSERT_BREAK( pSession );

							pSession->OnClose();
						}
						break;
					case EVENT_ERROR:
						{
							auto pSession = (INetworkSession*) pHeader->session;
							XGC_ASSERT_BREAK( pSession );
							auto pError = (NetError*)( pHeader + 1 );
							pSession->OnError( pError->error_type, pError->error_code );
						}
						break;
					case EVENT_DATA:
						{
							auto pSession = (INetworkSession*) pHeader->session;
							XGC_ASSERT_BREAK( pSession );
							auto pPacket = (NetData*)( pHeader + 1 );

							pSession->OnRecv( pPacket + 1, pPacket->length );
						}
						break;
					case EVENT_PING:
						{
							auto pSession = (INetworkSession*) pHeader->session;
							XGC_ASSERT_BREAK( pSession );

							pSession->OnAlive();
						}
						break;
					case EVENT_TIMER:
						{
							std::lock_guard< std::mutex > _lock( lock_timer_ );

							auto it = mTimerMap.find( pHeader->handle );
							if( it != mTimerMap.end() )
							{
								timer_info &info = it->second;
								info.on_timer();
							}
						}
						break;
					}
				} while (false);

				free( data );
			}

			return nStep;
		}

		///
		/// \brief 连接建立
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:07
		///

		xgc_bool asio_SocketMgr::LinkUp( asio_SocketPtr &pSocket )
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
		/// \brief 连接断开
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:07
		///

		xgc_void asio_SocketMgr::LinkDown( asio_SocketPtr &pSocket )
		{
			// 为防止 锁定顺序导致的死锁问题，这里不是用关键区锁保护。
			auto handle = pSocket->get_handle();
			if( handles[handle] == pSocket )
			{
				handles[handle] = xgc_nullptr;

				std::lock_guard< std::mutex > _guard( lock_ );
				free_handles.push( handle );
			}
		}

		xgc_void asio_SocketMgr::OnTimer( const asio::error_code & e, xgc_uint32 id, xgc_real64 period )
		{
			if( e == asio::error::operation_aborted )
				return;

			NetEvent evt;
			evt.handle = id;
			evt.event = EVENT_TIMER;
			evt.session = xgc_nullptr;

			auto data = malloc( NET_EVENT_SIZE );
			memcpy( data, &evt, NET_EVENT_SIZE );

			Push( data, NET_EVENT_SIZE );

			std::lock_guard< std::mutex > _lock( lock_timer_ );
			auto it = mTimerMap.find( id );
			if( it != mTimerMap.end() )
			{
				timer_info &info = it->second;
				info.timer->expires_after( std::chrono::milliseconds( (xgc_uint32) (period * 1000) ) );
				info.timer->async_wait( std::bind( &asio_SocketMgr::OnTimer, this, std::placeholders::_1, id, period ) );
			}
		}

		///
		/// \brief 连接建立
		///
		/// \author albert.xu
		/// \date 2017/07/18 18:07
		///
		xgc_bool LinkUp( asio_SocketPtr &pSocket )
		{
			XGC_ASSERT_RETURN( IsValidSocketMgr(), false );
			return getSocketMgr().LinkUp( pSocket );
		}

		///
		/// \brief 连接断开
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:07
		///
		xgc_void LinkDown( asio_SocketPtr &pSocket )
		{
			getSocketMgr().LinkDown( pSocket );
		}

		///
		/// \brief 管理器是否有效
		///
		/// \author albert.xu
		/// \date 2017/07/18 18:07
		///
		xgc_bool IsValidSocketMgr()
		{
			return SocketMgrAlive == 0;
		}
	}
}
