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
		public:
			enum mode{ swd = 1, srd = 2, srw = 3 };

			asio_NetworkPacket( xgc_size capacity )
				: rd_(0)
				, wd_(0)
				, link_(INVALID_NETWORK_HANDLE)
				, timestamp_(0)
				, buffer_( (xgc_lpstr)malloc( capacity + __packet_buffer_guard_size * 2 ) )
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
				, buffer_( (xgc_lpstr)malloc( capacity + __packet_buffer_guard_size * 2 ) )
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

			XGC_INLINE xgc_void done( network_t h, xgc_lpvoid u )
			{ 
				link_ = h; 
				stamptime();
			}

		private:
			XGC_INLINE xgc_size rd()const{ return rd_; }
			XGC_INLINE xgc_size wd()const{ return wd_; }

			XGC_INLINE xgc_lpcstr rd_ptr()const{ return base() + rd(); }
			XGC_INLINE xgc_lpcstr wd_ptr()const{ return base() + wd();	}

			XGC_INLINE xgc_lpstr rd_ptr(){ return base() + rd(); }
			XGC_INLINE xgc_lpstr wd_ptr(){ return base() + wd(); }

		public:
			///
			/// \brief ��ȡ�������׵�ַ
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:48
			///
			XGC_INLINE xgc_lpstr base()
			{
				return buffer_ + __packet_buffer_guard_size;
			}

			///
			/// \brief ��ȡ�������׵�ַ const
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:48
			///
			XGC_INLINE xgc_lpcstr base()const
			{
				return buffer_+ __packet_buffer_guard_size;
			}

			///
			/// \brief ��ȡʣ��д�ռ�
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:48
			///
			XGC_INLINE xgc_size space()const
			{
				return buffer_size_ - wd_;
			}

			///
			/// \brief ��ȡʣ����ռ�
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:48
			///
			XGC_INLINE xgc_size region()const
			{
				return wd_ - rd_;
			}

			///
			/// \brief ���Ӷ�ָ��ƫ����
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:49
			///
			XGC_INLINE xgc_size rd_plus( xgc_size pos )
			{
				return rd_ += pos;
			}

			///
			/// \brief ����дָ��ƫ����
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:49
			///
			XGC_INLINE xgc_size wd_plus( xgc_size pos )
			{
				return wd_ += pos;
			}

			///
			/// \brief ���ö�ָ��λ��
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:49
			///
			XGC_INLINE xgc_size rd_reset( xgc_size pos = 0 )
			{
				return rd_ = pos;
			}

			///
			/// \brief ����дָ��λ��
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:49
			///
			XGC_INLINE xgc_size wd_reset( xgc_size pos = 0 )
			{
				return wd_ = pos;
			}

			///
			/// \brief ���ö�дָ��λ��
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:49
			///
			XGC_INLINE xgc_void reset( xgc_size rd, xgc_size wd )
			{
				rd_ = rd; wd_ = wd;
			}

			///
			/// \brief ���ö�дָ��λ��
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:50
			///
			XGC_INLINE xgc_void reset( xgc_size val = 0 )
			{
				wd_ = val; rd_ = val;
			}

			///
			/// \brief д���ݵ�������
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:50
			///
			xgc_size putn( const xgc_void *p, xgc_size n ) throw()
			{
				xgc_size cpy = __min( space(), n );

				memcpy( wd_ptr(), p, cpy );
				wd_ += cpy;
				return cpy;
			}

			///
			/// \brief �����ݴӻ�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:50
			///
			xgc_size getn( xgc_void *p, xgc_size n ) throw()
			{
				xgc_size cpy = __min( region(), n );

				memcpy( p, rd_ptr(), cpy );
				rd_ += cpy;
				return cpy;
			}

			///
			/// \brief �ӻ�����ǰ���Ƴ���������
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:50
			///
			xgc_size popn( xgc_size n ) throw()
			{
				xgc_size cpy = __min( region(), n );
				memmove( base(), base() + n, region() - cpy );

				rd_ -= __min( rd_, cpy );
				wd_ -= __min( wd_, cpy );
				return cpy;
			}

			///
			/// \brief ����ʱ���
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:51
			///
			void stamptime()
			{
				timestamp_ = time(&timestamp_);
			}

			///
			/// \brief ��ȡ�������׵�ַ
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:51
			///
			virtual xgc_lpcstr header()const override
			{ 
				return base(); 
			}

			///
			/// \brief ���ݰ�����
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual xgc_size length()const override
			{
				return wd();
			}

			///
			/// \brief ��ȡ���������ݵ�ַ
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:51
			///
			virtual xgc_lpcstr data()const override
			{ 
				return rd_ptr(); 
			}

			///
			/// \brief ��ȡ����������
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:52
			///
			virtual xgc_size size()const override
			{ 
				return region(); 
			}

			///
			/// \brief ��ȡ����������
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:52
			///
			virtual xgc_size capacity()const override
			{ 
				return buffer_size_ - __packet_buffer_guard_size * 2; 
			}

			///
			/// \brief ��ȡ������������������
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:52
			///
			virtual network_t handle()const override
			{ 
				return link_; 
			}

			///
			/// \brief ��ȡ�������������û�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:52
			///
			virtual xgc_lpvoid userdata()const override
			{ 
				xgc_lpvoid pUserdata = xgc_nullptr;

				// ��������ʲô�õķ���
				//auto pHandle = getHandleManager().FetchHandle( link_ );
				//if( pHandle )
				//{
				//	pUserdata = pHandle->GetUserdata();
				//	getHandleManager().FreeHandle( pHandle );
				//}

				return pUserdata;
			}

			///
			/// \brief ��ȡ������ʱ���
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:53
			///
			virtual time_t timestamp()const override
			{
				return timestamp_;
			}

			///
			/// \brief �ͷŻ�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:53
			///
			virtual xgc_void release() throw() override
			{ 
				delete this; 
			}

			///
			/// \brief ����һ�����ݰ�
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:53
			///
			static XGC_INLINE asio_NetworkPacket* allocate( xgc_size alloc_size )
			{
				return XGC_NEW asio_NetworkPacket( alloc_size );
			}

			///
			/// \brief ����һ�����������������ݰ�
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:54
			///
			static XGC_INLINE asio_NetworkPacket* allocate( xgc_size alloc_size, network_t handle )
			{
				return XGC_NEW asio_NetworkPacket( alloc_size, handle );
			}

			///
			/// \brief �ͷ����ݰ�
			///
			/// \author albert.xu
			/// \date 2016/02/17 15:54
			///
			static XGC_INLINE xgc_void deallocate( asio_NetworkPacket*& packet )
			{
				if( packet )
				{
					packet->release();
					packet = xgc_nullptr;
				}
			}

		private:
			/// ���α�
			xgc_size	rd_;
			/// д�α�
			xgc_size	wd_;
			/// ʱ���
			time_t		timestamp_;
			/// ���������������ID
			network_t	link_;	  
			/// ��������С
			xgc_size	buffer_size_;
			/// ������ָ��
			xgc_lpstr	buffer_;
		};
	};
};
#endif // _NETWORK_PACKET_H_