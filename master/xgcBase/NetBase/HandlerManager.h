#pragma once

#include "defines.h"
#include "Netbase.h"
#include <queue>
#include <set>
#include <map>
#include <mutex>

using namespace XGC::common;
#define MAX_HANDLE_COUNT	0xffff
#define PROTOCAL_POOL_SIZE	0xff

namespace XGC
{
	namespace net
	{
		template< class NetworkHandler >
		class CHandlerManager
		{
		friend typename NetworkHandler;
		friend CHandlerManager< typename NetworkHandler >& getHandleManager();
		public:
			struct CHandleGroup : public std::set< group_t >
			{
				typedef std::set< group_t >::iterator pos;
				typedef std::set< group_t >::const_iterator cpos;

				std::mutex		group_guard_;
				network_t		network_;
				CHandleGroup( network_t handle )
					: network_( handle )
				{
				}

				~CHandleGroup()
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
			CHandlerManager(xgc_void)
			{
			}

			~CHandlerManager(xgc_void)
			{
				WaitForCloseAll( 0 );
			}

			CHandlerManager( const CHandlerManager& );
			CHandlerManager& operator =( const CHandlerManager& ) = delete;

		public:
			xgc_void Initialize( xgc_uint32 nMaxHandleCount = MAX_HANDLE_COUNT )
			{
				m_max_handle_count = nMaxHandleCount;

				m_handle_vector.resize( nMaxHandleCount, NULL );
				for( xgc_uint16 i = 0; i < nMaxHandleCount; ++i )
				{
					m_free_handle.push( i );
				}

				for( xgc_uint16 i = 0; i < nMaxHandleCount; ++i )
				{
					m_free_group.push( i );
				}
			}

			xgc_void Final()
			{
				WaitForCloseAll( 0 );
			}

			NetworkHandler* FetchHandle( network_t handle )
			{
				if( handle < m_handle_vector.size() )
				{
					std::lock_guard< std::mutex > _guard( m_guard );
					volatile asio_NetworkHandler *&pHandle = m_handle_vector[handle];
					NetworkHandler* pHandler = reinterpret_cast<NetworkHandler*>( InterlockedExchangePointer( (volatile PVOID*)&pHandle, (PVOID)pHandle ) );

					if( pHandler )
					{
						pHandler->Fetch();
						return reinterpret_cast<NetworkHandler*>( InterlockedExchangePointer( (volatile PVOID*)&pHandle, (PVOID)pHandle ) ); // 防止在FreeHandle中删除了指针后引发的野指针调用。
					}
				}
				return xgc_nullptr;
			}

			xgc_void FreeHandle( NetworkHandler* pHandler )
			{
				if( pHandler )
				{
					pHandler->Free();
				}
			}

			size_t HandleCount()const
			{
				return MAX_HANDLE_COUNT - m_free_handle.size();
			}

			xgc_void CloseHandler( network_t nHandle )
			{
				if( nHandle > m_handle_vector.size() )
					return;

				NetworkHandler* pHandler = FetchHandle(nHandle);
				if( pHandler )
				{
					// 如果有重入，则这里会进行保护，防止野指针操作。
					if( pHandler == m_handle_vector[nHandle] )
						pHandler->CloseHandler();
				}
			}

			xgc_void CloseAll( xgc_uintptr from = 0 )
			{
				for( network_t i = 0; i < m_handle_vector.size(); ++i )
				{
					NetworkHandler* pHandler = FetchHandle( i );
					if( pHandler && ( from == 0 || pHandler->Belong( from ) ) )
					{
						pHandler->CloseHandler();
					}
				}
			}

			xgc_bool WaitForClose( network_t handle, xgc_uint32 sleep = 100, xgc_int32 timeout = INFINITE )
			{
				if( handle >= m_handle_vector.size() )
					return;

				clock_t start = clock();
				while( clock() - start < timeout )
				{
					NetworkHandler* pHandler = FetchHandle( i );
					if( pHandler )
					{
						pHandler->CloseHandler();
					}
					else
					{
						return true;
					}
					Sleep( sleep );
				}

				return false;
			}

			xgc_uint32 WaitForCloseAll( xgc_uintptr from, xgc_uint32 sleep = 100, xgc_int32 timeout = INFINITE )
			{
				clock_t start = clock();
				xgc_int32 counter = 0;
				do
				{
					Sleep( sleep );

					counter = 0;
					for( network_t i = 0; i < m_handle_vector.size() ; ++i )
					{
						NetworkHandler* pHandler = FetchHandle( i );
						if( pHandler && ( pHandler->Belong( from ) || from == 0 ) )
						{
							pHandler->CloseHandler();
							++counter;
						}
					}
				}while( counter && clock() - start < timeout );

				return counter;
			}

			group_t NewGroup( network_t self_handle )
			{
				std::lock_guard< std::mutex > _l( m_group_guard );
				if( m_free_group.empty() )
				{
					return INVALID_NETWORK_HANDLE;
				}
				else
				{
					group_t group = m_free_group.front();
					auto ret = m_group_map.insert( CGroupMap::value_type( group, XGC_NEW CHandleGroup(self_handle) ) );
					if( ret.second == false )
					{
						return INVALID_NETWORK_HANDLE;
					}
					m_free_group.pop();
					return group;
				}

				return INVALID_NETWORK_HANDLE;
			}

			// param [in]: in - GroupParam{ xgc_uint32 handle; xgc_uint32 group; } *(xgc_uint32*)param, *(((xgc_uint32*)param)+1)
			// reutrn >0 成功 -1 指定的组已不存在 -2 网络句柄已被添加 -3 添加句柄到组失败
			xgc_long EnterGroup( group_t group, network_t handle )
			{
				CHandleGroup* pGroup = FetchHandleGroup( group );
				if( pGroup )
				{
					if( pGroup->find( handle ) == pGroup->end() )
					{
						CHandleGroup::_Pairib ret = pGroup->insert( handle );
						auto size = (xgc_long)pGroup->size();
						FreeHandleGroup(pGroup);
						return ret.second?size:-3;
					}
					else
					{
						FreeHandleGroup(pGroup);
						return -2;
					}
				}

				return -1;
			}

			// param [in]: in - GroupParam{ xgc_uint32 handle; xgc_uint32 group; }
			// return >0 成功返回当前组中的句柄个数, -1 指定的组已不存在 -2 网络句柄不存在
			xgc_long LeaveGroup( group_t group, network_t handle )
			{
				CHandleGroup* pGroup = FetchHandleGroup( group );
				if( pGroup )
				{
					if( pGroup->erase( handle ) == 0 )
					{
						FreeHandleGroup( pGroup );
						return -2;
					}

					auto size = (xgc_long)pGroup->size();
					FreeHandleGroup( pGroup );
					return size;
				}

				return -1;
			}

			xgc_long RemoveGroup( group_t group )
			{
				std::lock_guard< std::mutex > _l( m_group_guard );
				CGroupMap::iterator i = m_group_map.find( group );
				if( i != m_group_map.end() )
				{
					CHandleGroup* pGroup = i->second;
					m_group_map.erase( i );
					delete pGroup;
					return 0;
				}

				return -1;
			}

			CHandleGroup* FetchHandleGroup( group_t group )
			{
				std::lock_guard< std::mutex > _l( m_group_guard );
				CGroupMap::iterator i = m_group_map.find( group );
				if( i != m_group_map.end() )
				{
					CHandleGroup* pGroup = i->second;
					if( pGroup )
					{
						pGroup->lock_handle();
						return pGroup;
					}
				}
				return xgc_nullptr;
			}

			xgc_void FreeHandleGroup( CHandleGroup* pGroup )const
			{
				pGroup->free_handle();
			}

		protected:
			xgc_bool LinkUp( NetworkHandler* pHandler )
			{
				std::lock_guard< std::mutex > _guard( m_guard );
				if( m_free_handle.empty() ) 
					return false;

				pHandler->SetHandler( m_free_handle.front() );
				m_free_handle.pop();
				// 为防止 锁定顺序导致的死锁问题，这里不是用关键区锁保护。

				return reinterpret_cast< asio_NetworkHandler* >( InterlockedCompareExchangePointer( (volatile PVOID*)&m_handle_vector[pHandler->GetHandler()], (PVOID)pHandler, NULL ) ) == xgc_nullptr;
			}

			xgc_void LinkDown( NetworkHandler* pHandler )
			{
				// 为防止 锁定顺序导致的死锁问题，这里不是用关键区锁保护。
				if( reinterpret_cast< asio_NetworkHandler* >( InterlockedCompareExchangePointer( (volatile PVOID*)&m_handle_vector[pHandler->GetHandler()], NULL, (PVOID)pHandler ) ) )
				{
					std::lock_guard< std::mutex > _guard( m_guard );
					m_free_handle.push( pHandler->GetHandler() );
				}
			}

		private:
			typedef std::queue< network_t > CFreeHandleList;
			typedef std::map< group_t, CHandleGroup* > CGroupMap;
			typedef std::vector< volatile NetworkHandler* > CHandleVec;

			CFreeHandleList	m_free_handle;
			CFreeHandleList m_free_group;

			xgc_uint32		m_max_handle_count;

			CHandleVec		m_handle_vector;
			CGroupMap		m_group_map;

			std::mutex		m_guard;
			std::mutex		m_group_guard;
		};
	}
}