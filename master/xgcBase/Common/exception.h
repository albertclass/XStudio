/********************************************************************
	created:	2010/04/26 16:36
	filename: 	xcbexception.h
	author:		lcj
	purpose:	
*********************************************************************/
#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_
#include "defines.h"
#include "exports.h"
#include "debuger.h"

#include "allocator.h"
#include "xsystem.h"

namespace XGC
{
	class COMMON_API std::exception;
	class COMMON_API CSEHexception : public std::exception
	{
	public:
		CSEHexception( UINT nErr, LPEXCEPTION_POINTERS pExceptionPoint )
			: lpEP( pExceptionPoint )
		{
			XGC_UNREFERENCED_PARAMETER( nErr );
		}

		LPEXCEPTION_POINTERS GetSEHInfo()
		{
			return lpEP;
		}

		const char* what();

	private:
		LPEXCEPTION_POINTERS lpEP;
	};

	COMMON_API xgc_bool InitException();
	COMMON_API xgc_void FiniException();

	COMMON_API xgc_void SEH_Exception_Call( CSEHexception& e, const char* file, int line );
	COMMON_API xgc_void STD_Exception_Call( std::exception& e, const char* file, int line );
	COMMON_API xgc_void ETC_Exception_Call( const char* file, int line );

	///
	/// 打印当前堆栈信息
	/// [9/2/2014] create by albert.xu
	///
	COMMON_API void DumpStackFrame();

	///
	/// 设置异常捕获文件的路径
	/// [12/3/2014] create by albert.xu
	///
	COMMON_API void SetExceptionLog( xgc_lpcstr pathname );

	///
	/// 设置异常捕获文件的路径
	/// [12/3/2014] create by albert.xu
	///
	COMMON_API void SetExceptionExt( xgc_lpcstr pathname );

	///
	/// 设置异常捕获文件的路径
	/// [12/3/2014] create by albert.xu
	///
	COMMON_API void SetExceptionDmp( xgc_lpcstr pathname );

}; // end of namespace XGC

#endif // _EXCEPTION_H_