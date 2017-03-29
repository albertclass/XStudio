#include "exception.h"
#include "debuger.h"
#include "logger.h"
#include "datetime.h"
#include "xsystem.h"

#if defined(_WINDOWS)
#	include <io.h>
#	include <direct.h>
#	define _NO_CVCONST_H
#	include "dbghelp.h"
#	pragma comment( lib, "dbghelp.lib")
#	pragma comment (lib, "version.lib")
#endif

#define MAX_FRAME_SIZE	(128)
#define MAX_FRAME_SAVE	(32)

namespace xgc
{
	static xgc_char g_exception_log[XGC_MAX_PATH] = { 0 };
	static xgc_char g_exception_ext[XGC_MAX_PATH] = { 0 };
	static xgc_char g_exception_dmp[XGC_MAX_PATH] = { 0 };

	///
	/// 栈帧信息
	/// [9/2/2014] create by albert.xu
	///
	struct StackFrameSequence
	{
		void*		Frame[MAX_FRAME_SAVE];
		uint32_t	Count;
	};

	#if defined(_WINDOWS)
	static HANDLE hProcess = INVALID_HANDLE_VALUE;
	static _se_translator_function g_exception_translator;
	static LPTOP_LEVEL_EXCEPTION_FILTER g_exception_filter;

	LONG WINAPI exception_filter( LPEXCEPTION_POINTERS lpEP );

	xgc_void exception_translate( UINT nErr, LPEXCEPTION_POINTERS pExceptionPoint )
	{
		throw seh_exception( nErr, pExceptionPoint );
	}
	
	xgc_bool InitException()
	{
		# ifndef _DEBUG
		g_exception_filter = ::SetUnhandledExceptionFilter( exception_filter );
		g_exception_translator = ::_set_se_translator( exception_translate );
		# endif

		get_normal_path( g_exception_log, sizeof( g_exception_log ), "%s", "exception" );
		get_normal_path( g_exception_ext, sizeof( g_exception_ext ), "%s", "log" );
		get_normal_path( g_exception_dmp, sizeof( g_exception_dmp ), "%s", "dmp" );

		// 获取进程句柄
		hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId() );
		if( hProcess == NULL )
		{
			DBG_INFO( "OpenProcess failed. err = %u", GetLastError() );
			return false;
		}

		// If the symbol engine is not initialized, do it now.
		if( !SymInitialize( hProcess, NULL, TRUE ) )
		{
			DBG_INFO( "SymInitialize failed. err = %u", GetLastError() );
			return false;
		}

		DumpStackFrame();
		SYS_INFO( "异常报告生成!" );
		return true;
	}

	xgc_void FiniException()
	{
		# ifndef _DEBUG
		
		if( m_exception_filter )
		{
			::SetUnhandledExceptionFilter( g_exception_filter );
		}

		if( m_exception_translator )
		{
			::_set_se_translator( g_exception_translator );
		}

		# endif

		SymCleanup( hProcess );
		CloseHandle( hProcess );
	}

	xgc_void GetStackTrace( StackFrameSequence& FrameSequence )
	{
		UINT_PTR* fp = (UINT_PTR*) _AddressOfReturnAddress();
		UINT_PTR* framePointer = (UINT_PTR*) fp - 1;

		FrameSequence.Count = 0;

	#if defined(_M_IX86)
		while ( FrameSequence.Count < MAX_FRAME_SAVE) {
			if (*framePointer < (UINT_PTR)framePointer) {
				if (*framePointer == NULL) {
					// Looks like we reached the end of the stack.
					break;
				}
				else {
					// Invalid frame pointer. Frame pointer addresses should always
					// increase as we move up the stack.
					break;
				}
			}
			if (*framePointer & (sizeof(UINT_PTR*) - 1)) {
				// Invalid frame pointer. Frame pointer addresses should always
				// be aligned to the size of a pointer. This probably means that
				// we've encountered a frame that was created by a module built with
				// frame pointer omission (FPO) optimization turned on.
				break;
			}
			if (IsBadReadPtr((UINT*)*framePointer, sizeof(UINT_PTR*))) {
				// Bogus frame pointer. Again, this probably means that we've
				// encountered a frame built with FPO optimization.
				break;
			}
			//push_back(*(framePointer + 1));
			FrameSequence.Frame[ FrameSequence.Count ] = *(framePointer + 1);
			FrameSequence.Count += 1;
			framePointer = (UINT_PTR*)*framePointer;
		}
	#elif defined(_M_X64)
		xgc_uint32 maxframes = XGC_MIN( 62, MAX_FRAME_SAVE + 10 );
		static USHORT( WINAPI *s_pfnCaptureStackBackTrace )( ULONG FramesToSkip, ULONG FramesToCapture, PVOID* BackTrace, PULONG BackTraceHash ) = 0;
		if( s_pfnCaptureStackBackTrace == 0 )
		{
			const HMODULE hNtDll = GetModuleHandleW( L"ntdll.dll" );
			reinterpret_cast<void*&>( s_pfnCaptureStackBackTrace )
				= ::GetProcAddress( hNtDll, "RtlCaptureStackBackTrace" );
			if( s_pfnCaptureStackBackTrace == 0 )
				return;
		}
		UINT_PTR* myFrames = XGC_NEW UINT_PTR[maxframes];
		ZeroMemory( myFrames, sizeof( UINT_PTR ) * maxframes );
		s_pfnCaptureStackBackTrace( 0, maxframes, (PVOID*) myFrames, NULL );
		xgc_uint32  startIndex = 0;
		xgc_uint32 count = 0;
		while( count < maxframes )
		{
			if( myFrames[count] == 0 )
				break;
			if( myFrames[count] == *( framePointer + 1 ) )
				startIndex = count;
			count++;
		}
		FrameSequence.Count = 0;
		while( 
			FrameSequence.Count < maxframes && 
			FrameSequence.Count < XGC_COUNTOF( FrameSequence.Frame ) )
		{
			if( myFrames[FrameSequence.Count] == 0 )
				break;
			//push_back(myFrames[count]);
			FrameSequence.Frame[FrameSequence.Count] = (void*)myFrames[startIndex + FrameSequence.Count];
			FrameSequence.Count += 1;
		}
		delete[] myFrames;
	#endif
	}

	xgc_void DumpStackFrame()
	{
		StackFrameSequence FrameSequence;
		GetStackTrace( FrameSequence );

		xgc_char szSymbol[sizeof( SYMBOL_INFO ) + 1024];
		SYS_INFO( "---------------stack frame begin--------------" );

		for( UINT nCur = 0; nCur < FrameSequence.Count && FrameSequence.Frame[nCur]; ++nCur )
		{
			ZeroMemory( szSymbol, sizeof( szSymbol ) );
			PSYMBOL_INFO pSymInfo = (PSYMBOL_INFO) &szSymbol;

			pSymInfo->SizeOfStruct = sizeof( SYMBOL_INFO );
			pSymInfo->MaxNameLen = sizeof( szSymbol ) - sizeof( SYMBOL_INFO );

			// Get the function.
			DWORD64 dwDisp = 0;
			if( SymFromAddr( hProcess, (UINT_PTR)FrameSequence.Frame[nCur], &dwDisp, pSymInfo ) )
			{
				// If I got a symbol, give the source and line a whirl.
				IMAGEHLP_LINE lineInfo;
				lineInfo.SizeOfStruct = sizeof( lineInfo );

				DWORD dwDisplacement = 0;
				if( SymGetLineFromAddr( hProcess, (UINT_PTR)FrameSequence.Frame[nCur], &dwDisplacement, &lineInfo ) )
				{
					// Put this on the next line and indented a bit.
					LOGEXT( lineInfo.FileName, pSymInfo->Name, lineInfo.LineNumber, "dump stack", "stack frame %p : %s", FrameSequence.Frame[nCur], pSymInfo->Name );
				}
				else
				{
					LOGFMT( "dump stack", "LastError:[%u], stack frame %p", GetLastError(), FrameSequence.Frame[nCur], pSymInfo->Name );
				}
			}
			else
			{
				LOGFMT( "dump stack", "stack frame %p", FrameSequence.Frame[nCur] );
			}
		}
		SYS_INFO( "---------------stack frame end--------------" );
	}

#if defined(_WIN64)
	static DWORD64 __stdcall GetModBase( HANDLE hProcess, DWORD64 dwAddr )
#else
	static DWORD __stdcall GetModBase( HANDLE hProcess, DWORD dwAddr )
#endif
	{
		// Check in the symbol engine first.
		IMAGEHLP_MODULE stIHM;

		// This is what the MFC stack trace routines forgot to do so their
		//  code will not get the info out of the symbol engine.
		stIHM.SizeOfStruct = sizeof( IMAGEHLP_MODULE );

		if( SymGetModuleInfo( hProcess, dwAddr, &stIHM ) )
		{
			return ( stIHM.BaseOfImage );
		}
		else
		{
			// Let's go fishing.
			MEMORY_BASIC_INFORMATION stMBI;

			if( 0 != VirtualQueryEx( hProcess, (LPCVOID) dwAddr, &stMBI, sizeof( stMBI ) ) )
			{
				// Try and load it.
				DWORD dwNameLen = 0;
				TCHAR szFile[MAX_PATH];
				memset( szFile, 0, sizeof( szFile ) );
				HANDLE hFile = NULL;

				dwNameLen = GetModuleFileName( (HINSTANCE) stMBI.AllocationBase, szFile, MAX_PATH );
				if( 0 != dwNameLen )
				{
					hFile = CreateFile( szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0 );
					SymLoadModule( hProcess, hFile, (PSTR) ( dwNameLen ? szFile : NULL ), NULL, (ULONG_PTR) stMBI.AllocationBase, 0 );
					CloseHandle( hFile );
				}
				return ( (ULONG_PTR) stMBI.AllocationBase );
			}
		}

		return ( 0 );
	}

	void dump_file( LPEXCEPTION_POINTERS lpEP )
	{
		if( _access( g_exception_dmp, 0 ) == -1 || ( _mkdir( g_exception_dmp ) == -1 && errno != EEXIST ) )
			return;

		SYSTEMTIME tm;
		GetLocalTime( &tm );
		xgc_char dumpName[XGC_MAX_PATH] = { 0 };
		sprintf_s( dumpName, "%s\\dump_%04d-%02d-%02d_%02d%02d%02d.%03d.dmp"
			, g_exception_dmp
			, tm.wYear
			, tm.wMonth
			, tm.wDay
			, tm.wHour
			, tm.wMinute
			, tm.wSecond
			, tm.wMilliseconds );

		HANDLE lhDumpFile = CreateFileA( dumpName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( lhDumpFile != INVALID_HANDLE_VALUE )
		{
			MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
			loExceptionInfo.ExceptionPointers = lpEP;
			loExceptionInfo.ThreadId = GetCurrentThreadId();
			loExceptionInfo.ClientPointers = TRUE;

			if( MiniDumpWriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(),
				lhDumpFile,
				MiniDumpNormal,
				&loExceptionInfo,
				NULL,
				NULL ) != TRUE )
			{
				HRESULT hreult = (HRESULT) GetLastError();
				void* pMsgBuf;
				::FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					hreult,
					MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
					(LPTSTR) &pMsgBuf,
					0,
					NULL
					);
				LocalFree( pMsgBuf );
			}
			CloseHandle( lhDumpFile );
		}
		else
		{
			HRESULT hreult = (HRESULT) GetLastError();
			void* pMsgBuf;
			::FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				hreult,
				MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
				(LPTSTR) &pMsgBuf,
				0,
				NULL
				);
			LocalFree( pMsgBuf );
		}
	}

	struct LocalVariablesReport
	{
		enum BasicType  // Stolen from CVCONST.H in the DIA 2.0 SDK
		{
			btNoType = 0,
			btVoid = 1,
			btChar = 2,
			btWChar = 3,
			btInt = 6,
			btUInt = 7,
			btFloat = 8,
			btBCD = 9,
			btBool = 10,
			btLong = 13,
			btULong = 14,
			btCurrency = 25,
			btDate = 26,
			btVariant = 27,
			btComplex = 28,
			btBit = 29,
			btBSTR = 30,
			btHresult = 31
		};

		HANDLE		m_hProcess;
		STACKFRAME* m_pStackFrame;

		FILE*		m_pFileHandle;
		LocalVariablesReport( HANDLE hProcess, STACKFRAME *pStackFrame, FILE *pFile )
			: m_hProcess( hProcess )
			, m_pStackFrame( pStackFrame )
			, m_pFileHandle( pFile )
		{

		}

		//============================================================================
		// Helper function that writes to the report file, and allows the user to use 
		// printf style formating                                                     
		//============================================================================
		void XGC_CDECL_CALL _putc( CHAR szCh )
		{
			fputc( szCh, m_pFileHandle );
		}

		void XGC_CDECL_CALL _putw( WCHAR szCh )
		{
			fputwc( szCh, m_pFileHandle );
		}

		//============================================================================
		// Helper function that writes to the report file, and allows the user to use 
		// printf style formating                                                     
		//============================================================================
		int XGC_CDECL_CALL _printf( xgc_lpcstr format, ... )
		{
			va_list args;
			va_start( args, format );
			int write = vfprintf_s( m_pFileHandle, format, args );
			va_end( args );

			return write;
		}

		void DumpChild( DWORD64 modBase, DWORD dwTypeIndex, unsigned nestingLevel, DWORD_PTR offset )
		{
			// Determine how many children this type has.
			DWORD dwChildrenCount = 0;
			SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_GET_CHILDRENCOUNT, &dwChildrenCount );

			// Prepare to get an array of "TypeIds", representing each of the children.
			// SymGetTypeInfo(TI_FINDCHILDREN) expects more memory than just a
			// TI_FINDCHILDREN_PARAMS struct has.  Use derivation to accomplish this.
			struct FINDCHILDREN : TI_FINDCHILDREN_PARAMS
			{
				ULONG   MoreChildIds[64];
			} children;

			children.Count = __min( dwChildrenCount, _countof( children.MoreChildIds ) );
			children.Start = 0;

			// Get the array of TypeIds, one for each child type
			if( SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_FINDCHILDREN, &children ) )
			{
				_putc( '\n' );
				// Iterate through each of the children
				for( unsigned i = 0; i < dwChildrenCount; i++ )
				{
					DWORD dwMemberOffset = 0;
					// Get the offset of the child member, relative to its parent
					if( !SymGetTypeInfo( m_hProcess, modBase, children.ChildId[i], TI_GET_OFFSET, &dwMemberOffset ) )
						continue;

					// If the child wasn't a UDT, format it appropriately
					DumpTypeIndex( modBase, children.ChildId[i], nestingLevel + 1, offset + dwMemberOffset );
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////////
		// If it's a user defined type (UDT), recurse through its members until we're
		// at fundamental types.  When he hit fundamental types, return
		// bHandled = false, so that FormatSymbolValue() will format them.
		//////////////////////////////////////////////////////////////////////////////

		bool DumpTypeIndex( DWORD64 modBase, DWORD dwTypeIndex, unsigned nestingLevel, DWORD_PTR offset, char ending = '\n' )
		{
			if( nestingLevel > 5 )
			{
				return false;
			}

			DWORD typeTag = SymTagNull;
			if( !SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_GET_SYMTAG, &typeTag ) )
				return false;

			// don't process function.
			if( typeTag == SymTagFunction || typeTag == SymTagTypedef )
				return false;

			// Get the name of the symbol.  This will either be a Type name (if a UDT),
			// or the structure member name.
			WCHAR * pwszTypeName;
			CHAR pszTypeName[256] = { 0 };
			if( SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_GET_SYMNAME, &pwszTypeName ) )
			{
				if( pwszTypeName[0] == 0 )
				{
					LocalFree( pwszTypeName );
					return false;
				}

				//if( nestingLevel == 1 ) _putc( '\n' );

				if( typeTag == SymTagUDT )
				{
					_putc( '\n' );
					// Add appropriate indentation level (since this routine is recursive)
					for( unsigned j = 0; j <= nestingLevel; j++ )
						_putc( '\t' );
					_printf( "{%lS}", pwszTypeName );
				}
				else
				{
					// Add appropriate indentation level (since this routine is recursive)
					for( unsigned j = 0; j <= nestingLevel; j++ )
						_putc( '\t' );

					_printf( "%lS = ", pwszTypeName );
				}

				sprintf_s( pszTypeName, "%lS", pwszTypeName );
				LocalFree( pwszTypeName );
			}

			ULONG64 length = 0;

			// Get the size of the child member
			SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_GET_LENGTH, &length );

			DWORD typeId = 0;
			bool ret = true;
			switch( typeTag )
			{
				case SymTagData:
				if( SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_GET_TYPEID, &typeId ) )
				{
					ret = DumpTypeIndex( modBase, typeId, nestingLevel, offset );
				}
				else
				{
					_putc( ending );
				}

				break;
				case SymTagUDT:
				if( offset > 0xffff )
					DumpChild( modBase, dwTypeIndex, nestingLevel, offset );
				else
					_putc( ending );
				break;
				case SymTagEnum:
				if( SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_GET_TYPEID, &typeId ) )
				{
					ret = DumpTypeIndex( modBase, typeId, nestingLevel + 1, offset );
				}
				else
				{
					_putc( ending );
				}
				break;
				case SymTagPointerType:
				{
					_printf( "{0x%p}", *( (PVOID*) offset ) );
					if( !SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_GET_TYPEID, &typeId ) )
						break;

					ULONG64 dwLength;
					if( !SymGetTypeInfo( m_hProcess, modBase, typeId, TI_GET_LENGTH, &dwLength ) )
						break;

					DWORD dwBasicType = 0;
					if( SymGetTypeInfo( m_hProcess, modBase, typeId, TI_GET_BASETYPE, &dwBasicType ) )
					{
						if( dwBasicType == btChar || dwBasicType == btWChar )
						{
							FormatOutputValue( (BasicType) dwBasicType, length, *(LPVOID*) offset );
							if( ending ) _putc( ending );
							break;
						}
					}
					else
					{
						ret = DumpTypeIndex( modBase, typeId, nestingLevel, offset );
					}
				}
				break;
				case SymTagArrayType:
				{
					DWORD dwCount = 0;
					DWORD dwTypeId = 0;
					ULONG64 dwTypeLength = 0;
					DWORD dwBasicType;
					if( !SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_GET_COUNT, &dwCount ) )
					{
						_putc( ending );
						break;
					}
					if( !SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_GET_TYPEID, &dwTypeId ) )
					{
						_putc( ending );
						break;
					}

					if( !SymGetTypeInfo( m_hProcess, modBase, dwTypeId, TI_GET_LENGTH, &dwTypeLength ) )
					{
						_putc( ending );
						break;
					}

					char ending = ',';
					if( SymGetTypeInfo( m_hProcess, modBase, dwTypeId, TI_GET_BASETYPE, &dwBasicType ) )
					{
						if( dwBasicType == btWChar || dwBasicType == btChar )
							ending = 0;
					}

					_printf( "{0x%p} ", offset );
					for( DWORD dwIndex = 0; dwIndex < dwCount && dwIndex < 8; ++dwIndex )
					{
						DumpTypeIndex( modBase, dwTypeId, nestingLevel + 1, (DWORD_PTR) ( offset + dwIndex * dwTypeLength ), ending );
					}
					if( ending != '\n' ) _putc( '\n' );
				}
				break;
				case SymTagBaseType:
				{
					BasicType basicType = GetBasicType( dwTypeIndex, modBase );
					FormatOutputValue( basicType, length, (PVOID) offset );
					if( ending ) _putc( ending );
				}
				break;
				case SymTagBaseClass:
				if( SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_GET_TYPEID, &typeId ) )
				{
					ret = DumpTypeIndex( modBase, typeId, nestingLevel + 1, offset );
				}
				else
				{
					if( ending ) _putc( ending );
				}
				break;
				default:
				_printf( "{0x%p} ", offset );
				if( ending ) _putc( ending );
				break;
			}

			return ret;
		}

		void FormatOutputValue( BasicType basicType, DWORD64 length, PVOID pAddress )
		{
			// Format appropriately (assuming it's a 1, 2, or 4 bytes (!!!)
			if( basicType == btNoType )
			{
				_printf( " = 0x%p (NoType)", (PVOID) pAddress );
			}
			else if( !IsBadReadPtr( (PVOID) pAddress, (UINT_PTR) length ) )
			{
				switch( basicType )
				{
					case btBool:
					_printf( "%s", ( *(bool*) pAddress ) ? "true" : "false" );
					break;
					case btInt:
					case btLong:
					switch( length )
					{
						case 2:
						_printf( "%d", INT32( *(INT16*) pAddress ) );
						break;
						case 4:
						_printf( "%d", *(INT32*) pAddress );
						break;
						case 8:
						_printf( "%ld", *(LONG*) pAddress );
						break;
						default:
						_printf( "0x%p (Length = %d)", *(PBYTE) pAddress, length );
						break;
					}
					break;
					case btUInt:
					case btULong:
					switch( length )
					{
						case 2:
						_printf( "%u", xgc_uint32( *(UINT16*) pAddress ) );
						break;
						case 4:
						_printf( "%u", *(xgc_uint32*) pAddress );
						break;
						case 8:
						_printf( "%lu", *(ULONG*) pAddress );
						break;
						default:
						_printf( "0x%p (Length = %d)", *(PBYTE) pAddress, length );
						break;
					}
					break;
					case btFloat:
					switch( length )
					{
						case 4:
						_printf( "%f", *(FLOAT*) pAddress );
						break;
						case 8:
						_printf( "%lf", *(xgc_real64*) pAddress );
						break;
						default:
						_printf( "0x%p (Length = %d)", *(PBYTE) pAddress, length );
						break;
					}
					break;
					case btChar:
					if( !IsBadReadPtr( (PSTR) pAddress, (UINT_PTR) length ) )
						for( int n = 0; n < length && ( (CHAR*) pAddress )[n]; ++n )
						{
						if( ( (CHAR*) pAddress )[n] == '\n' )
							_printf( "\\n" );
						else if( ( (CHAR*) pAddress )[n] == '\r' )
							_printf( "\\r" );
						else
							_putc( ( (CHAR*) pAddress )[n] );
						}
					else
						_printf( " = 0x%p (Bad String)", *(PDWORD) pAddress );
					break;
					case btWChar:
					if( !IsBadReadPtr( (PWSTR) pAddress, (UINT_PTR) length ) )
						for( int n = 0; n < length / 2 && ( (WCHAR*) pAddress )[n]; ++n )
							_putw( ( (WCHAR*) pAddress )[n] );
					else
						_printf( "0x%p (Bad String)", *(PDWORD) pAddress );
					break;
					default:
					_printf( "0x%p (Length = %d)", (LPVOID) pAddress, length );
					break;
				}
			}
		}

		BasicType GetBasicType( DWORD typeIndex, DWORD64 modBase )
		{
			BasicType basicType;
			if( SymGetTypeInfo( m_hProcess, modBase, typeIndex, TI_GET_BASETYPE, &basicType ) )
			{
				return basicType;
			}

			// Get the real "TypeId" of the child.  We need this for the
			// SymGetTypeInfo( TI_GET_TYPEID ) call below.
			DWORD typeId;
			if( SymGetTypeInfo( m_hProcess, modBase, typeIndex, TI_GET_TYPEID, &typeId ) )
			{
				if( SymGetTypeInfo( m_hProcess, modBase, typeId, TI_GET_BASETYPE, &basicType ) )
				{
					return basicType;
				}
			}

			return btNoType;
		}

		void FormatSymbolValue( PSYMBOL_INFO pSym )
		{
			// If it's a function, don't do anything.
			if( pSym->Tag == SymTagFunction )   // SymTagFunction from CVCONST.H from the DIA SDK
			{
				_putc( '\n' );
				return;
			}

			// Indicate if the variable is a local or parameter
			if( pSym->Flags & IMAGEHLP_SYMBOL_INFO_PARAMETER )
				_printf( "\tParameter = %s", pSym->Name );
			else if( pSym->Flags & IMAGEHLP_SYMBOL_INFO_LOCAL )
				_printf( "\tLocal = %s", pSym->Name );
			else
			{
				// Emit the variable name
				_printf( "\t%s", pSym->Name );
			}

			DWORD_PTR pVariable = 0;    // Will point to the variable's data in memory

			if( pSym->Flags & IMAGEHLP_SYMBOL_INFO_REGRELATIVE )
			{
				pVariable = m_pStackFrame->AddrFrame.Offset;
				pVariable += (DWORD_PTR) pSym->Address;
			}
			else if( pSym->Flags & IMAGEHLP_SYMBOL_INFO_REGISTER )
			{
				_putc( '\n' );
				return;   // Don't try to report register variable
			}
			else
			{
				pVariable = (DWORD_PTR) pSym->Address;   // It must be a global variable
			}

			// Determine if the variable is a user defined type (UDT).  IF so, bHandled
			// will return true.

			if( !DumpTypeIndex( pSym->ModBase, pSym->TypeIndex, 1, pVariable ) )
			{
				_putc( '\n' );
			}
			return;
		}
	};

	BOOL CALLBACK EnumerateSymbolsCallback( PSYMBOL_INFO  pSymInfo, ULONG SymbolSize, PVOID UserContext )
	{
		__try
		{
			LocalVariablesReport* pReport = (LocalVariablesReport*) UserContext;
			if( pReport )
			{
				pReport->FormatSymbolValue( pSymInfo );
			}
		}
		__except( 1 )
		{
			printf( "puting on symbol %s\n", pSymInfo->Name );
		}

		return TRUE;
	}

	void __stdcall  dump_callstack( CONTEXT *pCtx )
	{
		FILE * fp = xgc_nullptr;
		if( fopen_s( &fp, g_exception_ext, "a+" ) == 0 && fp )
		{
			ULONGLONG dwTickCount;
			dwTickCount = GetTickCount64();
			STACKFRAME stFrame;
			ZeroMemory( &stFrame, sizeof( stFrame ) );
			DWORD dwMachine = IMAGE_FILE_MACHINE_I386;

#ifdef _M_IX86
			dwMachine					= IMAGE_FILE_MACHINE_I386 ;
			stFrame.AddrPC.Offset		= pCtx->Eip;
			stFrame.AddrPC.Mode			= AddrModeFlat;
			stFrame.AddrStack.Offset	= pCtx->Esp;
			stFrame.AddrStack.Mode		= AddrModeFlat;
			stFrame.AddrFrame.Offset	= pCtx->Ebp;
			stFrame.AddrFrame.Mode		= AddrModeFlat ;
#elif _M_X64
			dwMachine = IMAGE_FILE_MACHINE_AMD64;
			stFrame.AddrPC.Offset = pCtx->Rip;
			stFrame.AddrPC.Mode = AddrModeFlat;
			stFrame.AddrStack.Offset = pCtx->Rsp;
			stFrame.AddrStack.Mode = AddrModeFlat;
			stFrame.AddrFrame.Offset = pCtx->Rbp;
			stFrame.AddrFrame.Mode = AddrModeFlat;
#elif _M_IA64
			dwMachine					= IMAGE_FILE_MACHINE_I386;
			stFrame.AddrPC.Offset		= pCtx->StIIp;
			stFrame.AddrPC.Mode			= AddrModeFlat;
			stFrame.AddrStack.Offset	= pCtx->IntSp;
			stFrame.AddrStack.Mode		= AddrModeFlat;
			stFrame.AddrFrame.Offset	= pCtx->RsBSP;
			stFrame.AddrFrame.Mode		= AddrModeFlat;
#else
			return;
#endif
			HANDLE hThread = GetCurrentThread();

			SYSTEMTIME time;
			ZeroMemory( &time, sizeof( SYSTEMTIME ) );
			GetLocalTime( &time );
			fprintf( fp, "[%u-%02u-%02u %02u:%02u:%02u] 异常开始\n",
				time.wYear, time.wMonth, time.wDay,
				time.wHour, time.wMinute, time.wSecond );

			while( StackWalk( dwMachine, hProcess, hThread, &stFrame, pCtx, 0, SymFunctionTableAccess, SymGetModuleBase, 0 ) )
			{
				if( stFrame.AddrFrame.Offset == 0 )
				{
					break;
				}

				BYTE symbolBuffer[sizeof( SYMBOL_INFO ) + 256];
				ZeroMemory( symbolBuffer, sizeof( symbolBuffer ) );
				PSYMBOL_INFO pSymbol = (PSYMBOL_INFO) symbolBuffer;

				pSymbol->SizeOfStruct = sizeof( SYMBOL_INFO );
				pSymbol->MaxNameLen = 256;

				IMAGEHLP_LINE lineInfo;
				lineInfo.SizeOfStruct = sizeof( lineInfo );
				DWORD64 dwDisplacement = 0;
				ULONG dwLineDisplacement = 0;

				if( SymFromAddr( hProcess, stFrame.AddrPC.Offset, &dwDisplacement, pSymbol ) )
				{
					if( SymGetLineFromAddr( hProcess, stFrame.AddrPC.Offset, &dwLineDisplacement, &lineInfo ) )
					{
						fprintf( fp, "%s(%d):%s\n", lineInfo.FileName, lineInfo.LineNumber, pSymbol->Name );
					}
				}
				else
				{
					DWORD dwErr = GetLastError();
					fprintf( fp, "SymFromAddr Err=%u \n", dwErr );
				}
			}

			fprintf( fp, "记录用时：%I64u 毫秒 \n", GetTickCount64() - dwTickCount );
			fprintf( fp, "异常结束******************************\n\n" );

			fclose( fp );
		}
	}

	LONG WINAPI exception_filter( LPEXCEPTION_POINTERS lpEP )
	{
		UINT nCount = 0;
#ifndef _DEBUG
		if( !g_StackFrameRecoder.InsertFrame( ( (UINT_PTR*) _AddressOfReturnAddress() ) - 1, &nCount ) )
		{
			return EXCEPTION_EXECUTE_HANDLER;
		}
#endif
		dump_file( lpEP );
		dump_callstack( lpEP->ContextRecord );

		return EXCEPTION_EXECUTE_HANDLER;
	}

	const char* seh_exception::what() const throw()
	{
		if( _pcontext )
		{
			LPEXCEPTION_POINTERS lpEP = (LPEXCEPTION_POINTERS)_pcontext;
			xgc_lpcstr info = "";
			EXCEPTION_RECORD * pERec = lpEP->ExceptionRecord;
			switch( pERec->ExceptionCode )
			{
				case EXCEPTION_ACCESS_VIOLATION:
				info = "EXCEPTION_ACCESS_VIOLATION";
				break;
				case EXCEPTION_DATATYPE_MISALIGNMENT:
				info = "EXCEPTION_DATATYPE_MISALIGNMENT";
				break;
				case EXCEPTION_BREAKPOINT:
				info = "EXCEPTION_BREAKPOINT";
				break;
				case EXCEPTION_SINGLE_STEP:
				info = "EXCEPTION_SINGLE_STEP";
				break;
				case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
				info = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
				break;
				case EXCEPTION_FLT_DENORMAL_OPERAND:
				info = "EXCEPTION_FLT_DENORMAL_OPERAND";
				break;
				case EXCEPTION_FLT_DIVIDE_BY_ZERO:
				info = "EXCEPTION_FLT_DIVIDE_BY_ZERO";
				break;
				case EXCEPTION_FLT_INEXACT_RESULT:
				info = "EXCEPTION_FLT_INEXACT_RESULT";
				break;
				case EXCEPTION_FLT_INVALID_OPERATION:
				info = "EXCEPTION_FLT_INVALID_OPERATION";
				break;
				case EXCEPTION_FLT_OVERFLOW:
				info = "EXCEPTION_FLT_OVERFLOW";
				break;
				case EXCEPTION_FLT_STACK_CHECK:
				info = "EXCEPTION_FLT_STACK_CHECK";
				break;
				case EXCEPTION_FLT_UNDERFLOW:
				info = "EXCEPTION_FLT_UNDERFLOW";
				break;
				case EXCEPTION_INT_DIVIDE_BY_ZERO:
				info = "EXCEPTION_INT_DIVIDE_BY_ZERO";
				break;
				case EXCEPTION_INT_OVERFLOW:
				info = "EXCEPTION_INT_OVERFLOW";
				break;
				case EXCEPTION_PRIV_INSTRUCTION:
				info = "EXCEPTION_PRIV_INSTRUCTION";
				break;
				case EXCEPTION_IN_PAGE_ERROR:
				info = "EXCEPTION_IN_PAGE_ERROR";
				break;
				case EXCEPTION_ILLEGAL_INSTRUCTION:
				info = "EXCEPTION_ILLEGAL_INSTRUCTION";
				break;
				case EXCEPTION_NONCONTINUABLE_EXCEPTION:
				info = "EXCEPTION_NONCONTINUABLE_EXCEPTION";
				break;
				case EXCEPTION_STACK_OVERFLOW:
				info = "EXCEPTION_STACK_OVERFLOW";
				break;
				case EXCEPTION_INVALID_DISPOSITION:
				info = "EXCEPTION_INVALID_DISPOSITION";
				break;
				case EXCEPTION_GUARD_PAGE:
				info = "EXCEPTION_GUARD_PAGE";
				break;
				case EXCEPTION_INVALID_HANDLE:
				info = "EXCEPTION_INVALID_HANDLE";
				break;
			}
			return info;
		}

		return std::exception::what();
	}
	#elif defined _LINUX
	void bt_sighandler(int sig, siginfo_t * info, void * ptr) 
	{
		void *trace[16];
		char **messages = (char **)NULL;
		int i, trace_size = 0;
		struct sigcontext* ctx = (sigcontext*) ptr;
		
		#if defined(_AI_X32)
		void *call = (void*)ctx->eip;
		#endif

		#if defined(_AI_X64)
		void *call = (void*)ctx->rip;
		#endif
		
		if (sig == SIGSEGV)
			SYS_INFO("Got signal %d, faulty address is %p, from %p", sig, ctx->cr2, call);
		else
			SYS_INFO("Got signal %d", sig);

		trace_size = backtrace(trace, 16);
		/* overwrite sigaction with caller's address */
		// trace[1] = call;
		messages = backtrace_symbols(trace, trace_size);
		/* skip first stack frame (points here) */
		SYS_INFO("[bt] Execution stackwalk:");
		for (i=1; i<trace_size; ++i)
		{
			SYS_INFO("[bt] #%d %s\n", i, messages[i]);
		}

		siglongjmp(sigjmp_env, sig);
	}
	
	xgc_bool InitException()
	{
		struct sigaction sa;

		sigemptyset(&sa.sa_mask); 
		sa.sa_flags = SA_RESTART | SA_SIGINFO;
		sa.sa_sigaction = bt_sighandler;

		sigaction(SIGFPE ,  &sa, NULL);
		sigaction(SIGSEGV,  &sa, NULL); 
		sigaction(SIGUSR1,  &sa, NULL);
	}

	xgc_void FiniException()
	{
	}

	const char* seh_exception::what() const throw()
	{
		return std::exception::what();
	}

	xgc_void GetStackTrace( StackFrameSequence& FrameSequence )
	{
		FrameSequence.Count = backtrace( FrameSequence.Frame, XGC_COUNTOF(FrameSequence.Frame) );
	}

	xgc_void DumpStackFrame()
	{
		StackFrameSequence FrameSequence;
		GetStackTrace( FrameSequence );

		SYS_INFO( "---------------stack frame begin--------------" );

		char **messages = backtrace_symbols(FrameSequence.Frame, FrameSequence.Count);
		for( xgc_uint32 i = 0; i < FrameSequence.Count; ++i )
		{
			SYS_INFO("[bt] #%2d:%s", i, messages[i]);
		}

		SYS_INFO( "---------------stack frame end--------------" );
	}

	#endif // _WINDOWS

	void seh_exception_call( seh_exception& e, const char* file, int line )
	{
		FILE * fp = xgc_nullptr;
		if( fopen_s( &fp, g_exception_log, "a+" ) == 0 && fp )
		{
			datetime now = datetime::now();
			xgc_char szDateTime[64];
			now.to_string( szDateTime, sizeof( szDateTime ) );
			fprintf( fp, "[%s] SEH %s:%d:%s\n", szDateTime, file, line, e.what() );
			fclose( fp );
		}
	}

	void std_exception_call( std::exception& e, const char* file, int line )
	{
		FILE * fp = xgc_nullptr;
		if( fopen_s( &fp, g_exception_log, "a+" ) == 0 && fp )
		{
			datetime now = datetime::now();
			xgc_char szDateTime[64];
			now.to_string( szDateTime, sizeof( szDateTime ) );
			fprintf( fp, "[%s] STD %s:%d:%s\n", szDateTime, file, line, e.what() );
			fclose( fp );
		}
	}

	void etc_exception_call( const char* file, int line )
	{
		FILE * fp = xgc_nullptr;
		if( fopen_s( &fp, g_exception_log, "a+" ) == 0 && fp )
		{
			datetime now = datetime::now();
			xgc_char szDateTime[64];
			now.to_string( szDateTime, sizeof( szDateTime ) );
			fprintf( fp, "[%s] ETC %s:%d\n", szDateTime, file, line );
			fclose( fp );
		}
	}

	///
	/// 设置异常捕获文件的路径
	/// [12/3/2014] create by albert.xu
	///
	void SetExceptionLog( xgc_lpcstr pathname )
	{
		strcpy_s( g_exception_log, pathname );
	}

	///
	/// 设置异常捕获文件的路径
	/// [12/3/2014] create by albert.xu
	///
	void SetExceptionExt( xgc_lpcstr pathname )
	{
		strcpy_s( g_exception_ext, pathname );
	}

	///
	/// 设置异常捕获文件的路径
	/// [12/3/2014] create by albert.xu
	///
	void SetExceptionDmp( xgc_lpcstr pathname )
	{
		strcpy_s( g_exception_dmp, pathname );
	}

	struct StackFrameRecoder
	{
	private:
		StackFrameSequence StackFrame[MAX_FRAME_SIZE];
		xgc_ulong dwSaveFramePoint;
	public:
		StackFrameRecoder()
			: dwSaveFramePoint( 0 )
		{
			memset( StackFrame, 0, sizeof( StackFrame ) );
		}

		~StackFrameRecoder()
		{
		}

		StackFrameSequence* IsFrameExist( const StackFrameSequence& FrameSequence )
		{
			for( auto i = 0; i < MAX_FRAME_SAVE; ++i )
			{
				StackFrameSequence &backtrace = StackFrame[i];
				if( backtrace.Count != FrameSequence.Count )
					continue;
				
				uint32_t c = 0;;
				while( backtrace.Count > c )
				{
					if( FrameSequence.Frame[c] != backtrace.Frame[c] )
						break;

					--c;
				}

				if( c == backtrace.Count )
					return StackFrame + i;
			}

			return xgc_nullptr;
		}

		bool InsertFrame( StackFrameSequence &backtrace )
		{
			if( dwSaveFramePoint >= MAX_FRAME_SIZE )
				dwSaveFramePoint = 0;

			GetStackTrace( StackFrame[dwSaveFramePoint] );

			++dwSaveFramePoint;
		}
	};

	static StackFrameRecoder g_StackFrameRecoder;
}// end of namespace xgc
