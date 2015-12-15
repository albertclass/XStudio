#ifndef THREAD_POOL_CLASS
#define THREAD_POOL_CLASS
#pragma warning( disable : 4786) // remove irritating STL warnings
#include "defines.h"
#include "exports.h"

#define POOL_SIZE			10
#define SHUTDOWN_EVT_NAME	_T("PoolEventShutdown")
#define THREADPOOL			_T("THREADPOOL")

namespace XGC
{
	namespace common
	{
		struct COMMON_API IRunObject
		{
			virtual ~IRunObject(){};
			virtual xgc_void Run() = 0;
			virtual xgc_bool AutoDelete() = 0;
		};

		typedef unsigned int (__stdcall *pfnThreadProc)( xgc_lpvoid );

		class COMMON_API CThreadPool
		{
		public:
			enum ThreadPriority{ High,	Low	};
			// info about functions which require servicing will be saved using this struct.
			typedef struct tagFunctionData
			{
				pfnThreadProc lpStartAddress;
				union 
				{
					IRunObject* mRunObject;
					xgc_lpvoid	pData;
				};	
				xgc_real64	fTimeStab;
			} _FunctionData;

			// // info about threads in the pool will be saved using this struct.
			typedef struct tagThreadData
			{
				xgc_bool	bFree;
				xgc_handle	hWaitHandle;
				xgc_handle	hThread;
				xgc_uint32	uThreadId;
				xgc_real64	fLastUpdateTime;
			} _ThreadData;

			struct Impl;

			Impl	*m_pImpl;
			// this decides whether a function is added to the front or back of the list.
		private:
			static xgc_uint32 __stdcall _ThreadProc(xgc_lpvoid pParam);
			static xgc_uint32 __stdcall _CheckThreadProc(xgc_lpvoid pParam);

			xgc_handle	m_hCheckThread;		// 监控线程句柄
			xgc_size	m_nPoolSize;
			xgc_size	m_nActiveSize;		// Active thread size
			xgc_handle	m_hNotifyShutdown;	// notifies threads that a new function 
			// is added
			xgc_handle	m_hNotifyCheckdown;
			xgc_size	m_nMaxListCount;	// max list count
			xgc_uint32	m_dwTimeOut;		// time out setting. using second
			xgc_uint32	m_dwThreadTimeOut;	// thread time out;

			xgc_bool	GetThreadProc(xgc_uint32 uThreadId, pfnThreadProc&, xgc_lpvoid*, IRunObject**); 

			xgc_void	FinishNotify(xgc_uint32 uThreadId);
			xgc_void	BusyNotify(xgc_uint32 uThreadId);

			xgc_handle	GetWaitHandle(xgc_uint32 uThreadId);
			xgc_handle	GetShutdownHandle();

		public:
			CThreadPool( xgc_size nPoolSize = POOL_SIZE, xgc_size nMaxListCount = 100, xgc_bool bCreateNow = false );
			~CThreadPool();

			xgc_bool	Create();	// creates the thread pool
			xgc_void	Destroy();	// destroy the thread pool
			xgc_bool	EnableUpdateCheck( xgc_bool bEnable );	// enable threads update checking

			xgc_size	GetPoolSize();
			xgc_void	SetPoolSize(xgc_int32);

			//--------------------------------------------------------------------------------------------------------//
			// 得到队列中等待处理的业务个数
			//--------------------------------------------------------------------------------------------------------//
			xgc_int32	GetWaitCount()const;

			//--------------------------------------------------------------------------------------------------------//
			// 设置最大队列长度
			// nMax		:	队列最大值
			//--------------------------------------------------------------------------------------------------------//
			xgc_void	SetMaxWaitCount( size_t nMax );

			//--------------------------------------------------------------------------------------------------------//
			// 设置超时时间
			// dwSecond		:	超时时间
			//--------------------------------------------------------------------------------------------------------//
			xgc_void	SetTimeOut( xgc_uint32 dwSecond );

			//--------------------------------------------------------------------------------------------------------//
			// 设置线程超时时间
			// dwSecond		:	超时时间
			//--------------------------------------------------------------------------------------------------------//
			xgc_void	SetThreadTimeOut( xgc_uint32 dwSecond );

			//--------------------------------------------------------------------------------------------------------//
			// 更新调用该函数的线程的线程时间
			//--------------------------------------------------------------------------------------------------------//
			xgc_void	UpdateThreadTime();

			xgc_bool	Run(pfnThreadProc pFunc, xgc_lpvoid pData, ThreadPriority priority = Low);

			xgc_bool	Run(IRunObject*, ThreadPriority priority = Low);
		};
		//------------------------------------------------------------------------------
	}
}
#endif
