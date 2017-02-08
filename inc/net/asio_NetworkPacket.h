#ifndef _NETWORK_PACKET_H_
#define _NETWORK_PACKET_H_

namespace xgc
{
	namespace net
	{
		const xgc_size __packet_buffer_guard_size = sizeof(xgc_uint64);
		class asio_NetworkPacket : public INetPacket
		{
		friend class asio_Socket;
		private:
			asio_NetworkPacket( xgc_size capacity )
				: rd_(0)
				, wd_(0)
				, link_(INVALID_NETWORK_HANDLE)
				, timestamp_(0)
				, buffer_size_( capacity + __packet_buffer_guard_size * 2 )
			{
				*(xgc_uint64*) (buffer_) = 0xebebebebebebebebULL;
				*(xgc_uint64*) (buffer_ + capacity + 8) = 0xededededededededULL;
				stamptime();
			}

			asio_NetworkPacket( xgc_size capacity, network_t lnk )
				: rd_(0)
				, wd_(0)
				, link_(lnk)
				, timestamp_(0)
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
				
				free( (void*)buffer_ );
			}

		private:
			XGC_INLINE xgc_size rd()const{ return rd_; }
			XGC_INLINE xgc_size wd()const{ return wd_; }

			XGC_INLINE xgc_lpcstr rd_ptr()const{ return base() + rd(); }
			XGC_INLINE xgc_lpcstr wd_ptr()const{ return base() + wd();	}

			XGC_INLINE xgc_lpstr rd_ptr(){ return base() + rd(); }
			XGC_INLINE xgc_lpstr wd_ptr(){ return base() + wd(); }

		public:
			enum mode{ swd = 1, srd = 2, srw = 3 };

			///
			/// \brief 获取缓冲区首地址
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:48
			///
			XGC_INLINE xgc_lpstr base()
			{
				return buffer_ + __packet_buffer_guard_size;
			}

			///
			/// \brief 获取缓冲区首地址 const
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:48
			///
			XGC_INLINE xgc_lpcstr base()const
			{
				return buffer_ + __packet_buffer_guard_size;
			}

			///
			/// \brief 获取剩余写空间
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:48
			///
			XGC_INLINE xgc_size space()const
			{
				return buffer_size_ - __packet_buffer_guard_size - wd_;
			}

			///
			/// \brief 获取剩余读空间
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:48
			///
			XGC_INLINE xgc_size region()const
			{
				return wd_ - rd_;
			}

			///
			/// \brief 增加读指针偏移量
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:49
			///
			XGC_INLINE xgc_size rd_plus( xgc_size pos )
			{
				return rd_ += pos;
			}

			///
			/// \brief 增加写指针偏移量
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:49
			///
			XGC_INLINE xgc_size wd_plus( xgc_size pos )
			{
				return wd_ += pos;
			}

			///
			/// \brief 重置读指针位置
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:49
			///
			XGC_INLINE xgc_size rd_reset( xgc_size pos = 0 )
			{
				return rd_ = pos;
			}

			///
			/// \brief 重置写指针位置
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:49
			///
			XGC_INLINE xgc_size wd_reset( xgc_size pos = 0 )
			{
				return wd_ = pos;
			}

			///
			/// \brief 重置读写指针位置
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:49
			///
			XGC_INLINE xgc_void reset( xgc_size rd, xgc_size wd )
			{
				rd_ = rd; wd_ = wd;
			}

			///
			/// \brief 重置读写指针位置
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:50
			///
			XGC_INLINE xgc_void reset( xgc_size val = 0 )
			{
				wd_ = val; rd_ = val;
			}

			///
			/// \brief 写数据到缓冲区
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:50
			///
			xgc_size putn( const xgc_void *p, xgc_size n ) throw()
			{
				xgc_size cpy = XGC_MIN( space(), n );

				memcpy( wd_ptr(), p, cpy );
				wd_ += cpy;
				return cpy;
			}

			///
			/// \brief 读数据从缓冲区
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:50
			///
			xgc_size getn( xgc_void *p, xgc_size n ) throw()
			{
				xgc_size cpy = XGC_MIN( region(), n );

				memcpy( p, rd_ptr(), cpy );
				rd_ += cpy;
				return cpy;
			}

			///
			/// \brief 封包结束
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:51
			///

			XGC_INLINE xgc_void done( network_t h, xgc_lpvoid u )
			{ 
				link_ = h; 
				stamptime();
			}

			///
			/// \brief 生成时间戳
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:51
			///
			XGC_INLINE void stamptime()
			{
				timestamp_ = time(&timestamp_);
			}

			///
			/// \brief 获取缓冲区首地址
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:51
			///
			virtual xgc_lpcstr header()const override
			{ 
				return base(); 
			}

			///
			/// \brief 数据包长度
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual xgc_size length()const override
			{
				return wd();
			}

			///
			/// \brief 获取缓冲区数据地址
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:51
			///
			virtual xgc_lpcstr data()const override
			{ 
				return rd_ptr(); 
			}

			///
			/// \brief 获取缓冲区长度
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:52
			///
			virtual xgc_size size()const override
			{ 
				return region(); 
			}

			///
			/// \brief 获取缓冲区容量
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:52
			///
			virtual xgc_size capacity()const override
			{ 
				return buffer_size_ - __packet_buffer_guard_size * 2; 
			}

			///
			/// \brief 获取缓冲区关联的网络句柄
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:52
			///
			virtual network_t handle()const override
			{ 
				return link_; 
			}

			///
			/// \brief 获取缓冲区时间戳
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:53
			///
			virtual time_t timestamp()const override
			{
				return timestamp_;
			}

			///
			/// \brief 释放缓冲区
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:53
			///
			virtual xgc_void freedom() throw() override
			{ 
				free( this );
			}

			///
			/// \brief 分配一个数据包
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:53
			///
			static XGC_INLINE asio_NetworkPacket* allocate( xgc_size alloc_size )
			{
				xgc_lpvoid ptr = malloc( sizeof( asio_NetworkPacket ) + alloc_size + __packet_buffer_guard_size * 2 );

				new (ptr) asio_NetworkPacket( alloc_size );

				return (asio_NetworkPacket*)ptr;
			}

			///
			/// \brief 分配一个网络句柄关联的数据包
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:54
			///
			static XGC_INLINE asio_NetworkPacket* allocate( xgc_size alloc_size, network_t handle )
			{
				xgc_lpvoid ptr = malloc( sizeof( asio_NetworkPacket ) + alloc_size + __packet_buffer_guard_size * 2 );

				new (ptr) asio_NetworkPacket( alloc_size, handle );

				return (asio_NetworkPacket*)ptr;
			}

			///
			/// \brief 释放数据包
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:54
			///
			static XGC_INLINE xgc_void deallocate( asio_NetworkPacket*& packet )
			{
				if( packet )
				{
					packet->freedom();
					packet = xgc_nullptr;
				}
			}

		private:
			/// 读游标
			xgc_size	rd_;
			/// 写游标
			xgc_size	wd_;
			/// 时间戳
			time_t		timestamp_;
			/// 这个包关联的链接ID
			network_t	link_;	  
			/// 缓冲区大小
			xgc_size	buffer_size_;
			/// 缓冲区指针
			xgc_char	buffer_[1];
		};
	};
};
#endif // _NETWORK_PACKET_H_