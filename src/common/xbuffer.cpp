#include "xbuffer.h"

#include "logger.h"
#include "datetime.h"
#include "xsystem.h"
#include "xutility.h"

#ifdef _LINUX
#	include <sys/mman.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#endif

namespace xgc
{
	namespace common
	{
		///
		/// \brief 默认构造
		///
		/// \author albert.xu
		/// \date 2016/09/14 13:23
		///
		shared_memory_buffer::shared_memory_buffer()
			: data_( xgc_nullptr )
			, size_( 0 )
		#ifdef _WINDOWS
			, shared_memory_( xgc_invalid_handle )
		#endif
		{
			memset( shared_file_name, 0, sizeof( shared_file_name ) );
			memset( shared_file_path, 0, sizeof( shared_file_path ) );
		}

		shared_memory_buffer::shared_memory_buffer( xgc_lpcstr shared_memory_name, xgc_size size )
			: shared_memory_buffer()
		{
			create( shared_memory_name, size, xgc_nullptr );
		}

		///
		/// \brief 拷贝构造
		///
		/// \author albert.xu
		/// \date 2015/12/18 16:31
		///
		shared_memory_buffer::shared_memory_buffer( const shared_memory_buffer& buffer )
		{
			create( buffer.shared_file_name, buffer.size_, buffer.shared_file_path );
		}

		///
		/// \brief 右值拷贝构造
		///
		/// \author albert.xu
		/// \date 2015/12/18 16:31
		///
		shared_memory_buffer::shared_memory_buffer( shared_memory_buffer&& buffer )
			: data_( buffer.data_ )
			, size_( buffer.size_ )
			#if defined _WINDOWS
			, shared_memory_( buffer.shared_memory_ )
			#endif
		{
			strcpy_s( shared_file_name, buffer.shared_file_name );
			strcpy_s( shared_file_path, buffer.shared_file_path );

			buffer.data_ = xgc_nullptr;
			buffer.size_ = 0;

			buffer.shared_file_name[0] = 0;
			buffer.shared_file_path[0] = 0;

			#if defined _WINDOWS
			buffer.shared_memory_ = xgc_invalid_handle;
			#endif

		}

		///
		/// \brief 析构函数
		///
		/// \author albert.xu
		/// \date 2015/12/18 16:34
		///
		shared_memory_buffer::~shared_memory_buffer()
		{
			destroy();
		}

		///
		/// \brief 创建共享内存
		///
		/// \author albert.xu
		/// \date 2015/12/18 16:29
		///

		#if defined _WINDOWS
		xgc_long shared_memory_buffer::create( xgc_lpcstr shared_memory_name, xgc_size size, xgc_lpcstr file_path )
		{
			strcpy_s( shared_file_name, shared_memory_name );
			strcpy_s( shared_file_path, file_path );

			size_ = size;


			xgc_handle shared_file_h = INVALID_HANDLE_VALUE;
			SECURITY_ATTRIBUTES sa = { 0 };
			SECURITY_DESCRIPTOR sd = { 0 };
			InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );
			#pragma warning(suppress: 6248)
			SetSecurityDescriptorDacl( &sd, TRUE, NULL, FALSE );
			sa.nLength = sizeof( sa );
			sa.lpSecurityDescriptor = &sd;
			sa.bInheritHandle = TRUE;

			if( file_path && file_path[0] )
			{
				xgc_char sz_path[XGC_MAX_PATH] = { 0 };
				get_normal_path( sz_path, "%s/%s", file_path, shared_memory_name );
				shared_file_h = CreateFileA(
					sz_path,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_ALWAYS,
					NULL,
					NULL );

				if( shared_file_h == xgc_invalid_handle )
				{
					SYS_ERROR( "CreateFile %s failed. err = %u", file_path, GetLastError() );
					return -1;
				}
			}

			shared_memory_ = CreateFileMappingA(
				shared_file_h,									// use paging file, else use physical file
				&sa,											// default security 
				PAGE_READWRITE,									// read/write access
				0,												// max. object size 
				(DWORD) ( size ),									// buffer size  
				shared_memory_name );							// name of mapping object

			if( shared_memory_ == NULL || shared_memory_ == INVALID_HANDLE_VALUE )
			{
				XGC_ASSERT_MESSAGE( false, "CreateFileMappingA error %u", GetLastError() );
				CloseHandle( shared_file_h );
				return -1;
			}

			xgc_bool is_created = GetLastError() != ERROR_ALREADY_EXISTS;

			/// 装载内存映射 \///
			data_ = (xgc_lpstr) MapViewOfFile( shared_memory_,   // handle to map object
											   FILE_MAP_ALL_ACCESS,                  // read/write permission
											   0,
											   0,
											   size );

			if( xgc_nullptr == data_ )
			{
				XGC_ASSERT_MESSAGE( false, "MapViewOfFile error %u", GetLastError() );

				CloseHandle( shared_file_h );
				shared_file_h = xgc_invalid_handle;

				CloseHandle( shared_memory_ );
				shared_memory_ = xgc_invalid_handle;

				return -1;
			}

			CloseHandle( shared_file_h );
			shared_file_h = xgc_invalid_handle;

			return is_created ? 1L : 0L;
		}

		///
		/// \brief 销毁共享内存对象
		///
		/// \author albert.xu
		/// \date 2015/12/18 16:35
		///

		xgc_void shared_memory_buffer::destroy()
		{
			UnmapViewOfFile( data_ );

			if( shared_memory_ != xgc_invalid_handle )
				CloseHandle( shared_memory_ );

			shared_memory_ = xgc_invalid_handle;

			data_ = xgc_nullptr;
			size_ = 0;

			shared_file_name[0] = 0;
			shared_file_path[0] = 0;
		}
		#elif defined _LINUX
		xgc_long shared_memory_buffer::create( xgc_lpcstr shared_memory_name, xgc_size size, xgc_lpcstr file_path )
		{
			xgc_bool already_exists = true;

			strcpy_s( shared_file_name, shared_memory_name );
			strcpy_s( shared_file_path, file_path );

			size_ = size;

			int fd = -1;
			//创建或者打开一个共享内存
			if( file_path && file_path[0] )
			{
				// using file mapping
				xgc_char sz_file[XGC_MAX_PATH] = { 0 };
				get_normal_path( sz_file, "%s/%s", file_path, shared_memory_name );
				fd = open( sz_file, O_RDWR, 0 );
				if( -1 == fd  )
				{
					already_exists = false;
					fd = open(sz_file, O_RDWR|O_CREAT, 00666 );

					if( -1 == fd )
					{
						SYS_ERROR( "shm file open failed: ");
						return -1;
					}
					
					//调整确定文件共享内存的空间
					if( -1 == ftruncate(fd, size) )
					{
						SYS_ERROR("ftruncate faile: ");
						close( fd );
						return -1;
					}
				}
			}
			else
			{
				// using system shared memory, file in /dev/shm/shm.%shared_memory_name%
				fd = shm_open(shared_memory_name, O_RDWR, 0);
				if( -1 == fd  )
				{
					already_exists = false;
					fd = shm_open(shared_memory_name, O_RDWR|O_CREAT, 00666 );

					if( -1 == fd )
					{
						SYS_ERROR( "shm open failed: ");
						return -1;
					}
					
					//调整确定文件共享内存的空间
					if( -1 == ftruncate(fd, size) )
					{
						SYS_ERROR("ftruncate faile: ");
						close( fd );
						shm_unlink( shared_memory_name );
						return -1;
					}
				}
			}

			//映射目标文件的存储区
			data_ = (xgc_lpstr)mmap( xgc_nullptr, size, PROT_WRITE, MAP_SHARED, fd, SEEK_SET );
			if( xgc_nullptr == data_ )
			{
				SYS_ERROR("mmap add_r failed: ");
				shm_unlink( shared_memory_name );
				return -1;
			}
		
			close( fd );
			return already_exists ? 1 : 0;
		}

		///
		/// \brief 销毁共享内存对
		///
		/// \author albert.xu
		/// \date 2015/12/18 16:35
		///
		xgc_void shared_memory_buffer::destroy()
		{
			munmap( (void*)data_, size_ );
			if( shared_file_path[0] == 0 )
			{
				// using system shared memory mapping
				shm_unlink( shared_file_name );
			}

			data_ = xgc_nullptr;
			size_ = 0;
			shared_file_name[0] = 0;
			shared_file_path[0] = 0;
		}
		#endif // _WINDOWS
	}
}