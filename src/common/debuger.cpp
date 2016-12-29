#include "debuger.h"
#include "logger.h"
#include "xsystem.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#if defined _LINUX
XGC_DECLSPEC_THREAD sigjmp_buf sigjmp_env;
#endif

namespace xgc
{
	static xgc_char gTimeLogPath[XGC_MAX_PATH] = { 0 };
	static xgc_time64 gTimeoutSeconds = 500;

	InvokeWatcherMgr::InvokeWatcherMgr()
		: mFinished( false )
		, mInvokeWatcherHead( xgc_nullptr )
	{
	}

	InvokeWatcherMgr::~InvokeWatcherMgr()
	{
		Stop();
		InvokeWatcher* pWatcher = mInvokeWatcherHead;
		while( pWatcher )
		{
			mInvokeWatcherHead = pWatcher->Next();
			SAFE_DELETE( pWatcher );

			pWatcher = mInvokeWatcherHead;
		}
	}

	xgc_bool InvokeWatcherMgr::Start()
	{
		if( !mFinished )
		{
			// 创建监控线程
			mThread = std::thread( std::bind( &InvokeWatcherMgr::checkThread, this ) );

			mFinished = mThread.native_handle() == 0;
		}

		return mFinished;
	}

	xgc_void InvokeWatcherMgr::Stop()
	{
		if( !mFinished )
		{
			mFinished = true;
			if( mThread.joinable() )
				mThread.join();
		}
	}

	xgc_bool InvokeWatcherMgr::IsFinish()
	{
		return mFinished;
	}

	xgc_void InvokeWatcherMgr::InsertInvokeWatcher( InvokeWatcher* pWatcher )
	{
		autolock lock( mSection );
		pWatcher->mNext = mInvokeWatcherHead;
		mInvokeWatcherHead = pWatcher;
	}

	static InvokeWatcherMgr* gpInvokeWatcherMgr = XGC_NEW InvokeWatcherMgr();
	static xgc_void DeleteInvokeWatcherMgr()
	{
		SAFE_DELETE( gpInvokeWatcherMgr );
	}

	static int gpInvokeWatcherMgrExit = atexit( &DeleteInvokeWatcherMgr );

	InvokeWatcherMgr& getInvokeWatcherMgr()
	{
		return *gpInvokeWatcherMgr;
	}

	xgc_void InvokeWatcherMgr::checkThread()
	{
		while( !IsFinish() )
		{
			{
				InvokeWatcherMgr::autolock lock( mSection );
				InvokeWatcher* pWatcher = (InvokeWatcher*) mInvokeWatcherHead;
				while( pWatcher )
				{
					pWatcher->FroceWrite( gTimeLogPath, 1 );
					pWatcher = pWatcher->Next();
				}
			}

			std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
		}
	}

	XGC_DECLSPEC_THREAD InvokeWatcher *gpInvokeWatcher = xgc_nullptr;

	InvokeWatcher* getInvokeWatcher()
	{
		if( getInvokeWatcherMgr().IsFinish() )
		{
			return xgc_nullptr;
		}

		if( gpInvokeWatcher == xgc_nullptr )
		{
			gpInvokeWatcher = XGC_NEW InvokeWatcher();
			getInvokeWatcherMgr().InsertInvokeWatcher( gpInvokeWatcher );
		}

		return gpInvokeWatcher;
	}

	InvokeWatcher::InvokeWatcher( void )
		: mLastUpdate( datetime::now() )
		, mCallDeep( 0 )
		, mRef( 0 )
		, mIsDirty( false )
	{
	}

	InvokeWatcher::~InvokeWatcher( void )
	{
	}

	void InvokeWatcher::FunctionBegin( const char* function, int line )
	{
		if( true == mIsClose )
			return;

		mLastUpdate = datetime::now();
		if( mCallDeep < XGC_COUNTOF( mStack ) )
		{
			mStack[mCallDeep].lpFileName = function;
			mStack[mCallDeep].nTime = mLastUpdate.to_ftime();
			mStack[mCallDeep].nLine = line;
			mIsDirty = true;
		}
		++mCallDeep;
	}

	void InvokeWatcher::FunctionEnd( xgc_time64 timeout )
	{
		if( true == mIsClose )
			return;

		if( mCallDeep > 0 )
		{
			mCallDeep --;
			if( mCallDeep < XGC_COUNTOF( mStack ) )
			{
				datetime pre = datetime::from_ftime( mStack[mCallDeep].nTime );
				datetime now = datetime::now();
				if( now > pre )
				{
					timespan spn = now - pre;
					if( spn.to_millisecnods() > (xgc_int64)timeout )
					{
						mIsClose = true;
						LOGEXT( mStack[mCallDeep].lpFileName, mStack[mCallDeep].nLine, LOGLVL_SYS_WARNING, "函数执行超时%I64u毫秒", spn.to_millisecnods() );
						mIsClose = false;
					}
				}
			}
		}
	}

	void InvokeWatcher::FroceWrite( xgc_lpcstr file, xgc_uint32 uDelaySeconds )
	{
		datetime now = datetime::now();
		auto deep = mCallDeep;
		// 超时则写日志
		if( deep && mIsDirty && timespan( now - mLastUpdate ).to_seconds() >= (xgc_int64) uDelaySeconds )
		{
			char str[128];
			now.to_string( str, sizeof( str ) );
			FILE * fp = xgc_nullptr;
			if( fopen_s( &fp, file, "a+" ) == 0 && fp )
			{
				fputs( "--------------------------------------------------------------\n", fp );
				fputs( "invoke watcher log\n", fp );
				fprintf( fp, "%s.%u timeout %lld millsecnods\n", str, now.to_systime().milliseconds, ( now - mLastUpdate ).to_millisecnods());
				fputs( "-----------------------------CALL STACK-----------------------\n", fp );

				xgc_size length = 0;
				for( xgc_uint32 i = 0; i < deep; ++i )
				{
					fprintf( fp, "%llu : %s(%u)\n",
						mStack[deep - i - 1].nTime,
						mStack[deep - i - 1].lpFileName,
						mStack[deep - i - 1].nLine );
				}

				fputs( "--------------------------------------------------------------\n\n", fp );
				fflush( fp );
				fclose( fp );
			}
			mIsDirty = false;
			mLastUpdate = now;
		}
	}

	InvokeWatcherWarp::InvokeWatcherWarp( InvokeWatcher* pWatcher, xgc_lpcstr pFile, xgc_uint32 nLine ) 
		: mWatcher( pWatcher )
	{
		if( mWatcher )
		{
			mWatcher->AddRef();
			mWatcher->FunctionBegin( pFile, nLine );
		}
	}

	InvokeWatcherWarp::~InvokeWatcherWarp()
	{
		if( mWatcher )
		{
			mWatcher->Release();
			mWatcher->FunctionEnd( gTimeoutSeconds );
		}
	}

	///
	/// 设置超时日志
	/// [12/3/2014] create by albert.xu
	///
	COMMON_API xgc_void SetDebugerLog( xgc_lpcstr pathname )
	{
		strcpy_s( gTimeLogPath, pathname );
	}

	///
	/// 设置超时日志
	/// [12/3/2014] create by albert.xu
	///
	COMMON_API xgc_void SetDebugerTimeout( xgc_time64 millseconds )
	{
		gTimeoutSeconds = millseconds;
	}

	struct MemStatus
	{
		xgc_size	index;	// 对象位置

		xgc_time32	time;	// 时间
		xgc_uint64	pmem;	// 物理内存
		xgc_uint64	vmem;	// 虚拟内存
		xgc_char	name[32];	// 名字

		xgc_size	parent;		// 父对象位置
		xgc_size	lastchild;	// 最后一个子对象的位置
	};

	struct MemStatusHead
	{
		xgc_size	alloced;	// 已分配内存
		xgc_size	current;	// 当前指向
		xgc_lpcstr	pname;		// 父对象名
		xgc_size	parent;		// 父对象位置
		MemStatus	status[1];	// MemStatus数组
	};

	XGC_DECLSPEC_THREAD xgc_lpvoid gpMemStatus = xgc_nullptr;
	xgc_lpcstr MemMark( xgc_lpcstr name, xgc_lpcstr parent/* = xgc_nullptr*/, xgc_int32( *Report )( xgc_lpcstr fmt, ... )/* = xgc_nullptr*/ )
	{
		if( gpMemStatus == xgc_nullptr )
		{
			// 未分配内存时
			gpMemStatus = malloc( sizeof(MemStatusHead) +sizeof(MemStatus) * 100 );
			MemStatusHead* pHead = (MemStatusHead*) gpMemStatus;
			pHead->alloced = 100;
			pHead->current = 0;
			pHead->pname = xgc_nullptr;
			pHead->parent = 0;
		}

		MemStatusHead* pHead = (MemStatusHead*) gpMemStatus;
		if( pHead->current == pHead->alloced )
		{
			// 内存不足
			xgc_lpvoid mem = realloc( gpMemStatus, sizeof(MemStatusHead) +sizeof(MemStatus) * ( pHead->alloced + 20 ) );
			if( mem == xgc_nullptr )
				return name;

			gpMemStatus = mem;
			pHead = (MemStatusHead*) gpMemStatus;
			pHead->alloced += 20;
		}

		// 获取信息
		MemStatus& mMem = pHead->status[pHead->current];
		mMem.index = pHead->current;

		get_process_memory_usage( xgc_nullptr, &mMem.pmem, &mMem.vmem );
		mMem.time = (xgc_time32) clock();
		strcpy_s( mMem.name, name ? name: "" );

		// 第一个,内存记录开始
		if( pHead->current == 0 || name == xgc_nullptr )
		{
			mMem.parent		= -1;
			mMem.lastchild	= 0;
		}
		// 设置了父对象名
		else
		{
			// 找到前一个
			MemStatus& mParent = pHead->status[pHead->parent];
			MemStatus& mLast = pHead->status[mParent.lastchild];

			if( parent ? strcmp( parent, mParent.name ) == 0 : parent == xgc_nullptr )
			{
				// 设置了父的名字,且与前一项父的名字相同,说明是同级的.
				mMem.parent		= pHead->parent;
				mMem.lastchild	= pHead->current; // 指向自己

				// 更新父对象最后的子对象坐标
				mParent.lastchild = pHead->current;
			}
			else if( strcmp( parent, mLast.name ) == 0 )
			{
				// 设置了父的名字,且与前一项相同,说明是前一项的子项
				pHead->pname = mLast.name;
				pHead->parent = mLast.index;

				mMem.parent = mLast.index;
				mMem.lastchild = pHead->current;	// 指向自己

				// 更新父对象最后的子对象坐标
				mLast.lastchild = mMem.index;
			}
			else
			{
				// 设置了父的名字, 则回溯, 查找到正确的父节点
				xgc_size pos = pHead->parent;

				// 搜索匹配的父对象
				while( pos < pHead->alloced )
				{
					MemStatus& mParent = pHead->status[pos];
					if( strcmp( parent, mParent.name ) == 0 )
					{
						pHead->pname			= mParent.name;
						pHead->parent       = mParent.index;

						mMem.parent         = pos;
						mMem.lastchild      = pHead->current;

						mParent.lastchild	= pHead->current;
						break;
					}
					pos = mParent.parent;
				}
			}
		}

		++pHead->current;
		return name;
	}

	static xgc_size _MemMarkReport( xgc_size root, xgc_string& prefix, xgc_int32( *Report )( xgc_lpcstr fmt, ... ) )
	{
		MemStatusHead* pMemHeader = (MemStatusHead*) gpMemStatus;

		xgc_size pos = root;
		MemStatus& mMemRoot = pMemHeader->status[pos++];
		while( pos <= mMemRoot.lastchild )
		{
			MemStatus& mMemCurrent = pMemHeader->status[pos];
			xgc_size compair = ( mMemCurrent.index == mMemCurrent.lastchild ? mMemCurrent.index : mMemCurrent.lastchild ) + 1;
			MemStatus& mMemCompair = pMemHeader->status[compair];
			if( mMemCurrent.index == mMemCompair.index )
			{
				Report( "%s#%-15s# PMEM = %6.2fM, VMEM = %6.2fM"
					, prefix.c_str()
					, mMemCurrent.name
					, mMemCurrent.pmem
					, mMemCurrent.vmem
					);
			}
			else
			{
				Report( "%s#%-20s# PASS %7.3f SECONDS, PMEM = %6.2fM, INC %6.2fM, VMEM = %6.2fM, INC %6.2fM"
					, prefix.c_str()
					, mMemCurrent.name
					, ( mMemCompair.time - mMemCurrent.time ) / 1000.0
					, mMemCompair.pmem / ( 1024 * 1024 * 1.0 )
					, ( (xgc_int64) mMemCompair.pmem - (xgc_int64) mMemCurrent.pmem ) / ( 1024 * 1024 * 1.0 )
					, mMemCompair.vmem / ( 1024 * 1024 * 1.0 )
					, ( (xgc_int64) mMemCompair.vmem - (xgc_int64) mMemCurrent.vmem ) / ( 1024 * 1024 * 1.0 )
					);
			}

			if( mMemCurrent.lastchild != mMemCurrent.index )
			{
				prefix.append( "  " );
				pos = _MemMarkReport( pos, prefix, Report );
				prefix.erase( prefix.length() - 2, 2 );
				continue;
			}
			else
			{
				pos += 1;
			}
		}

		return pos;
	}

	xgc_void MemMarkReport( xgc_lpcstr name, xgc_int32( *Report )( xgc_lpcstr fmt, ... ) )
	{
		MemMark( xgc_nullptr );
		MemStatusHead* pMemHeader = (MemStatusHead*) gpMemStatus;

		xgc_size pos = 0;
		while( name && pos < pMemHeader->current )
		{
			MemStatus& mMemCurrent = pMemHeader->status[pos];
			if( strcmp( mMemCurrent.name, name ) == 0 )
				break;

			++pos;
		}

		if( pos == pMemHeader->current )
			pos = 0;

		xgc_string prefix = "";
		_MemMarkReport( pos, prefix, Report );
	}

	xgc_void MemMarkClear()
	{
		free( gpMemStatus );
		gpMemStatus = xgc_nullptr;
	}
}
