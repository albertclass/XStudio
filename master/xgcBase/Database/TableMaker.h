#pragma once
#ifndef _TABLE_MAKER_H_
#define _TABLE_MAKER_H_
#include "Database.h"

//////////////////////////////////////////////////////////////////////////
// add by albert [4/28/2013]
namespace XGC
{
	namespace DB
	{
		xgc_bool MakeTables( XGC::DB::sql_connection conn, xgc_lpcstr fconfig );
	}
}

extern "C"
{
	xgc_lpstr __declspec( dllexport ) hash_namerule( xgc_lpcstr name, xgc_lpcstr sql, xgc_lpcstr params );
}

xgc_void TableMakerTest();

#endif // _TABLE_MAKER_H_