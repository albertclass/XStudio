#pragma once
#ifndef _FREELIST_H_
#define _FREELIST_H_

#include "defines.h"
#include "commonspp.h"
#include <list>

namespace xgc
{
	namespace memory
	{
		template< class T, class M >
		struct pool_object;

		template< class T, class ThreadPolicy >
		class freelist
		{
		public:
			freelist()
				: max_list_size( 0 )
				, once_alloc_count( 10 )
			{
			}

			freelist( xgc_size init_count, xgc_size upper_count, xgc_size once_alloc = 10 )
				: max_list_size( upper_count )
				, once_alloc_count( once_alloc )
			{
				alloc_once( init_count, _NORMAL_BLOCK, __FILE__, __LINE__ );
			}

			~freelist()
			{
				for( auto iter = free_list.begin(); iter != free_list.end(); ++iter )
				{
					xgc_lpvoid p = *iter;
					::free( p );
				}
				free_list.clear();
			}

		protected:
			xgc_size alloc_once( xgc_size alloc_count, xgc_int32 block, xgc_lpcstr file, xgc_int32 line )
			{
				mthread.lock();
				xgc_size count = 0;
				for( ; count < alloc_count; ++count )
				{
					xgc_lpvoid pobject = _malloc_dbg( sizeof( T ), block, file, line );
					if( pobject )
						free_list.push_back( pobject );
					else
						break;
				}
				mthread.free_lock( );
				return count;
			}

		public:
			//-----------------------------------//
			// [12/30/2013 albert.xu]
			// 分配一个对象,返回指针
			//-----------------------------------//
			xgc_lpvoid alloc_memory( xgc_int32 block, xgc_lpcstr file, xgc_int32 line )
			{
				mthread.lock();

				if( free_list.empty() )
				{
					if( alloc_once( once_alloc_count, block, file, line ) == 0 )
					{
						mthread.free_lock();
						return xgc_nullptr;
					}
				}

				xgc_lpvoid pobject = free_list.front();
				XGC_ASSERT( pobject );
				free_list.pop_front();

				mthread.free_lock();
				return pobject;
			}

			xgc_void garbage_memory( xgc_lpvoid p )
			{
				if( free_list.size() > max_list_size )
				{
					::free( p );
				}
				else
				{
					mthread.lock();

					free_list.push_back( p );

					mthread.free_lock();
				}
			}

			//-----------------------------------//
			// [12/30/2013 albert.xu]
			// 分配一个对象,并调用构造函数
			//-----------------------------------//
			XGC_INLINE T* alloc()
			{
				T* pMem = (T*)alloc_memory( _NORMAL_BLOCK, __FILE__, __LINE__ );
				if( pMem ) { new (pMem) T(); }

				return pMem;
			}

			//-----------------------------------//
			// [12/30/2013 albert.xu]
			// 带构造参数的对象分配
			//-----------------------------------//
#			define XGC_ALLOC_REPEAT(N)\
			template< XGC_ENUM_D(N,class P) >\
			XGC_INLINE T* alloc( XGC_PARAM_D(N, P, &p) )\
			{\
				T* pMem = alloc_memory( _NORMAL_BLOCK, __FILE__, __LINE__ ); \
				if( pMem ) { new (pMem)T( XGC_ENUM_D(N, p) ); }\
				return pMem;\
			}\

			XGC_MAIN_REPEAT( 10, XGC_ALLOC_REPEAT )
			//-----------------------------------//
			// [12/30/2013 albert.xu]
			// 释放一个对象
			//-----------------------------------//
			xgc_void dealloc( T* p )
			{
				p->~T();
				garbage_memory( p );
			}

		private:
			// 池中最多
			xgc_size max_list_size, once_alloc_count;

			// 未分配内存池
			std::list< xgc_lpvoid > free_list;

			// 线程模型
			ThreadPolicy mthread;
		};

		//-----------------------------------//
		// [12/31/2013 albert.xu]
		// 可被继承的
		// 继承该类后可自动转为使用空闲列表的对象
		// 继承后需要在.cpp中对pool对象进行声明
		// static pool_object< T, M >::pool( l, u, a );
		//-----------------------------------//
		template< class T, class M >
		struct pool_object
		{
			static freelist< T, M > pool;

			static void * operator new( xgc_size s )
			{
				return pool.alloc_memory( 0, xgc_nullptr, 0 );
			}
			
			static void * operator new( xgc_size s, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line )
			{
				return pool.alloc_memory( block, file, line );
			}

			static void operator delete( void * p )
			{
				pool.garbage_memory( p );
			}

			static void operator delete( void * p, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line )
			{
				pool.garbage_memory( p );
			}
		};
	}
}
#endif //_FREELIST_H_