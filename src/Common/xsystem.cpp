#include "xsystem.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <time.h>

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

namespace xgc
{
	static xgc_char gModuleName[_MAX_FNAME] = { 0 };
	static xgc_char gModulePath[_MAX_PATH] = { 0 };

	xgc_lpcstr GetModuleName( HMODULE hModule/* = xgc_nullptr*/ )
	{
		if( gModuleName[0] == 0 )
		{
			xgc_size cpyLen = GetModuleFileNameA( hModule, gModuleName, sizeof( gModuleName ) );
			if( cpyLen == 0 )
				return gModuleName;

			xgc_lpstr p = strrchr( gModuleName, '\\' );
			if( p )
				strcpy_s( gModuleName, p + 1 );
		}
		return gModuleName;
	}

	xgc_lpcstr GetModulePath( HMODULE hModule/* = xgc_nullptr*/ )
	{
		if( gModulePath[0] == 0 )
		{
			DWORD dwRet = GetModuleFileNameA( xgc_nullptr, gModulePath, sizeof( gModulePath ) );
			XGC_ASSERT_RETURN( dwRet, "" );
			xgc_lpstr last = xgc_nullptr;
			xgc_lpstr found = strchr( gModulePath, '\\' );
			while( found )
			{
				last = found++;
				found = strchr( found, '\\' );
			}

			if( last ) last[1] = 0;
		}
		return gModulePath;
	}

	xgc_lpcstr GetNormalPath( xgc_lpstr szOut, xgc_size nSize, xgc_lpcstr lpRelativePath, ... )
	{
		int cpy1 = 0;
		xgc_lpcstr driver = strchr( lpRelativePath, ':' );
		if( xgc_nullptr == driver )
		{
			cpy1 = sprintf_s( szOut, nSize, "%s", GetModulePath() );
			XGC_ASSERT_RETURN( cpy1 >= 0, xgc_nullptr );
		}
		else if( driver - lpRelativePath != 1 )
		{
			return xgc_nullptr;
		}
		else if( driver[1] != '\\' && driver[1] != '/' )
		{
			return xgc_nullptr;
		}

		va_list args;
		va_start( args, lpRelativePath );
		int cpy2 = vsprintf_s( szOut + cpy1, nSize - cpy1, lpRelativePath, args );
		va_end( args );

		if( cpy2 < 0 )
			return xgc_nullptr;

		if( cpy2 == sizeof(szOut) -cpy1 )
			return xgc_nullptr;

		return _fullpath( szOut, szOut, nSize );
	}

	xgc_bool GetProcessMemoryUsage( xgc_handle h, xgc_uint64 *pnPMem, xgc_uint64 *pnVMem )
	{
		XGC_ASSERT_RETURN( pnPMem != pnVMem, false );
		if( pnPMem == xgc_nullptr && pnVMem == xgc_nullptr )
			return false;

		PROCESS_MEMORY_COUNTERS MEM;
		if( 0 == GetProcessMemoryInfo( (HANDLE)h, &MEM, sizeof( MEM ) ) )
			return false;

		if( pnPMem )
			*pnPMem = MEM.WorkingSetSize;

		if( pnVMem )
			*pnVMem = MEM.PagefileUsage;

		return true;
	}

	xgc_bool GetSystemMemoryUsage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint32 *pnLoadMem )
	{
		XGC_ASSERT_RETURN( pnUsageMem != pnTotalMem, false );
		if( pnUsageMem == xgc_nullptr && pnTotalMem == xgc_nullptr )
			return false;

		MEMORYSTATUSEX MEM;
		MEM.dwLength = sizeof( MEM );
		if( 0 == GlobalMemoryStatusEx( &MEM ) )
			return false;

		if( pnUsageMem )
			*pnUsageMem = MEM.ullTotalPhys - MEM.ullAvailPhys;

		if( pnTotalMem )
			*pnTotalMem = MEM.ullTotalPhys;

		if( pnLoadMem )
			*pnLoadMem = MEM.dwMemoryLoad;

		return true;
	}

	/// 获得CPU的核数
	static int get_processor_number()
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return (int)info.dwNumberOfProcessors;
	}

	static int32_t processor_count = get_processor_number();

	static int64_t last_time_ = 0;
	static int64_t last_system_time_ = 0;

	/// 时间转换
	static uint64_t file_time_2_utc(const FILETIME* ftime)
	{
		LARGE_INTEGER li;

		assert(ftime);
		li.LowPart = ftime->dwLowDateTime;
		li.HighPart = ftime->dwHighDateTime;
		return li.QuadPart;
	}

	xgc_int32 GetProcessCpuUsage( xgc_handle h )
	{
		FILETIME now;
		FILETIME creation_time;
		FILETIME exit_time;
		FILETIME kernel_time;
		FILETIME user_time;
		int64_t system_time;
		int64_t time;
		int64_t system_time_delta;
		int64_t time_delta;

		int cpu = -1;

		GetSystemTimeAsFileTime(&now);

		if (!GetProcessTimes((HANDLE)h, &creation_time, &exit_time,
							  &kernel_time, &user_time))
		{
			// We don't assert here because in some cases (such as in the Task  Manager)
			// we may call this function on a process that has just exited but  we have
			// not yet received the notification.
			return -1;
		}

		system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time)) / processor_count;
		time = file_time_2_utc(&now);

		if ((last_system_time_ == 0) || (last_time_ == 0))
		{
			// First call, just set the last values.
			last_system_time_ = system_time;
			last_time_ = time;
			return -1;
		}

		system_time_delta = system_time - last_system_time_;
		time_delta = time - last_time_;

		assert(time_delta != 0);

		if (time_delta == 0)
			return -1;

		// We add time_delta / 2 so the result is rounded.
		cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);
		last_system_time_ = system_time;
		last_time_ = time;
		return cpu;
	}

	static int64_t global_system_time_ = 0;
	static int64_t global_kernel_time_ = 0;
	static int64_t global_user_time_ = 0;
	static int64_t global_idle_time_ = 0;

	xgc_int32 GetSystemCpuUsage()
	{
		FILETIME now;
		FILETIME idle_time;
		FILETIME kernel_time;
		FILETIME user_time;

		double cpu = -1;

		GetSystemTimeAsFileTime(&now);

		if (!GetSystemTimes(&idle_time, &kernel_time, &user_time))
			return -1;

		int64_t utc_kernel_time = file_time_2_utc( &kernel_time );
		int64_t utc_user_time   = file_time_2_utc( &user_time );
		int64_t utc_idle_time   = file_time_2_utc( &idle_time );

		int64_t delta_kernel_time = utc_kernel_time - global_kernel_time_;
		int64_t delta_user_time   = utc_user_time - global_user_time_;
		int64_t delta_idle_time   = utc_idle_time - global_idle_time_;

		// We add time_delta / 2 so the result is rounded.
		cpu = ( (double)(delta_kernel_time + delta_user_time - delta_idle_time) * 100.0f / (double)(delta_kernel_time +delta_user_time) ) + 0.5f;
		global_kernel_time_ = utc_kernel_time;
		global_user_time_   = utc_user_time;
		global_idle_time_   = utc_idle_time;
		return (int)cpu;
	}

	int get_io_bytes(uint64_t* read_bytes, uint64_t* write_bytes)
	{
		IO_COUNTERS io_counter;
		if(GetProcessIoCounters(GetCurrentProcess(), &io_counter))
		{
			if(read_bytes) *read_bytes = io_counter.ReadTransferCount;
			if(write_bytes) *write_bytes = io_counter.WriteTransferCount;
			return 0;
		}
		return -1;
	}

	std::mt19937& GetRandomDriver()
	{
		static std::default_random_engine mt( (xgc_int32) time( xgc_nullptr ) );
		return mt;
	}

	xgc_bool privilege( xgc_lpcstr prililege_name, xgc_bool enable /*= true */ )
	{
		bool bSuccess = FALSE;
		HANDLE hToken = NULL;

		if( !OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken ) )
			return FALSE;


		TOKEN_PRIVILEGES tp = { 1 };

		if( LookupPrivilegeValue( NULL, prililege_name, &tp.Privileges[0].Luid ) )
		{
			tp.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;

			AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof( tp ), NULL, NULL );
			bSuccess = ( GetLastError() == ERROR_SUCCESS );
		}

		CloseHandle( hToken );
		return bSuccess;
	}

}