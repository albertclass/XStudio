/*******************************************************************/
//! \file debuger.h
//! \brief ���Ը�������ش���
//! 2014/06/11 by Albert.xu
/*******************************************************************/
#ifndef _DEBUGER_H_
#define _DEBUGER_H_
#include "defines.h"
#include "exports.h"

#include "datetime.h"
#include "exception.h"
#include "synchronous.h"

#define WATCH_STACKFRAME_MAX (64)

// ���̼߳�飬���˺�д��ֻ�ܵ��̵߳��õĺ����м��ɣ����ж��̵߳�������ڿ��߳�ʱ������
#define XGC_SINGLE_THREAD_CHECKER( FMT, ... )\
	static DWORD __single_thread_checker__ = GetCurrentThreadId();\
	if( __single_thread_checker__ != GetCurrentThreadId() )\
	{\
		SYS_WARNING( "Current Thread %u Previous Thread %u >" FMT, \
		__single_thread_checker__, \
		GetCurrentThreadId(), \
		__VA_ARGS__ );\
		DumpStackFrame();\
	}\

#define XGC_CHECK_REENTTRY_CALL( V ) \
	TwiceEnterChecker __FUNCTION__##_CHECKER( __FILE__, __LINE__, V );\

#define FUNCTION_BEGIN\
	try\
	{\
		XGC::InvokeWatcherWarp __InvokerWatcher__( XGC::getInvokeWatcher( ), __FILE__, __LINE__ ); \

#define FUNCTION_CATCH( ... )\
	}\
	catch( __VA_ARGS__ )\
	{\

#define FUNCTION_END\
	}\
	catch( XGC::CSEHexception& e )\
	{\
	SYS_INFO( "SEH Exception throw." );\
	XGC_DEBUG_MESSAGE( "SEH Exception. %s", e.what() ); \
	XGC::SEH_Exception_Call( e, __FILE__, __LINE__ ); \
	}\
	catch( std::exception e )\
	{\
		SYS_INFO( "STD Exception throw. %s", e.what() ); \
		XGC_DEBUG_MESSAGE( "STD Exception. %s", e.what() ); \
		XGC::STD_Exception_Call( e, __FILE__, __LINE__ ); \
		DumpStackFrame();\
	}\
	catch( ... ) \
	{\
		SYS_INFO( "Other Exception throw." ); \
		XGC_DEBUG_MESSAGE( "THR Exception. Unknowe" ); \
		XGC::ETC_Exception_Call( __FILE__, __LINE__ ); \
		DumpStackFrame();\
	}\

#define SEGMENT_BEGIN( TITLE, ... )		FUNCTION_BEGIN
#define SEGMENT_FINAL()					FUNCTION_END
#define SEGMENT_CATCH( ... )			FUNCTION_CATCH( __VA_ARGS__ )

///
/// @namespace XGC 
/// [6/11/2014] create by albert.xu
///
namespace XGC
{
	using common::sync_section;
	using common::autolock;
	using common::datetime;
	using common::timespan;


	struct COMMON_API InvokeWatcher;

	///
	/// ���ö�ջ�Ĺ�����
	/// [6/11/2014] create by albert.xu
	///
	class COMMON_API InvokeWatcherMgr
	{
		friend static unsigned int __stdcall CheckThread( LPVOID lpParams );
		friend COMMON_API InvokeWatcher* getInvokeWatcher();
	public:
		InvokeWatcherMgr();
		~InvokeWatcherMgr();

		///
		/// ������ʱ����߳� 
		/// [6/11/2014] create by albert.xu
		///
		xgc_bool Start();	// return true if start thread success

		///
		/// ֹͣ��ʱ����߳� 
		/// [6/11/2014] create by albert.xu
		///
		xgc_void Stop();

		///
		/// ��ʱ����߳��Ƿ��Ѿ����� 
		/// [6/11/2014] create by albert.xu
		///
		xgc_bool IsFinish();

	private:
		///
		/// ����һ�����Ӷ���ÿ�߳�һ����
		/// [12/4/2014] create by albert.xu
		///
		xgc_void InsertInvokeWatcher( InvokeWatcher* pWatcher );

		xgc_handle		mThread;
		InvokeWatcher	*pInvokeWatcherHead;
		sync_section	mSection;

		xgc_bool		mFinished;
	};

	extern COMMON_API InvokeWatcherMgr& getInvokeWatcherMgr();

	struct COMMON_API InvokeWatcherWarp;
	struct COMMON_API InvokeWatcher
	{
	private:
		InvokeWatcher( void );
		~InvokeWatcher( void );

		InvokeWatcher( const InvokeWatcher& ) = delete;
		InvokeWatcher& operator=( const InvokeWatcher& ) = delete;

		InvokeWatcher *Next() { return mNext; }
		void FroceWrite( xgc_lpcstr file, xgc_uint32 uDelaySeconds );

		friend class COMMON_API InvokeWatcherMgr;
		friend struct COMMON_API InvokeWatcherWarp;
		friend COMMON_API InvokeWatcher* getInvokeWatcher();
		friend static unsigned int __stdcall CheckThread( LPVOID lpParams );

		xgc_uint32 AddRef() { ++mRef; return mRef; }
		xgc_uint32 Release() { --mRef; return mRef; }
	public:
		void FunctionBegin( const char* function, int line );
		void FunctionEnd( xgc_time32 nTimeOut );

	private:
		struct CallFrame
		{
			xgc_lpcstr lpFileName;
			volatile xgc_uint64 nTime;
			xgc_uint32 nLine;
		}mStack[WATCH_STACKFRAME_MAX];


		datetime	mLastUpdate;
		xgc_uint32	mCallDeep;
		xgc_uint32	mRef;
		struct InvokeWatcher *mNext;
		xgc_bool mIsDirty;	// flag that shows data was saved to file before
		xgc_bool mIsClose;	// �Ƿ�رռ�¼, ���ڴ���Log������쳣���.
	};

	struct COMMON_API InvokeWatcherWarp
	{
		///
		/// �����Զ�����FunctionBegin
		/// [12/4/2014] create by albert.xu
		///
		InvokeWatcherWarp( InvokeWatcher* pWatcher, xgc_lpcstr pFile, xgc_uint32 nLine );

		///
		/// �����Զ�����FunctionEnd
		/// [12/4/2014] create by albert.xu
		///
		~InvokeWatcherWarp();

		/// @var �߳���صļ��Ӷ���
		InvokeWatcher* mWatcher;
	};

	COMMON_API InvokeWatcher* getInvokeWatcher();

	///
	/// �����飬���ں����������ƣ��ݹ���ȵļ��
	/// [11/20/2014] create by albert.xu
	///
	struct COMMON_API TwiceEnterGuard
	{
		TwiceEnterGuard() : mGuardValue( false )
		{

		}

		xgc_void Enter()
		{
			++mGuardValue;
		}

		xgc_void Return()
		{
			--mGuardValue;
		}

		xgc_uint32 mGuardValue;
	};

	struct COMMON_API TwiceEnterChecker
	{
		TwiceEnterChecker( xgc_lpcstr lpFunction, xgc_uint32 nLine, TwiceEnterGuard& R )
			: mGuard( R )
		{
			XGC_ASSERT_MESSAGE( mGuard.mGuardValue == 0, "%s:(%d) enter twice at once call checked.", lpFunction, nLine );
			mGuard.Enter();
		}

		TwiceEnterChecker( const TwiceEnterChecker& R )
			: mGuard( R.mGuard )
		{

		}

		~TwiceEnterChecker()
		{
			mGuard.Return();
		}

		TwiceEnterChecker& operator=( const TwiceEnterChecker& rhs ) = delete;

		TwiceEnterGuard &mGuard;
	};

	///
	/// ���ó�ʱ��־
	/// [12/3/2014] create by albert.xu
	///
	COMMON_API xgc_void SetDebugerLog( xgc_lpcstr pathname );

	///
	/// ���ó�ʱ��־
	/// [12/3/2014] create by albert.xu
	///
	COMMON_API xgc_void SetDebugerTimeout( xgc_time32 millseconds );

	///
	/// ��¼�ڴ�ʹ����Ϣ
	/// [1/21/2014 albert.xu]
	///
	COMMON_API xgc_lpcstr MemMark( xgc_lpcstr name, xgc_lpcstr parent = xgc_nullptr, xgc_int32( *Report )( xgc_lpcstr fmt, ... ) = xgc_nullptr );

	///
	/// �����Ѵ���ڴ�ʹ�����
	/// [1/21/2014 albert.xu]
	///
	COMMON_API xgc_void MemMarkReport( xgc_lpcstr name, xgc_int32( *Report )( xgc_lpcstr fmt, ... ) );

	///
	/// ����������
	/// [1/21/2014 albert.xu]
	///
	COMMON_API xgc_void MemMarkClear();

}
#endif // _DEBUGER_H_