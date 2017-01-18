#include "xsystem.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef _WINDOWS
#	include <direct.h>
#	include <psapi.h>
#	include <pdh.h>
#	pragma comment(lib, "psapi.lib")
#	pragma comment(lib, "pdh.lib")
#endif

#ifdef _LINUX
#	include "sys/types.h"
#	include "sys/stat.h"
#	include "sys/sysinfo.h"
#	include "sys/times.h"
#	include "sys/vtimes.h"
#endif

#ifdef _WINDOWS
#	define SLASH '\\'
#endif

#ifdef _LINUX
#	define SLASH '/'
#endif

namespace xgc
{
	static xgc_char g_module_name[XGC_MAX_FNAME] = { 0 };
	static xgc_char g_module_path[XGC_MAX_PATH] = { 0 };

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

			xgc_lpstr p = strrchr( g_module_name, SLASH );
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
			xgc_size cpyLen = GetModuleFileNameA( NULL, g_module_path, sizeof( g_module_path ) );
			#elif defined(_LINUX)
			xgc_size cpyLen = readlink("/proc/self/exe", g_module_path, sizeof( g_module_path ) );
			#else
			static_assert( false, "unknowe operator system." );
			#endif

			XGC_ASSERT_RETURN( cpyLen, xgc_nullptr );
			xgc_lpstr last = xgc_nullptr;
			xgc_lpstr found = strchr( g_module_path, SLASH );
			while( found )
			{
				last = found++;
				found = strchr( found, SLASH );
			}

			if( last ) *last = 0;
		}

		return g_module_path;
	}

	xgc_lpcstr get_normal_path( xgc_lpstr absolute, xgc_size size, xgc_lpcstr relative, ... )
	{
		xgc_lpcstr ret = xgc_nullptr;
		
		va_list args;
		va_start( args, relative );
		ret = get_normal_path_args( absolute, size, relative, args );
		va_end( args );

		return ret;
	}

	xgc_lpcstr get_normal_path_args( xgc_lpstr absolute, xgc_size size, xgc_lpcstr relative, va_list args )
	{
		int cpy1 = 0;
		char absolute_path[XGC_MAX_PATH] = { 0 };
		char relative_path[XGC_MAX_PATH] = { 0 };

		int cpy2 = vsprintf_s( relative_path, sizeof(relative_path), relative, args );
		if( cpy2 <= 0 )
			return xgc_nullptr;

		#if defined _WINDOWS
		xgc_lpcstr root = strchr( relative_path, ':' );
		if( xgc_nullptr == root )
		{
			cpy1 = sprintf_s( absolute_path, "%s%c", get_module_path(), SLASH );
			XGC_ASSERT_RETURN( cpy1 >= 0, xgc_nullptr );
		}
		else if( root - relative_path != 1 )
		{
			return xgc_nullptr;
		}
		#elif defined _LINUX
		if( relative_path[0] != SLASH )
		{
			cpy1 = sprintf_s( absolute_path, "%s%c", get_module_path(), SLASH );
			XGC_ASSERT_RETURN( cpy1 >= 0, xgc_nullptr );
		}
		#endif

		int cpy3 = sprintf_s( absolute_path + cpy1, sizeof(absolute_path) - cpy1, "%s", relative_path );

		if( cpy3 < 0 )
			return xgc_nullptr;

		if( cpy3 == sizeof(absolute_path) - cpy1 )
			return xgc_nullptr;

		#if defined _WINDOWS
		return _fullpath( absolute, absolute_path, size );
		#elif defined _LINUX
		if( realpath(absolute_path, absolute) )
			return absolute;
		
		return xgc_nullptr;
		#endif
	}
	
	COMMON_API xgc_ulong makepath( xgc_lpcstr path, xgc_bool recursion )
	{
		xgc_char absolute[XGC_MAX_PATH] = { 0 };
		//if( xgc_nullptr == get_normal_path(absolute, "%s", path) )
		//	return -1;

		strcpy_s(absolute, path);

		if( recursion )
		{
			xgc_lpcstr slash = "\\/";
			xgc_lpstr found = strpbrk( absolute, slash );
			while( found )
			{
				*found = 0;
				if( _access( absolute, 0 ) == -1 )
				{
					#ifdef _WINDOWS
					if( -1 == _mkdir( absolute ) )
						return -1;
					#endif

					#ifdef _LINUX
					if( -1 == mkdir( absolute, 0777 ) )
						return -1;
					#endif
				}
				*found = '/';

				found = strpbrk( found += strspn( found, slash ), slash );
			}
		}
		else
		{
			#ifdef _WINDOWS
			if( -1 == _mkdir( absolute ) )
				return -1;
			#endif

			#ifdef _LINUX
			if( -1 == mkdir( absolute, 0777 ) )
				return -1;
			#endif
		}
		return 0;
	}

	xgc_ulong get_process_id()
	{
		#if defined(_WINDOWS)
			return (xgc_ulong)GetCurrentProcessId();
		#elif defined(_LINUX)
			return (xgc_ulong)getpid();
		#endif
	}

	xgc_ulong get_thread_id()
	{
		#if defined(_WINDOWS)
			return (xgc_ulong)GetCurrentThreadId();
		#elif defined(_LINUX)
			return (xgc_ulong)gettid();
		#endif

		return -1;
	}
	
	#ifdef _WINDOWS
	xgc_bool get_process_memory_usage( xgc_uint64 *pnPMem, xgc_uint64 *pnVMem )
	{
		XGC_ASSERT_RETURN( pnPMem != pnVMem, false );
		if( pnPMem == xgc_nullptr && pnVMem == xgc_nullptr )
			return false;

		PROCESS_MEMORY_COUNTERS MEM;
		if( 0 == GetProcessMemoryInfo( GetCurrentProcess(), &MEM, sizeof( MEM ) ) )
			return false;

		if( pnPMem )
			*pnPMem = MEM.WorkingSetSize;

		if( pnVMem )
			*pnVMem = MEM.PagefileUsage;

		return true;
	}

	xgc_bool get_system_memory_usage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint64 *pnPrivateMem )
	{
		XGC_ASSERT_RETURN( pnUsageMem != pnTotalMem, false );
		if( pnUsageMem == xgc_nullptr && pnTotalMem == xgc_nullptr )
			return false;

		MEMORYSTATUSEX MEM;
		MEM.dwLength = sizeof( MEM );
		if( 0 == GlobalMemoryStatusEx( &MEM ) )
			return false;

		if( pnTotalMem )
			*pnTotalMem = MEM.ullTotalPhys;

		if( pnUsageMem )
			*pnUsageMem = MEM.ullTotalPhys - MEM.ullAvailPhys;

		if( pnPrivateMem )
			*pnPrivateMem = MEM.dwMemoryLoad;

		PROCESS_MEMORY_COUNTERS PMC;
		GetProcessMemoryInfo(GetCurrentProcess(), &PMC, sizeof(PMC));

		if( pnPrivateMem )
			*pnPrivateMem = PMC.WorkingSetSize;

		return true;
	}

	xgc_bool get_system_virtual_usage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint64 *pnPrivateMem )
	{
		XGC_ASSERT_RETURN( pnUsageMem != pnTotalMem, false );
		if( pnUsageMem == xgc_nullptr && pnTotalMem == xgc_nullptr )
			return false;

		MEMORYSTATUSEX MEM;
		MEM.dwLength = sizeof( MEM );
		if( 0 == GlobalMemoryStatusEx( &MEM ) )
			return false;

		if( pnTotalMem )
			*pnTotalMem = MEM.ullTotalPageFile;

		if( pnUsageMem )
			*pnUsageMem = MEM.ullTotalPageFile - MEM.ullAvailPageFile;

		PROCESS_MEMORY_COUNTERS_EX PMC;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&PMC, sizeof(PMC));

		if( pnPrivateMem )
			*pnPrivateMem = PMC.PrivateUsage;

		return true;
	}

	static PDH_HQUERY	cpuQuery = INVALID_HANDLE_VALUE;
	static PDH_HCOUNTER cpuTotal = INVALID_HANDLE_VALUE;

	xgc_real64 get_system_cpu_usage()
	{
		if( cpuQuery == INVALID_HANDLE_VALUE )
			PdhOpenQuery(NULL, NULL, &cpuQuery);

		if( cpuTotal == INVALID_HANDLE_VALUE )
			PdhAddCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);

		PdhCollectQueryData(cpuQuery);


		PDH_FMT_COUNTERVALUE counterVal;
		PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
		return counterVal.doubleValue;
	}

	static ULARGE_INTEGER lastCPU, lastSysCPU, lastUsrCPU;
	static int numProcessors = 0;

	xgc_real64 get_process_cpu_usage()
	{
		if( numProcessors == 0 )
		{
			SYSTEM_INFO sysInfo;
			FILETIME ftime, fsys, fuser;

			GetSystemInfo(&sysInfo);
			numProcessors = sysInfo.dwNumberOfProcessors;

			GetSystemTimeAsFileTime(&ftime);
			memcpy(&lastCPU, &ftime, sizeof(FILETIME));

			HANDLE self = GetCurrentProcess();
			GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
			memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
			memcpy(&lastUsrCPU, &fuser, sizeof(FILETIME));
		}

		FILETIME ftime, fsys, fuser;
		ULARGE_INTEGER now, sys, usr;
		double percent;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&now, &ftime, sizeof(FILETIME));

		HANDLE self = GetCurrentProcess();

		GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&usr, &fuser, sizeof(FILETIME));
		percent  = (sys.QuadPart - lastSysCPU.QuadPart) * 1.0;
		percent += (usr.QuadPart - lastUsrCPU.QuadPart) * 1.0;
		percent /= (now.QuadPart - lastCPU.QuadPart);
		percent /= numProcessors;
		lastCPU = now;
		lastUsrCPU = usr;
		lastSysCPU = sys;

		return percent * 100.0;
	}

	xgc_bool privilege( xgc_lpcstr prililege_name, xgc_bool enable /*= true */ )
	{
		bool bSuccess = false;

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
		
		return bSuccess;
	}
	#endif // _WINDOWS
	
	#ifdef _LINUX
	xgc_bool get_process_memory_usage( xgc_uint64 *pnMem, xgc_uint64 *pnVem )
	{
		auto parseLine = [](char* line){
			// This assumes that a digit will be found and the line ends in " Kb".
			const char* p = line;
			while (*p < '0' || *p > '9') p++;
			line[strlen(line)-3] = '\0';
			return atoi(p);
		};

		FILE* file = fopen("/proc/self/status", "r");
		char line[256];

		while (fgets(line, sizeof(line), file) != NULL)
		{
			if( pnMem && strncmp(line, "VmSize:", 7) == 0 )
				*pnMem = parseLine(line);

			if( pnVem && strncmp(line, "VmRSS:", 6) == 0 )
				*pnVem = parseLine(line);
		}

		fclose(file);

		return true;
	}

	xgc_bool get_system_virtual_usage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint64 *pnPrivateMem )
	{
		struct sysinfo memInfo;

		sysinfo (&memInfo);

		if( pnTotalMem )
		{
			*pnUsageMem  = memInfo.totalswap;
			*pnTotalMem *= memInfo.mem_unit;
		}

		if( pnUsageMem )
		{
			*pnUsageMem  = memInfo.totalswap - memInfo.freeswap;
			//Add other values in next statement to avoid int overflow on right hand side...
			*pnUsageMem *= memInfo.mem_unit;
		}

		if( pnPrivateMem )
		{
			auto parseLine = [](char* line){
				// This assumes that a digit will be found and the line ends in " Kb".
				const char* p = line;
				while (*p < '0' || *p > '9') p++;
				line[strlen(line)-3] = '\0';
				return atoi(p);
			};

			FILE* file = fopen("/proc/self/status", "r");
			char line[256];

			while (fgets(line, sizeof(line), file) != NULL)
			{
				if (strncmp(line, "VmRSS:", 6) == 0)
				{
					*pnPrivateMem = parseLine(line);
					break;
				}
			}
			fclose(file);
		}

		return true;
	}

	xgc_bool get_system_memory_usage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint64 *pnPrivateMem )
	{
		struct sysinfo memInfo;

		sysinfo (&memInfo);

		if( pnTotalMem )
		{
			*pnTotalMem = memInfo.totalram;
			//Add other values in next statement to avoid int overflow on right hand side...
			*pnTotalMem *= memInfo.mem_unit;
		}

		if( pnUsageMem )
		{
			*pnUsageMem  = memInfo.totalram - memInfo.freeram;
			//Add other values in next statement to avoid int overflow on right hand side...
			*pnUsageMem *= memInfo.mem_unit;
		}

		if( pnPrivateMem )
		{
			auto parseLine = [](char* line){
				// This assumes that a digit will be found and the line ends in " Kb".
				const char* p = line;
				while (*p < '0' || *p > '9') p++;
				line[strlen(line)-3] = '\0';
				return atoi(p);
			};

			FILE* file = fopen("/proc/self/status", "r");
			char line[256];

			while (fgets(line, sizeof(line), file) != NULL)
			{
				if (strncmp(line, "VmSize:", 7) == 0)
				{
					*pnPrivateMem = parseLine(line);
					break;
				}
			}
			fclose(file);
		}

		return true;
	}

	static clock_t lastCPU = 0;
	static clock_t lastSysCPU = 0;
	static clock_t lastUsrCPU = 0;
	static int numProcessors = 0;
	static xgc_uint64 lastTotalUser = 0, lastTotalUserLow = 0, lastTotalSys = 0, lastTotalIdle = 0;

	xgc_real64 get_process_cpu_usage()
	{
		struct tms timeSample;

		if( numProcessors == 0 )
		{
			FILE* file;
			char line[128];

			lastCPU = times(&timeSample);
			lastSysCPU = timeSample.tms_stime;
			lastUsrCPU = timeSample.tms_utime;

			file = fopen("/proc/cpuinfo", "r");
			numProcessors = 0;
			while(fgets(line, 128, file) != NULL){
				if (strncmp(line, "processor", 9) == 0) numProcessors++;
			}
			fclose(file);
		}

		clock_t now;
		xgc_real64 percent = 0.0;

		now = times(&timeSample);
		if( now <= lastCPU || 
			timeSample.tms_stime < lastSysCPU ||
			timeSample.tms_utime < lastUsrCPU )
		{
			//Overflow detection. Just skip this value.
			percent = 0.0;
		}
		else
		{
			percent  = (timeSample.tms_stime - lastSysCPU) + (timeSample.tms_utime - lastUsrCPU);
			percent /= (now - lastCPU);
			percent /= numProcessors;
			percent *= 100.0;
		}

		lastCPU = now;
		lastSysCPU = timeSample.tms_stime;
		lastUsrCPU = timeSample.tms_utime;

		return percent;
	}

	xgc_real64 get_system_cpu_usage()
	{
		xgc_real64 percent = 0.0f;
		xgc_uint64 totalUser, totalUserLow, totalSys, totalIdle, total;

		FILE* file = fopen("/proc/stat", "r");
		fscanf( file, 
				"cpu %llu %llu %llu %llu", 
				&totalUser, 
				&totalUserLow,
				&totalSys, 
				&totalIdle);

		fclose(file);

		if( totalUser < lastTotalUser || 
			totalUserLow < lastTotalUserLow ||
			totalSys < lastTotalSys || 
			totalIdle < lastTotalIdle)
		{
			//Overflow detection. Just skip this value.
			percent = 0.0;
		}
		else
		{
			total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) + (totalSys - lastTotalSys);
			percent = total;
			total += (totalIdle - lastTotalIdle);
			percent /= total;
			percent *= 100;
		}

		lastTotalUser = totalUser;
		lastTotalUserLow = totalUserLow;
		lastTotalSys = totalSys;
		lastTotalIdle = totalIdle;

		return percent;
	}

	xgc_bool privilege( xgc_lpcstr prililege_name, xgc_bool enable )
	{
		return true;
	}

	#endif
}