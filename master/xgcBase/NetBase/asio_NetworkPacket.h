#ifndef _NETWORK_PACKET_H_
#define _NETWORK_PACKET_H_
#include "Netbase.h"
#include <time.h>
#include <memory>

namespace XGC
{
	namespace net
	{
		const xgc_size __packet_buffer_guard_size = 8;
		class asio_NetworkPacket : public INetPacket
		{
		friend struct asio_Socket;
		public:
			enum mode{ swd = 1, srd = 2, srw = 3 };

			asio_NetworkPacket( xgc_size capacity )
				: rd_(0)
				, wd_(0)
				, link_(INVALID_NETWORK_HANDLE)
				, userdata_(xgc_nullptr)
				, timestamp_(0)
				, buffer_( (xgc_byte*)malloc( capacity + __packet_buffer_guard_size * 2 ) )
				, buffer_size_( capacity + __packet_buffer_guard_size * 2 )
			{
				*(xgc_uint64*) (buffer_) = 0xebebebebebebebebULL;
				*(xgc_uint64*) (buffer_ + capacity + 8) = 0xededededededededULL;
				stamptime();
			}

			asio_NetworkPacket( xgc_size capacity, network_t lnk, xgc_lpvoid ud )
				: rd_(0)
				, wd_(0)
				, link_(lnk)
				, userdata_(ud)
				, timestamp_(0)
				, buffer_( (xgc_byte*)malloc( capacity + __packet_buffer_guard_size * 2 ) )
				, buffer_size_( capacity + __packet_buffer_guard_size * 2 )
			{
				*(xgc_uint64*) (buffer_) = 0xebebebebebebebebULL;
				*(xgc_uint64*) (buffer_ + __packet_buffer_guard_size + capacity ) = 0xededededededededULL;
				stamptime();
			}

			~asio_NetworkPacket()
			{
				XGC_ASSERT( 
					*(xgc_uint64*) (buffer_) == 0xebebebebebebebebULL && 
					*(xgc_uint64*) (buffer_ + __packet_buffer_guard_size + capacity()) == 0xededededededededULL );
				
				free( buffer_ );
			}

			XGC_INLINE xgc_void done( network_t h, xgc_lpvoid u )
			{ 
				link_ = h; 
				userdata_ = u; 
				stamptime();
			}

		private:
			XGC_INLINE xgc_size rd()const{ return rd_; }
			XGC_INLINE xgc_size wd()const{ return wd_; }

			XGC_INLINE const xgc_byte *rd_ptr()const{ return base() + rd(); }
			XGC_INLINE const xgc_byte *wd_ptr()const{ return base() + wd();	}

			XGC_INLINE xgc_byte *rd_ptr(){ return base() + rd(); }
			XGC_INLINE xgc_byte *wd_ptr(){ return base() + wd(); }

		public:
			XGC_INLINE xgc_byte *base()
			{
				return buffer_ + __packet_buffer_guard_size;
			}

			XGC_INLINE const xgc_byte *base()const
			{
				return buffer_+ __packet_buffer_guard_size;
			}

			XGC_INLINE xgc_size space()const
			{
				return buffer_size_ - wd_;
			}

			XGC_INLINE xgc_size region()const
			{
				return wd_ - rd_;
			}

			XGC_INLINE xgc_size rd_plus( xgc_size pos )
			{
				return rd_ += pos;
			}

			XGC_INLINE xgc_size wd_plus( xgc_size pos )
			{
				return wd_ += pos;
			}

			XGC_INLINE xgc_size rd_reset( xgc_size pos = 0 )
			{
				return rd_ = pos;
			}

			XGC_INLINE xgc_size wd_reset( xgc_size pos = 0 )
			{
				return wd_ = pos;
			}

			XGC_INLINE xgc_void reset( xgc_size rd, xgc_size wd )
			{
				rd_ = rd; wd_ = wd;
			}

			XGC_INLINE xgc_void reset()
			{
				wd_ = 0; rd_ = 0;
			}

			xgc_size putn( const xgc_void *p, xgc_size n ) throw()
			{
				xgc_size cpy = __min( space(), n );

				memcpy( wd_ptr(), p, cpy );
				wd_ += cpy;
				return cpy;
			}

			xgc_size getn( xgc_void *p, xgc_size n ) throw()
			{
				xgc_size cpy = __min( region(), n );

				memcpy( p, rd_ptr(), cpy );
				rd_ += cpy;
				return cpy;
			}

			xgc_size popn( xgc_size n ) throw()
			{
				xgc_size cpy = __min( size(), n );
				memmove( base(), base() + n, cpy );

				rd_ -= __min( rd_, cpy );
				wd_ -= __min( wd_, cpy );
				return cpy;
			}

			void stamptime()
			{
				timestamp_ = time(&timestamp_);
			}

			virtual const xgc_byte*	header()const
			{ 
				return base(); 
			}

			virtual const xgc_byte*	data()const
			{ 
				return rd_ptr(); 
			}

			virtual xgc_size size()const
			{ 
				return wd(); 
			}

			virtual xgc_size capacity()const
			{ 
				return buffer_size_ - 16; 
			}

			virtual network_t handle()const
			{ 
				return link_; 
			}

			virtual xgc_lpvoid userdata()const
			{ 
				return userdata_; 
			}

			virtual time_t timestamp()const
			{
				return timestamp_;
			}

			virtual xgc_void release()
			{ 
				delete this; 
			}

			static XGC_INLINE asio_NetworkPacket* allocate( xgc_size alloc_size )
			{
				return XGC_NEW asio_NetworkPacket( alloc_size );
			}

			static XGC_INLINE asio_NetworkPacket* allocate( xgc_size alloc_size, network_t handle, xgc_lpvoid userdata )
			{
				return XGC_NEW asio_NetworkPacket( alloc_size, handle, userdata );
			}

			static XGC_INLINE xgc_void deallocate( asio_NetworkPacket*& packet )
			{
				SAFE_DELETE( packet );
			}

		private:
			xgc_size	rd_; // 读游标
			xgc_size	wd_; // 写游标

			time_t		timestamp_;	// 时间戳
			network_t	link_;	    // 这个包的链接ID
			xgc_lpvoid	userdata_;	// 用户数据指针

			xgc_size	buffer_size_;	// 缓冲区大小
			xgc_byte*	buffer_;        // 缓冲区指针
		};

		typedef std::shared_ptr< asio_NetworkPacket > MessageBlockPtr;

	};
};
#endif // _NETWORK_PACKET_H_