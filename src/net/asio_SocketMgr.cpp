#include "asio_Header.h"
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
		/// \brief 等待所有连接关闭
		///
		/// \author albert.xu
		/// \date 2016/02/24 18:20
		///

		xgc_void asio_SocketMgr::Final( xgc_ulong timeout )
		{
			auto start = clock();
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

			} while( counter > 0 && (clock() - start < (clock_t) timeout) );
		}

		asio_SocketPtr asio_SocketMgr::getSocket( network_t handle )
		{
			XGC_ASSERT_RETURN( handle < XGC_COUNTOF( handles ), xgc_nullptr );

			return handles[handle];
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

			clock_t start = clock();
			while( clock() - start < timeout )
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

		///
		/// \brief 连接建立
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
			pSocket->set_handler( handler );
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

		xgc_void asio_SocketMgr::LinkDown( asio_SocketPtr pSocket )
		{
			// 为防止 锁定顺序导致的死锁问题，这里不是用关键区锁保护。
			auto handle = pSocket->get_handler();
			if( handles[handle] == pSocket )
			{
				handles[handle] = xgc_nullptr;

				std::lock_guard< std::mutex > _guard( lock_ );
				free_handles.push( handle );
			}
		}
	}
}
