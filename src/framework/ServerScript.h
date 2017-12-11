#ifndef __SERVER_SCRIPT_H__
#define __SERVER_SCRIPT_H__
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "LuaBridge/LuaBridge.h"
using namespace luabridge;

///
/// \brief 初始化脚本模块
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_bool InitServerScript( ini_reader &ini );

///
/// \brief 清理脚本模块
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_void FiniServerScript();

///
/// \brief 获取 Lua 状态机
/// \date 2017/08/28 
/// \author albert.xu
///
lua_State* luaState();

///
/// \brief 获取脚本全局量
/// \date 2017/08/28 
/// \author albert.xu
///
LuaRef luaGlobal( xgc_lpcstr lpName );

///
/// \brief 杂交脚本和C++类
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_void luaHybrid( LuaRef &luaTable, xgc_lpcstr lpClassName );

///
/// \brief 执行脚本
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_bool luaDoString( xgc_lpcstr lpScript );

///
/// \brief 执行脚本文件
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_bool luaDoFile( xgc_lpcstr lpScriptFile );

///
/// \brief 执行脚本文件
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_bool luaDoFileEx( xgc_lpcstr lpScriptFile, LuaRef* luaRet, int &nCount );

///
/// \brief 打印脚本堆栈
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_void luaTraceback( xgc_lpcstr lpFileName, xgc_lpcstr lpFuncName, xgc_int32 nLine );

///
/// \brief 打印脚本栈元素
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_void luaStackDump( xgc_lpcstr lpMark = "empty" );

#endif // __SERVER_SCRIPT_H__