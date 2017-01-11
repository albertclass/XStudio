#include "frame.h"
#include "xsystem.h"
#include <thread>
#include <chrono>

using namespace xgc;

static int testmain( int agrc, char * argv[] )
{
	const char* module_name = get_module_name();
	const char* module_path = get_module_path();

	output( "module name = %s\n", module_name );
	output( "module path = %s\n", module_path );

	char path[XGC_MAX_PATH] = { 0 };
	get_normal_path( path, sizeof(path), "%s", "../../src/" );
	output( "normal path = %s\n", path );

	for( int i = 0; i < 100; ++i )
	{
		xgc_real64 cpu_proc = get_process_cpu_usage();
		xgc_real64 cpu_sys  = get_system_cpu_usage();
		output( "proc cpu usage %lf%%, system cpu usage %lf%%\n", cpu_proc, cpu_sys );

		xgc_uint64 total, usage, pmem, vmem;
		get_system_memory_usage( &total, &usage, &pmem );
		output( "system physical memory usage : total = %llu, usage = %llu, mem = %llu\n", total, usage, pmem );

		get_system_virtual_usage( &total, &usage, &vmem );
		output( "system virtual memory usage : total = %llu, usage = %llu, mem = %llu\n", total, usage, vmem );

		get_process_memory_usage( &pmem, &vmem );
		output( "process memory usage : pmem = %llu, vmem = %llu\n\n", pmem, vmem );

		std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
	}
	return 0;
}

UNIT_TEST( "system", "system test", testmain );
