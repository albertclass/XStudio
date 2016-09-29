/*******************************************************************/
//! \file autolist.h
//! \brief �Զ��б����ڹ������ID
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
					xgc_uint32	position : mybase::pow_pos;	///< λ��λ��
					xgc_uint32	chunk    : mybase::pow_chk;	///< �ֿ�
					xgc_uint32	round    : mybase::pow_rnd;	///< ��ѯλ��
					xgc_uint32	check    : mybase::pow_crc;	///< ����λ�ñ�־
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
			/// ��ȡ���
			/// [7/16/2014] create by albert.xu
			///
			_handle_type handle()const { return *this; }

			///
			/// ͨ������ID��ȡ����ָ��
			/// [1/7/2014 albert.xu]
			///
			static _type* handle_exchange( _handle_type handle );

			///
			/// ��ȡ����ض���
			/// [7/18/2014] create by albert.xu
			///
			static auto_handle_pool < _type, _handle_type >& pool();
		protected:
			///
			/// �����¾��
			/// [7/16/2014] create by albert.xu
			///
			static xgc_void new_handle( auto_handle* ptr );

			///
			/// �ͷž��
			/// [7/16/2014] create by albert.xu
			///
			static xgc_void del_handle( auto_handle* ptr );

			///
			/// ��������
			/// [7/16/2014] create by albert.xu
			///
			auto_handle()
			{
				new_handle( this );
			}

			///
			/// ���ٶ���
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
			// Ϊ��ֹ��̬��ͬʱ�������̬���ִ���ļ���������⣬����ֻ������Static����
			static auto_handle_pool *pInstance;

			typedef auto_handle< _type, _handle_type > *object_ptr;

			struct chunk
			{
				xgc_uint16 index;	///< chunk ������
				xgc_uint16 cursor;	///< �α�λ�ã���Զָ��һ�����õĿ�λ���������Chunk���ˡ�
				xgc_uint16 alloca;	///< �ѷ���ĸ���
				xgc_uint16 check;	///< ���ֵ

				object_ptr ptr[1 << _handle_type::mybase::pow_pos]; ///< ָ���б�
			};

			// ��������֣�һ����1024���飬ÿ������16384���ɷ���ID
			// ��ֵ��ȡ��Ϊ2��n���ݣ����ֵ�������Ʒ���㷨�л�ʹ�õ��������Ĳ�Ҫ�Ҹģ��������BUG
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
			/// �Զ��б�ģʽ����������XObject�Ķ��󶼹�������
			/// [5/2/2014 albert.xu]
			///
			xgc_void new_handle( object_ptr ptr )
			{
				chunk* pChunk = mCurrentChunk;

				// ��ȷ���α��Ƿ�Ϸ�
				if( pChunk->cursor >= xgc_countof( pChunk->ptr ) )
				{
					pChunk->cursor = 0;
					// ��ʼ����Chunk
					do
					{
						pChunk = mChunk[( pChunk->index + 1 ) & ( ( 1 << _handle_type::mybase::pow_chk ) - 1 )];
						if( pChunk == xgc_nullptr )
							pChunk = mChunk[0];

						if( pChunk == mChunk[0] )
							++mCurrentRound;

						if( pChunk == mCurrentChunk )
							pChunk = new_chunk( mChunkCount );

						// �������������˲�������������ܿ�����ջ
						assert( pChunk );
					} while( pChunk->alloca >= XGC_COUNTOF( pChunk->ptr ) );
				}

				// ��һ�����õ�λ��
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

							// �������������˲�������������ܿ�����ջ
							assert( pChunk );
						} while( pChunk->alloca >= XGC_COUNTOF( pChunk->ptr ) );
					}
				}
				mCurrentChunk = pChunk;

				ptr->check    = 0x0b & mCurrentChunk->check++;
				ptr->round    = mCurrentRound;
				ptr->chunk    = mCurrentChunk->index;
				ptr->position = mCurrentChunk->cursor;

				// ������ָ����ھ���������
				XGC_ASSERT( mCurrentChunk->ptr[mCurrentChunk->cursor] == xgc_nullptr );
				mCurrentChunk->ptr[mCurrentChunk->cursor] = ptr;
				mCurrentChunk->alloca++;
				mCurrentChunk->cursor++;
				mTotalHandle++;
			}

			///
			/// �Զ��б�ģʽ����������XObject�Ķ��󶼹�������
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
			/// ���·���һ��Chunk
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
			///	ͨ�������ȡ����ָ��
			/// @param handle �����ľ��
			///
			_type* exchange( _handle_type handle )
			{
				if( mChunk[handle.chunk] == xgc_nullptr )
					return xgc_nullptr;

				// ���⽫�±�д��ô�󣬷�������������������
				if( handle.position >= ( 1 << _handle_type::mybase::pow_chk ) )
					return xgc_nullptr;

				_type* ptr = static_cast< _type* >( mChunk[handle.chunk]->ptr[handle.position] );
				return ptr && handle == ptr->handle() ? ptr : xgc_nullptr;
			}

			///
			/// ��ȡ�ѷ���ľ����
			/// [7/18/2014] create by albert.xu
			///
			xgc_size total_handle()const
			{
				return mTotalHandle;
			}
		};

		///
		/// ��ȡ�����
		/// [1/7/2014 albert.xu]
		///
		template< typename _type, typename _handle_type >
		auto_handle_pool< _type, _handle_type >& get_handle_pool()
		{
			static auto_handle_pool < _type, _handle_type > inst;
			return inst;
		}

		///
		/// ͨ������ID��ȡ����ָ��
		/// [1/7/2014 albert.xu]
		///
		template< typename _type, typename _handle_type >
		_type* auto_handle< _type, _handle_type >::handle_exchange( _handle_type handle )
		{
			return get_handle_pool< _type, _handle_type >().exchange( handle );
		}

		///
		/// ͨ������ID��ȡ����ָ��
		/// [1/7/2014 albert.xu]
		///
		template< typename _type, typename _handle_type >
		auto_handle_pool < _type, _handle_type >& auto_handle< _type, _handle_type >::pool()
		{
			return get_handle_pool< _type, _handle_type >();
		}

		///
		/// �����¾��
		/// [7/16/2014] create by albert.xu
		///
		template< typename _type, typename _handle_type >
		xgc_void auto_handle< _type, _handle_type >::new_handle( auto_handle< _type, _handle_type >* ptr )
		{
			get_handle_pool< _type, _handle_type >().new_handle( ptr );
		}

		///
		/// �ͷž��
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