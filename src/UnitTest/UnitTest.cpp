// UnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <inttypes.h>

using namespace xgc;
using namespace xgc::common;

int main()
{
	ring_buffer< static_buffer< 1024 >, unite_buffer_recorder > ring;

	ring 
		<< 'c' << L'c' << 1 << 1L << 1LL
		<< 'c' << L'c' << 1U << 1UL << 1ULL 
		<< 1.0f << 1.0 
		<< "1234567890";

	xgc_int8  i8;
	xgc_int16 i16;
	xgc_int32 i32;
	xgc_int64 i64;
	xgc_long  il;

	xgc_uint8  u8;
	xgc_uint16 u16;
	xgc_uint32 u32;
	xgc_uint64 u64;
	xgc_ulong  ul;

	xgc_real32 r32;
	xgc_real64 r64;

	xgc_lpcstr str;

	ring
		>> i8 >> i16 >> i32 >> il >> i64
		>> u8 >> u16 >> u32 >> ul >> u64
		>> r32 >> r64
		>> str
		;

	shared_memory_buffer buf;
	buf.create( "utest", 1024 * 1024 );

	ring_buffer< reference_buffer, unite_buffer_recorder > shm( buf );
	shm << "1234567890" << endl;

	return 0;
}

