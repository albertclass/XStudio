#include "xsystem.h"
#include <stdio.h>
#include <time.h>

#ifdef _WINDOWS
#	include <psapi.h>
#	pragma comment(lib, "psapi.lib")
#endif

namespace xgc
{
	static xgc_char g_module_name[_MAX_FNAME] = { 0 };
	static xgc_char g_module_path[_MAX_PATH] = { 0 };

	xgc_lpcstr get_module_name( xgc_bool rebuild /*= false*/ )
	{
		if( g_module_name[0] == 0 || rebuild )
		{
			#if defined(_WINDOWS)
			xgc_size cpyLen = GetModuleFileNameA( NULL, g_module_name, sizeof( g_module_name ) );
			#elif defined(_LINUX)
			xgc_size cpyLen = readlink("/proc/self/exe", g_module_name, sizeof( g_module_name ) );
			#else
			static_assert( false, "unknowe operator system." );
			#endif

			if( cpyLen == 0 )
				return g_module_name;

			xgc_lpstr p = strrchr( g_module_name, '\\' );
			if( p )
				strcpy_s( g_module_name, p + 1 );
		}
		return g_module_name;
	}

	xgc_lpcstr get_module_path( xgc_bool rebuild /*= false*/ )
	{
		if( g_module_path[0] == 0 || rebuild )
		{
			#if defined(_WINDOWS)
			xgc_size cpyLen = GetModuleFileNameA( NULL, g_module_name, sizeof( g_module_name ) );
			#elif defined(_LINUX)
			xgc_size cpyLen = readlink("/proc/self/exe", g_module_name, sizeof( g_module_name ) );
			#else
			static_assert( false, "unknowe operator system." );
			#endif

			XGC_ASSERT_RETURN( cpyLen, xgc_nullptr );
			xgc_lpstr last = xgc_nullptr;
			xgc_lpstr found = strchr( g_module_path, '\\' );
			while( found )
			{
				last = found++;
				found = strchr( found, '\\' );
			}

			if( last ) last[1] = 0;
		}

		return g_module_path;
	}

	xgc_lpcstr get_normal_path( xgc_lpstr szOut, xgc_size nSize, xgc_lpcstr lpRelativePath, ... )
	{
		int cpy1 = 0;
		xgc_lpcstr driver = strchr( lpRelativePath, ':' );
		if( xgc_nullptr == driver )
		{
			cpy1 = sprintf_s( szOut, nSize, "%s", get_module_path() );
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

	xgc_bool get_process_memory_usage( xgc_handle h, xgc_uint64 *pnPMem, xgc_uint64 *pnVMem )
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

	xgc_bool get_system_memory_usage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint32 *pnLoadMem )
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

	#if defined(_WINDOWS)
	/// 获得CPU的核数
	static int32_t get_processor_number()
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return (int32_t)info.dwNumberOfProcessors;
	}

	static int32_t processor_count = get_processor_number();

	static int64_t last_time_ = 0;
	static int64_t last_system_time_ = 0;

	/// 时间转换
	static uint64_t file_time_2_utc(const FILETIME* ftime)
	{
		LARGE_INTEGER li;

		XGC_ASSERT_RETURN(ftime,0);
		li.LowPart 	= ftime->dwLowDateTime;
		li.HighPart = ftime->dwHighDateTime;
		return li.QuadPart;
	}
	#endif

	xgc_int32 get_process_cpu_usage( xgc_handle h )
	{
		#if defined(_WINDOWS)
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

		XGC_ASSERT_RETURN(time_delta != 0, -1);

		// We add time_delta / 2 so the result is rounded.
		cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);
		last_system_time_ = system_time;
		last_time_ = time;
		return cpu;
		#else
		return 0;
		#endif
	}

	xgc_int32 get_system_cpu_usage()
	{
		#if defined(_WINDOWS)
		static int64_t global_system_time_ = 0;
		static int64_t global_kernel_time_ = 0;
		static int64_t global_user_time_ = 0;
		static int64_t global_idle_time_ = 0;

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
		#else
		return 0;
		#endif
	}

	xgc_bool privilege( xgc_lpcstr prililege_name, xgc_bool enable /*= true */ )
	{
		bool bSuccess = false;

		#ifdef _WINDOWS
		HANDLE hToken = NULL;

		if( !OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken ) )
			return false;


		TOKEN_PRIVILEGES tp = { 1 };

		if( LookupPrivilegeValue( NULL, prililege_name, &tp.Privileges[0].Luid ) )
		{
			tp.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;

			AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof( tp ), NULL, NULL );
			bSuccess = ( GetLastError() == ERROR_SUCCESS );
		}

		CloseHandle( hToken );
		#endif
		
		return bSuccess;
	}

}