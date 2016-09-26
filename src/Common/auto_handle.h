/*******************************************************************/
//! \file autolist.h
//! \brief 自动列表，用于管理对象ID
//! 2014/07/16 by Albert.xu
/*******************************************************************/
#pragma once
#ifndef _AUTO_HANDLE_H_
#define _AUTO_HANDLE_H_
#include "defines.h"
#include "exports.h"

namespace xgc
{
	namespace common
	{
		enum COMMON_API pool_handle_base 
		{
			pow_pos = 10,
			pow_chk = 12,
			pow_rnd = 6,
			pow_crc = 4 
		};

		template< typename base = pool_handle_base >
		struct pool_handle
		{
			typedef base mybase;
			union
			{
				xgc_uint32 _handle;
				struct
				{
					xgc_uint32	position : mybase::pow_pos;	///< 位置位域
					xgc_uint32	chunk    : mybase::pow_chk;	///< 分块
					xgc_uint32	round    : mybase::pow_rnd;	///< 轮询位域
					xgc_uint32	check    : mybase::pow_crc;	///< 对象位置标志
				};
			};

			pool_handle()
				: _handle( -1 )
			{
			}

			pool_handle( xgc_uint32 handle )
				: _handle( handle )
			{
			}

			pool_handle& operator=( const pool_handle& handle )
			{
				_handle = handle._handle;
				return *this;
			}

			pool_handle& operator=( xgc_uint32 handle )
			{
				_handle = handle;
				return *this;
			}
		};

		template struct COMMON_API pool_handle < pool_handle_base > ;
		extern COMMON_API const pool_handle< pool_handle_base > _my_invalid_handle_value_;

		template< typename base >
		XGC_INLINE xgc_bool operator == ( const pool_handle< base > &lhs, const pool_handle< base > &rhs )
		{
			return lhs._handle == rhs._handle;
		}

		template< typename base >
		XGC_INLINE xgc_bool operator != ( const pool_handle< base > &lhs, const pool_handle< base > &rhs )
		{
			return lhs._handle != rhs._handle;
		}

		template< typename base >
		XGC_INLINE xgc_bool operator < ( const pool_handle< base > &lhs, const pool_handle< base > &rhs )
		{
			return lhs._handle < rhs._handle;
		}

		template< typename _type, typename _handle_type = pool_handle >
		class auto_handle_pool;

		template< typename _type, typename _handle_type >
		auto_handle_pool< _type, _handle_type >& get_handle_pool();

		template< typename _type, typename _handle_type = pool_handle< pool_handle_base > >
		class auto_handle : public _handle_type
		{
			friend class auto_handle_pool < _type, _handle_type > ;
		public:
			///
			/// 获取句柄
			/// [7/16/2014] create by albert.xu
			///
			_handle_type handle()const { return *this; }

			///
			/// 通过对象ID获取对象指针
			/// [1/7/2014 albert.xu]
			///
			static _type* handle_exchange( _handle_type handle );

			///
			/// 获取句柄池对象
			/// [7/18/2014] create by albert.xu
			///
			static auto_handle_pool < _type, _handle_type >& pool();
		protected:
			///
			/// 分配新句柄
			/// [7/16/2014] create by albert.xu
			///
			static xgc_void new_handle( auto_handle* ptr );

			///
			/// 释放句柄
			/// [7/16/2014] create by albert.xu
			///
			static xgc_void del_handle( auto_handle* ptr );

			///
			/// 创建对象
			/// [7/16/2014] create by albert.xu
			///
			auto_handle()
			{
				new_handle( this );
			}

			///
			/// 销毁对象
			/// [7/16/2014] create by albert.xu
			///
			~auto_handle()
			{
				del_handle( this );
			}
		};

		//----------------------------------------------------------------------//
		template< typename _type, typename _handle_type >
		class auto_handle_pool
		{
			friend auto_handle_pool& get_handle_pool< _type, _handle_type >();
			friend xgc_void auto_handle< _type, _handle_type >::new_handle( auto_handle< _type, _handle_type >* ptr );
			friend xgc_void auto_handle< _type, _handle_type >::del_handle( auto_handle< _type, _handle_type >* ptr );

			//////////////////////////////////////////////////////////////////////////
			// 为防止静态库同时被多个动态库或执行文件共享的问题，这里只声明该Static对象。
			static auto_handle_pool *pInstance;

			typedef auto_handle< _type, _handle_type > *object_ptr;

			struct chunk
			{
				xgc_uint16 index;	///< chunk 的索引
				xgc_uint16 cursor;	///< 游标位置，永远指向一个可用的空位。除非这个Chunk满了。
				xgc_uint16 alloca;	///< 已分配的个数
				xgc_uint16 check;	///< 检查值

				object_ptr ptr[1 << _handle_type::mybase::pow_pos]; ///< 指针列表
			};

			// 按块儿来分，一共有1024个块，每个块有16384个可分配ID
			// 该值的取法为2的n次幂，这个值在添加物品的算法中会使用到，不懂的不要乱改，以免出现BUG
			chunk*		mChunk[1 << _handle_type::mybase::pow_chk];
			chunk*		mCurrentChunk;

			xgc_uint16	mCurrentRound;
			xgc_uint16	mChunkCount;

			xgc_size	mTotalHandle;
		protected:
			auto_handle_pool( xgc_uint8 nChunk = 8 )
				: mCurrentChunk( xgc_nullptr )
				, mCurrentRound( 0 )
				, mChunkCount( 0 )
				, mTotalHandle( 0 )
			{
				memset( mChunk, 0, sizeof( mChunk ) );
				while( nChunk-- )
				{
					mCurrentChunk = new_chunk( nChunk );
					XGC_ASSERT_THROW( mCurrentChunk, std::runtime_error( "CHUNK ALLOC ERROR!" ) );
				}
			}

			~auto_handle_pool()
			{
				for each( auto pChunk in mChunk )
				{
					SAFE_DELETE( pChunk );
				}
			}

			///
			/// 自动列表模式，将派生自XObject的对象都管理起来
			/// [5/2/2014 albert.xu]
			///
			xgc_void new_handle( object_ptr ptr )
			{
				chunk* pChunk = mCurrentChunk;

				// 先确定游标是否合法
				if( pChunk->cursor >= xgc_countof( pChunk->ptr ) )
				{
					pChunk->cursor = 0;
					// 开始游历Chunk
					do
					{
						pChunk = mChunk[( pChunk->index + 1 ) & ( ( 1 << _handle_type::mybase::pow_chk ) - 1 )];
						if( pChunk == xgc_nullptr )
							pChunk = mChunk[0];

						if( pChunk == mChunk[0] )
							++mCurrentRound;

						if( pChunk == mCurrentChunk )
							pChunk = new_chunk( mChunkCount );

						// 如果这里出问题了不如崩溃掉，还能看到堆栈
						assert( pChunk );
					} while( pChunk->alloca >= XGC_COUNTOF( pChunk->ptr ) );
				}

				// 找一个可用的位置
				while( pChunk->ptr[pChunk->cursor] )
				{
					if( ++pChunk->cursor >= xgc_countof( pChunk->ptr ) )
					{
						pChunk->cursor = 0;
						do
						{
							pChunk = mChunk[( pChunk->index + 1 ) & ( ( 1 << _handle_type::mybase::pow_chk ) - 1 )];
							if( pChunk == xgc_nullptr )
								pChunk = mChunk[0];

							if( pChunk == mChunk[0] )
								++mCurrentRound;

							if( pChunk == mCurrentChunk )
								pChunk = new_chunk( mChunkCount );

							// 如果这里出问题了不如崩溃掉，还能看到堆栈
							assert( pChunk );
						} while( pChunk->alloca >= XGC_COUNTOF( pChunk->ptr ) );
					}
				}
				mCurrentChunk = pChunk;

				ptr->check    = 0x0b & mCurrentChunk->check++;
				ptr->round    = mCurrentRound;
				ptr->chunk    = mCurrentChunk->index;
				ptr->position = mCurrentChunk->cursor;

				// 将对象指针放在句柄缓冲池中
				XGC_ASSERT( mCurrentChunk->ptr[mCurrentChunk->cursor] == xgc_nullptr );
				mCurrentChunk->ptr[mCurrentChunk->cursor] = ptr;
				mCurrentChunk->alloca++;
				mCurrentChunk->cursor++;
				mTotalHandle++;
			}

			///
			/// 自动列表模式，将派生自XObject的对象都管理起来
			/// [5/2/2014 albert.xu]
			///
			xgc_void del_handle( object_ptr ptr )
			{
				XGC_ASSERT_RETURN( ptr && exchange( ptr->handle() ) == ptr, xgc_void( 0 ) );

				mChunk[ptr->chunk]->ptr[ptr->position] = xgc_nullptr;
				mChunk[ptr->chunk]->alloca--;
				mTotalHandle--;
			}

			///
			/// 重新分配一个Chunk
			/// [5/2/2014 albert.xu]
			///
			chunk* new_chunk( xgc_uint16 nChunkIndex )
			{
				XGC_ASSERT_RETURN( nChunkIndex < XGC_COUNTOF( mChunk ) &&
					mChunk[nChunkIndex] == xgc_nullptr,
					xgc_nullptr );

				chunk* pChunk = XGC_NEW chunk;
				XGC_ASSERT_RETURN( pChunk, xgc_nullptr );

				pChunk->index  = nChunkIndex;
				pChunk->cursor = 0;
				pChunk->alloca = 0;
				pChunk->check  = 1;

				memset( pChunk->ptr, 0, sizeof( pChunk->ptr ) );
				mChunk[nChunkIndex] = pChunk;

				++mChunkCount;
				return pChunk;
			}

		public:
			///
			///	通过句柄获取对象指针
			/// @param handle 交换的句柄
			///
			_type* exchange( _handle_type handle )
			{
				if( mChunk[handle.chunk] == xgc_nullptr )
					return xgc_nullptr;

				// 故意将下标写这么大，反正不会访问里面的数据
				if( handle.position >= ( 1 << _handle_type::mybase::pow_chk ) )
					return xgc_nullptr;

				_type* ptr = static_cast< _type* >( mChunk[handle.chunk]->ptr[handle.position] );
				return ptr && handle == ptr->handle() ? ptr : xgc_nullptr;
			}

			///
			/// 获取已分配的句柄数
			/// [7/18/2014] create by albert.xu
			///
			xgc_size total_handle()const
			{
				return mTotalHandle;
			}
		};

		///
		/// 获取对象池
		/// [1/7/2014 albert.xu]
		///
		template< typename _type, typename _handle_type >
		auto_handle_pool< _type, _handle_type >& get_handle_pool()
		{
			static auto_handle_pool < _type, _handle_type > inst;
			return inst;
		}

		///
		/// 通过对象ID获取对象指针
		/// [1/7/2014 albert.xu]
		///
		template< typename _type, typename _handle_type >
		_type* auto_handle< _type, _handle_type >::handle_exchange( _handle_type handle )
		{
			return get_handle_pool< _type, _handle_type >().exchange( handle );
		}

		///
		/// 通过对象ID获取对象指针
		/// [1/7/2014 albert.xu]
		///
		template< typename _type, typename _handle_type >
		auto_handle_pool < _type, _handle_type >& auto_handle< _type, _handle_type >::pool()
		{
			return get_handle_pool< _type, _handle_type >();
		}

		///
		/// 分配新句柄
		/// [7/16/2014] create by albert.xu
		///
		template< typename _type, typename _handle_type >
		xgc_void auto_handle< _type, _handle_type >::new_handle( auto_handle< _type, _handle_type >* ptr )
		{
			get_handle_pool< _type, _handle_type >().new_handle( ptr );
		}

		///
		/// 释放句柄
		/// [7/16/2014] create by albert.xu
		///
		template< typename _type, typename _handle_type >
		xgc_void auto_handle< _type, _handle_type >::del_handle( auto_handle< _type, _handle_type >* ptr )
		{
			get_handle_pool< _type, _handle_type >().del_handle( ptr );
		}
	}
}
#endif // _AUTO_HANDLE_H_