///
/// CopyRight ? 2016 X Studio
/// \file exception.h
/// \date 八月 2016
///
/// \author albert.xu windxu@126.com
/// \brief windows 异常处理类
/// 

#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_
#include "defines.h"
#include "exports.h"
#include "debuger.h"

#include "allocator.h"
#include "xsystem.h"

namespace xgc
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

	class COMMON_API CMsgException : public std::exception
	{
	public:
		CMsgException( xgc_lpcstr msg, xgc_int32 iPos, xgc_lpcstr data, xgc_int32 size ) 
		{
			ZeroMemory( m_pCode, sizeof( m_pCode ) );
			sprintf_s( m_pCode, "%s\r\niPos:[%d],size:[%d]\r\n%s", msg, iPos, size, data );
		}

		const char* what() const
		{
			if (m_pCode)
			{
				return m_pCode;
			}

			return std::exception::what(); 
		}

	private:
		xgc_char m_pCode[16 * 1024];
	};

	COMMON_API xgc_bool InitException();
	COMMON_API xgc_void FiniException();

	COMMON_API xgc_void seh_exception_call( CSEHexception& e, const char* file, int line );
	COMMON_API xgc_void std_exception_call( std::exception& e, const char* file, int line );
	COMMON_API xgc_void etc_exception_call( const char* file, int line );

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

}; // end of namespace xgc

#endif // _EXCEPTION_H_