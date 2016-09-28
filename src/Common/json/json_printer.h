#ifndef _JSON_PRINTER_H_
#define _JSON_PRINTER_H_
#include "..\defines.h"
#include "..\exports.h"
#include "json.h"
#include "value.h"
//////////////////////////////////////////////////////////////////////////
//
// 打印JSON对象内容
// 示例：
// 	char prefix[256] = { ' ' };
//	JsonPrint( json_object, prefix, prefix+1, _countof(prefix), "\r\n" );
//////////////////////////////////////////////////////////////////////////
COMMON_API xgc_void JsonPrint( const Json::Value& node, char *prefix, char *prefix_cur, int max_deep, char *endless );
#endif // _JSON_PRINTER_H_