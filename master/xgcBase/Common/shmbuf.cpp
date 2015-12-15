#include "shmbuf.h"
#include "logger.h"
#include "datetime.h"
#include "xsystem.h"
#include <sstream>
#include <fstream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
namespace XGC
{
	using XGC::Logger::FileLog;
	using XGC::common::datetime;
	ShmBuffer* CreateShmBuffer( xgc_lpcstr shmName, xgc_size size, xgc_lpcstr shmLogPath, xgc_bool isCreateFile /*= false*/ )
	{
		ShmBuffer* pShm = XGC_NEW ShmBuffer;
		if( pShm )
		{
			try
			{
				if( !pShm->Create( shmName, size, shmLogPath, isCreateFile ) )
				{
					SAFE_DELETE( pShm );
				}
			}
			catch( std::string& e )
			{
				FileLog( shmLogPath, "%s\n", e.c_str() );
				SAFE_DELETE( pShm );
			}
		}

		return pShm;
	}

	struct ShmBuffer::Infomation
	{
		volatile xgc_uint64 wd_pos;
		volatile xgc_uint64 rd_pos;
		volatile xgc_uint64 maxsize;
		volatile xgc_uint64 read_or_write;
	};

	ShmBuffer::ShmBuffer() 
		: mShm()
		, mShmBuffer()
		, pInfomation( xgc_nullptr )
	{

	}

	ShmBuffer::~ShmBuffer()
	{

	}

	xgc_bool ShmBuffer::Create( xgc_lpcstr shmName, xgc_size size, xgc_lpcstr shmLogPath, xgc_bool isCreateFile )
	{
		mShmName = shmName;

		mCreated = false;

		mLogPath = shmLogPath;

		/// 若不存在则创建 \///

		//////////////////////////////////////////////////////////////////////////

		SECURITY_ATTRIBUTES sa = { 0 };
		SECURITY_DESCRIPTOR sd = { 0 };
		InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );
#pragma warning(suppress: 6248)
		SetSecurityDescriptorDacl( &sd, TRUE, NULL, FALSE );

		sa.nLength = sizeof( sa );
		sa.lpSecurityDescriptor = &sd;
		sa.bInheritHandle = TRUE;

		//////////////////////////////////////////////////////////////////////////
		HANDLE hFileHandle = INVALID_HANDLE_VALUE;
		if( isCreateFile )
		{
			xgc_char szDate[128] = { 0 };
			datetime::current_time().to_string( szDate, sizeof( szDate ), "%Y_%m_%d_%H%M%S" );

			xgc_char szPath[_MAX_PATH] = { 0 };

			sprintf_s( szPath, sizeof( szPath ), "%s%s_%s.log", mLogPath.c_str(), mShmName.c_str(), szDate );
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
				SYS_ERROR( "CreateFile %s failed. err = %u", szPath, GetLastError() );
				return false;
			}
		}

		FileLog( mLogPath.c_str(), "open %s:%d failed, error=%d create it\n", mShmName.c_str(), size, GetLastError() );

		xgc_string globalShmName( "Global\\" );
		globalShmName.append( mShmName );
		hMapFile = CreateFileMappingA(
			hFileHandle,               // use paging file, else use physical file
			&sa,                                // default security 			            
			//NULL,
			PAGE_READWRITE,                     // read/write access
			0,                                  // max. object size 
			(DWORD) ( size + sizeof( Infomation ) ),  // buffer size  
			globalShmName.c_str() );        // name of mapping object

		if( hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE )
		{
			CloseHandle( hFileHandle );

			SYS_ERROR( "CreateFileMapping Error %u", GetLastError() );
			return false;
		} 
		else
		{
			SYS_INFO( "create share file mapping %s successful!", mShmName.c_str() );
		}

		mCreated = GetLastError() != ERROR_ALREADY_EXISTS;

		/// 装载内存映射 \///
		pShm = (LPTSTR)MapViewOfFile( hMapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS,                  // read/write permission
			0,
			0,
			size + sizeof( Infomation ) );

		if( xgc_nullptr == pShm )
		{
			CloseHandle( hFileHandle );
			CloseHandle( hMapFile );

			std::stringstream strs;
			strs << "MapViewOfFile.Error." << GetLastError();

			throw strs.str();
		}

		pInfomation = (Infomation*) pShm;
		if( mCreated )
		{
			/// 如果是初次创建共享内存，则设置头尾信息,并初始化 \///
			pInfomation->maxsize = size;
			pInfomation->wd_pos = pInfomation->rd_pos = 0;
			pInfomation->read_or_write = false;
			memset( (char*)pShm + sizeof( Infomation ), 0, pInfomation->maxsize );

			mShm.assign( pShm, pInfomation->maxsize + sizeof( Infomation ) );

			FileLog( mLogPath.c_str(), "init %s:%d success\n", mShmName.c_str(), pInfomation->maxsize );
		}

		mShmBuffer.assign( (char*)pShm + sizeof( Infomation ), pInfomation->maxsize );

		return true;
	}

	xgc_void ShmBuffer::Destroy()
	{
		if( xgc_nullptr != pShm )
		{
			FileLog( mLogPath.c_str(), "destroy ShmBuf %s:%d\n", mShmName.c_str(), pInfomation->maxsize );
			/// 卸载内存映射 \///
			UnmapViewOfFile( pShm );
		}
		
		CloseHandle( hMapFile );
	}

	xgc_bool ShmBuffer::Write( const xBuffer& buf, xgc_bool isOverWrite /*= false*/ )
	{
		if( !pInfomation )
		{
			return false;
		}

		if( InterlockedExchange( &pInfomation->read_or_write, pInfomation->read_or_write ) )
		{
			return false;
		}

		//char tmp[1024] = { 0 };
		//sprintf_s( tmp, "Write(%s): rd: %d, wd: %d, buflen: %d \n",
		//	mShmName.c_str(), pInfomation->rd_pos, pInfomation->wd_pos, buf.length() );
		//OutputDebugStringA( tmp );
		
		if( buf.length() > pInfomation->maxsize )
		{
			return false;
		}

		InterlockedExchange( &pInfomation->read_or_write, true );

		if( pInfomation->wd_pos + buf.length() - pInfomation->rd_pos > pInfomation->maxsize ) // 有重叠
		{
			if( !isOverWrite )
			{
				InterlockedExchange( &pInfomation->read_or_write, false );
				return false; // 等待下次写
			}
		}

		xgc_int32 real_wd = pInfomation->wd_pos % pInfomation->maxsize;
		xBuffer part1 = mShmBuffer.part< xBuffer >( real_wd, pInfomation->maxsize - real_wd );
		xgc_size cnt = part1.write( buf );
		if( cnt < buf.length() )
		{
			xBuffer part2 = mShmBuffer.part< xBuffer >( 0, real_wd );
			cnt += part2.write( buf.part< xBuffer >( cnt ) );
		}

		pInfomation->wd_pos += cnt;
		
		InterlockedExchange( &pInfomation->read_or_write, false );
		return true;
	}

	xgc_bool ShmBuffer::Read( xBuffer& buf, xgc_size &readSize )
	{
		if( !pInfomation )
		{
			return false;
		}

		if( InterlockedExchange( &pInfomation->read_or_write, pInfomation->read_or_write ) )
		{
			return false;
		}

		//char tmp[1024] = { 0 };
		//sprintf_s( tmp, "Read(%s): rd: %d, wd: %d, buflen: %d \n",
		//	mShmName.c_str(), pInfomation->rd_pos, pInfomation->wd_pos, buf.length() );
		//OutputDebugStringA( tmp );

		readSize = 0;
		if( pInfomation->rd_pos == pInfomation->wd_pos )
		{
			return false;
		}
			
		InterlockedExchange( &pInfomation->read_or_write, true );
		if( pInfomation->wd_pos - pInfomation->rd_pos > pInfomation->maxsize )
		{
			pInfomation->rd_pos = pInfomation->wd_pos; // 发生重叠， 丢掉脏数据，重新开始
			InterlockedExchange( &pInfomation->read_or_write, false );
			return false;
		}

		if( pInfomation->rd_pos < pInfomation->maxsize && pInfomation->wd_pos > pInfomation->maxsize )
		{ // 有从头开始写的数据
			pInfomation->rd_pos %= pInfomation->maxsize;
			pInfomation->wd_pos %= pInfomation->maxsize;

			xBuffer part1 = mShmBuffer.part< xBuffer >(
				pInfomation->rd_pos, pInfomation->maxsize - pInfomation->rd_pos );

			readSize = buf.write( part1 );
			
			xBuffer part2 = mShmBuffer.part< xBuffer >( 0, pInfomation->wd_pos );
			xgc_size cnt = buf.write( readSize, part2 );
			readSize += cnt;

			pInfomation->rd_pos = cnt;
		}
		else
		{
			pInfomation->rd_pos %= pInfomation->maxsize;
			pInfomation->wd_pos %= pInfomation->maxsize;

			xBuffer part = mShmBuffer.part< xBuffer >( pInfomation->rd_pos, pInfomation->wd_pos - pInfomation->rd_pos );
			xgc_size cnt = buf.write( part );
			readSize = cnt;

			pInfomation->rd_pos += cnt;
		}
		InterlockedExchange( &pInfomation->read_or_write, false );
		return true;
	}

	xgc_bool ShmBuffer::ForceWriteToFile( xgc_lpcstr pszFilename )
	{
		std::fstream fs;
		fs.open( pszFilename, std::ios_base::out | std::ios_base::trunc );
		if( fs.is_open() )
		{
			xBuffer part1 = mShmBuffer.part< xBuffer >(
				pInfomation->wd_pos % pInfomation->maxsize, 
				pInfomation->maxsize - pInfomation->wd_pos % pInfomation->maxsize );
			fs.write( (xgc_lpcstr) part1.data(), part1.length() );

			xBuffer part2 = mShmBuffer.part< xBuffer >( 0, pInfomation->wd_pos % pInfomation->maxsize );
			fs.write( (xgc_lpcstr) part2.data(), part2.length() );
		}

		fs.close();
		return true;
	}

}
