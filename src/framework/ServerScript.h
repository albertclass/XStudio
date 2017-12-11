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
/// \brief ��ʼ���ű�ģ��
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_bool InitServerScript( ini_reader &ini );

///
/// \brief ����ű�ģ��
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_void FiniServerScript();

///
/// \brief ��ȡ Lua ״̬��
/// \date 2017/08/28 
/// \author albert.xu
///
lua_State* luaState();

///
/// \brief ��ȡ�ű�ȫ����
/// \date 2017/08/28 
/// \author albert.xu
///
LuaRef luaGlobal( xgc_lpcstr lpName );

///
/// \brief �ӽ��ű���C++��
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_void luaHybrid( LuaRef &luaTable, xgc_lpcstr lpClassName );

///
/// \brief ִ�нű�
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_bool luaDoString( xgc_lpcstr lpScript );

///
/// \brief ִ�нű��ļ�
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_bool luaDoFile( xgc_lpcstr lpScriptFile );

///
/// \brief ִ�нű��ļ�
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_bool luaDoFileEx( xgc_lpcstr lpScriptFile, LuaRef* luaRet, int &nCount );

///
/// \brief ��ӡ�ű���ջ
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_void luaTraceback( xgc_lpcstr lpFileName, xgc_lpcstr lpFuncName, xgc_int32 nLine );

///
/// \brief ��ӡ�ű�ջԪ��
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_void luaStackDump( xgc_lpcstr lpMark = "empty" );

#endif // __SERVER_SCRIPT_H__