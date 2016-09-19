#include "xbuffer.h"
#include "datetime.h"
#include "xutility.h"

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
			: shared_memory_( xgc_invalid_handle )
			, shared_file_( xgc_invalid_handle )
			, data_( xgc_nullptr )
		{
		}

		shared_memory_buffer::shared_memory_buffer( xgc_lpcstr shared_memory_name, xgc_size size )
			: shared_memory_buffer()
		{
			create( shared_memory_name, size, xgc_nullptr );
		}

		shared_memory_buffer::shared_memory_buffer( const shared_memory_buffer& buffer ) 
			: shared_memory_( INVALID_HANDLE_VALUE )
		{
			assert( DuplicateHandle( GetCurrentProcess(), buffer.shared_memory_,
									 GetCurrentProcess(), &shared_memory_, FILE_MAP_ALL_ACCESS, false, 0 ) );
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
		xgc_long shared_memory_buffer::create( xgc_lpcstr shared_memory_name, xgc_size size, xgc_lpcstr file_path )
		{
			xgc_handle hFileHandle = INVALID_HANDLE_VALUE;
			SECURITY_ATTRIBUTES sa = { 0 };
			SECURITY_DESCRIPTOR sd = { 0 };
			InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );
			#pragma warning(suppress: 6248)
			SetSecurityDescriptorDacl( &sd, TRUE, NULL, FALSE );
			sa.nLength = sizeof( sa );
			sa.lpSecurityDescriptor = &sd;
			sa.bInheritHandle = TRUE;

			if( file_path )
			{
				xgc_char szDate[128] = { 0 };
				datetime::now( szDate, "%Y_%m_%d_%H%M%S" );

				xgc_char szPath[_MAX_PATH] = { 0 };
				sprintf_s( szPath, sizeof( szPath ), "%s%s_%s.log", file_path, shared_memory_name, szDate );
				hFileHandle = CreateFileA(
					szPath,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_ALWAYS,
					NULL,
					NULL );

				if( hFileHandle == INVALID_HANDLE_VALUE )
				{
					SYS_ERROR( "CreateFile %s failed. err = %u", file_path, GetLastError() );
					return -1;
				}
			}

			shared_memory_ = CreateFileMappingA(
				shared_file_,									// use paging file, else use physical file
				&sa,											// default security 
				PAGE_READWRITE,									// read/write access
				0,												// max. object size 
				(DWORD) ( size ),									// buffer size  
				shared_memory_name );							// name of mapping object

			if( shared_memory_ == NULL || shared_memory_ == INVALID_HANDLE_VALUE )
			{
				XGC_ASSERT_MESSAGE( false, "CreateFileMappingA error %u", GetLastError() );
				CloseHandle( hFileHandle );
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

				CloseHandle( shared_file_ );
				shared_file_ = xgc_invalid_handle;

				CloseHandle( shared_memory_ );
				shared_memory_ = xgc_invalid_handle;

				return -1;
			}

			*(xgc_size*) data_ = size - sizeof( xgc_size );

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

			if( shared_file_ != xgc_invalid_handle )
				CloseHandle( shared_file_ );

			shared_file_ = xgc_invalid_handle;

			if( shared_memory_ != xgc_invalid_handle )
				CloseHandle( shared_memory_ );

			shared_memory_ = xgc_invalid_handle;
		}
	}
}