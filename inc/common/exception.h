///
/// CopyRight ? 2016 X Studio
/// \file exception.h
/// \date 八月 2016
///
/// \author albert.xu windxu@126.com
/// \brief windows 异常处理类
/// 

#pragma  once

#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include "defines.h"
#include "exports.h"
#include "debuger.h"

#include "allocator.h"
#include "xsystem.h"

namespace xgc
{
	#if defined _WINDOWS
	class COMMON_API std::exception;
	#endif

	class COMMON_API seh_exception : public std::exception
	{
	public:
		seh_exception( int sig, void* pcontext )
			: _pcontext( pcontext )
		{
			XGC_UNREFERENCED_PARAMETER( sig );
		}

		~seh_exception() throw()
		{

		}

		void * get_context()
		{
			return _pcontext;
		}

		const char* what()const throw();

	private:
		void * _pcontext;
	};
	
	COMMON_API xgc_bool InitException();
	COMMON_API xgc_void FiniException();

	COMMON_API xgc_void seh_exception_call( seh_exception& e, const char* file, int line );
	COMMON_API xgc_void std_exception_call( std::exception& e, const char* file, int line );
	COMMON_API xgc_void etc_exception_call( const char* file, int line );

	///
	/// 打印当前堆栈信息
	/// [9/2/2014] create by albert.xu
	///
	COMMON_API void DumpStackFrame();

	///
	/// 设置异常日志文件的路径
	/// [12/3/2014] create by albert.xu
	///
	COMMON_API void SetExceptionLog( xgc_lpcstr pathname );

	///
	/// 设置异常日志文件的后缀名
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