// Filename		: ThreadPool.cpp
// Author		: Siddharth Barman
// Date			: 18 Sept 2005
// Description	: Implementation file for CThreadPool class. 
//------------------------------------------------------------------------------
#include "defines.h"
#include <exception>
#include "thread_pool.h"
#include "synchronous.h"

#include <map>
#include <list>
#include <process.h>
//------------------------------------------------------------------------------
/* Parameters	: pointer to a _threadData structure.
   Description	: This is the internal thread function which will run 
				  continuously till the Thread Pool is deleted. Any user thread
				  functions will run from within this function.
*/
namespace XGC
{
	namespace common
	{
		// info about all threads belonging to this pool will be stored in this map
		typedef std::map<xgc_uint32, CThreadPool::_ThreadData > ThreadMap;

		// all functions passed in by clients will be initially stored in this list.
		typedef std::list<CThreadPool::_FunctionData> FunctionList;

		struct CThreadPool::Impl
		{
			FunctionList	m_functionList;
			ThreadMap		m_threads;
			sync_section	m_section;
		};

		UINT __stdcall CThreadPool::_ThreadProc(xgc_lpvoid pParam)
		{
			xgc_uint32				dwWait;
			CThreadPool*			pool;
			pfnThreadProc			proc;
			xgc_lpvoid				data;
			xgc_uint32				thread_id = GetCurrentThreadId();
			xgc_handle				hWaits[2];
			IRunObject*				runObject;

			XGC_ASSERT(pParam != NULL);
			if(NULL == pParam)
			{
				return -1;
			}

			pool = static_cast<CThreadPool*>(pParam);
			hWaits[0] = pool->GetWaitHandle(thread_id);
			hWaits[1] = pool->GetShutdownHandle();

			loop_here:

			dwWait = WaitForMultipleObjects(2, hWaits, FALSE, INFINITE);

			if(dwWait - WAIT_OBJECT_0 == 0)
			{
				// a new function was added, go and get it
				pool->BusyNotify(thread_id);
				while(pool->GetThreadProc(thread_id, proc, &data, &runObject))
				{
					if(proc == NULL)
					{
						// a function object is being used instead of 
						// a function pointer.
						runObject->Run();

						// see if we need to free this object
						runObject->AutoDelete();
					}
					else
					{
						proc(data);
					}
				}
				pool->FinishNotify(thread_id); // tell the pool, i am now free

				goto loop_here;
			}

			return 0;
		}

		UINT __stdcall CThreadPool::_CheckThreadProc(xgc_lpvoid pParam)
		{
			CThreadPool* pThis = (CThreadPool*)pParam;

			while( WaitForSingleObject( pThis->m_hNotifyCheckdown, 1000 ) == WAIT_TIMEOUT )
			{
				if( pThis->m_dwThreadTimeOut == 0 )
					continue;

				autolock _lock( pThis->m_pImpl->m_section );
				ThreadMap::iterator iter = pThis->m_pImpl->m_threads.begin();
				while( iter != pThis->m_pImpl->m_threads.end() )
				{
					_ThreadData data = iter->second;
					if( !data.bFree && GetTickCount() - data.fLastUpdateTime > pThis->m_dwThreadTimeOut )
					{
						// 监测出超时线程
						pThis->m_pImpl->m_threads.erase( iter );

#pragma warning(suppress: 6258)
						TerminateThread( data.hThread, -1 );
						CloseHandle( data.hThread );

						xgc_uint32	dwThreadId;
						xgc_handle	hThread;

						UINT uThreadId;
						hThread = (xgc_handle)_beginthreadex(NULL, 0, CThreadPool::_ThreadProc, pThis, CREATE_SUSPENDED, (UINT*)&uThreadId);
						dwThreadId = uThreadId;
						XGC_ASSERT(NULL != hThread);

						if(hThread)
						{
							// add the entry to the map of threads
							data.bFree		= true;
							data.hThread	= hThread;
							data.uThreadId	= dwThreadId;

							// 下面这一句我也不想加,因为创建的时候已经设置为未通知状态了,本来不需要再Reset一次的。
							// 但是，如果没有下面这句WaitSignleObjects 就会不阻塞直接返回。[4/27/2007]
							ResetEvent( data.hWaitHandle );
							ResumeThread(hThread);

							pThis->m_pImpl->m_threads.insert(ThreadMap::value_type(dwThreadId, data));
						}
					}
					++iter;
				}
			}
			return 0;
		}

		//------------------------------------------------------------------------------
		/* Parameters	: Pool size, indicates the number of threads that will be 
						  available in the queue.
		*******************************************************************************/
		CThreadPool::CThreadPool( xgc_size nPoolSize /*= POOL_SIZE*/, xgc_size nMaxListCount /*= 100*/, xgc_bool bCreateNow /*= false*/ )
			: m_nMaxListCount( nMaxListCount )
			, m_dwTimeOut( 0 )
			, m_nActiveSize( 0 )
			, m_dwThreadTimeOut( 10000 )
			, m_nPoolSize( nPoolSize )
			, m_hCheckThread( INVALID_HANDLE_VALUE )
			, m_pImpl( XGC_NEW CThreadPool::Impl )
		{
			if(bCreateNow)
			{
				if(!Create())
				{
					throw 1;
				}
			}
		}
		//------------------------------------------------------------------------------

		/* Description	: Use this method to create the thread pool. The constructor of
						  this class by default will create the pool. Make sure you 
						  do not call this method without first calling the Destroy() 
						  method to release the existing pool.
		   Returns		: true if everything went fine else returns false.
		  *****************************************************************************/
		xgc_bool CThreadPool::Create()
		{
			xgc_handle	hThread;
			xgc_uint32	uThreadId;
			xgc_tchar	szEvtName[20];

			_ThreadData ThreadData;
			
			// create the event which will signal the threads to stop
			m_hNotifyShutdown	= CreateEvent(NULL, TRUE, FALSE, NULL);
			m_hNotifyCheckdown	= CreateEvent(NULL, TRUE, FALSE, NULL);
			XGC_ASSERT(m_hNotifyShutdown != NULL);
			if(!m_hNotifyShutdown)
			{
				return false;
			}

			// create the threads
			for( xgc_size nIndex = 0; nIndex < m_nPoolSize; nIndex++ )
			{
				sprintf_s(szEvtName, XGC_COUNTOF(szEvtName), "Thread%Iu", nIndex);
						
				hThread = (xgc_handle)_beginthreadex(NULL, 0, CThreadPool::_ThreadProc, this, CREATE_SUSPENDED, (UINT*)&uThreadId);

				XGC_ASSERT(NULL != hThread);
				
				if(hThread)
				{
					// add the entry to the map of threads
					ThreadData.bFree		= true;
					ThreadData.hWaitHandle	= CreateEvent(NULL, TRUE, FALSE, NULL);
					ThreadData.hThread		= hThread;
					ThreadData.uThreadId	= uThreadId;
				
					// 下面这一句我也不想加,因为创建的时候已经设置为未通知状态了,本来不需要再Reset一次的。
					// 但是，如果没有下面这句WaitSignleObjects 就会不阻塞直接返回。[4/27/2007]
					if( ThreadData.hWaitHandle != 0 )
						ResetEvent( ThreadData.hWaitHandle );

					m_pImpl->m_threads.insert(ThreadMap::value_type(uThreadId, ThreadData));		

					ResumeThread(hThread); 
				}
				else
				{
					return false;
				}
			}

			return true;
		}
		//------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------//
		xgc_bool CThreadPool::EnableUpdateCheck( xgc_bool bEnable )	// enable threads update checking
		{
			if( bEnable )
			{
				// 启动监控线程
				m_hCheckThread = (xgc_handle)_beginthreadex(NULL, 0, CThreadPool::_CheckThreadProc, this, 0, NULL);
				if( !m_hCheckThread ) return false;
			}
			else
			{
				SetEvent( m_hNotifyCheckdown );
				WaitForSingleObject( m_hCheckThread, 5000 );
				CloseHandle( m_hCheckThread );
				m_hCheckThread = NULL;
			}
			return true;
		}

		CThreadPool::~CThreadPool()
		{
			Destroy();
			SAFE_DELETE( m_pImpl );
		}
		//------------------------------------------------------------------------------

		/* Description	: Use this method to destory the thread pool. The destructor of
						  this class will destory the pool anyway. Make sure you 
						  this method before calling a Create() when an existing pool is 
						  already existing.
		   Returns		: xgc_void
		  *****************************************************************************/
		xgc_void CThreadPool::Destroy()
		{
			// tell all threads to shutdown.
			SetEvent(m_hNotifyShutdown);
			SetEvent(m_hNotifyCheckdown);

			// lets give the thread one second atleast to terminate
			Sleep(1000);
			
			ThreadMap::iterator iter;
			_ThreadData ThreadData;
			
			// walk through the events and threads and close them all
			for(iter = m_pImpl->m_threads.begin(); iter != m_pImpl->m_threads.end(); iter++)
			{
				ThreadData = (*iter).second;		
				CloseHandle(ThreadData.hWaitHandle);
				CloseHandle(ThreadData.hThread);
			}

			// close the shutdown event
			CloseHandle(m_hNotifyShutdown);
			CloseHandle(m_hNotifyCheckdown);
			CloseHandle(m_hCheckThread);

			// empty all collections
			FunctionList::iterator iterList = m_pImpl->m_functionList.begin();
			while( iterList != m_pImpl->m_functionList.end() )
			{
				if( (*iterList).mRunObject )
				{
					(*iterList).mRunObject->AutoDelete();
					(*iterList).mRunObject = NULL;
				}
				++iterList;
			}

			m_pImpl->m_functionList.clear();
			m_pImpl->m_threads.clear();
		}
		//------------------------------------------------------------------------------

		xgc_size CThreadPool::GetPoolSize()
		{
			return m_nPoolSize;
		}
		//------------------------------------------------------------------------------

		/* Parameters	: nSize - number of threads in the pool.   
		   ****************************************************************************/
		xgc_void CThreadPool::SetPoolSize(xgc_int32 nSize)
		{
			XGC_ASSERT(nSize > 0);
			if(nSize <= 0)
			{
				return;
			}

			m_nPoolSize = nSize;
		}

		xgc_int32	CThreadPool::GetWaitCount()const
		{
			return (xgc_int32)m_pImpl->m_functionList.size();
		}

		//--------------------------------------------------------------------------------------------------------//
		// 设置最大队列长度
		// nMax		:	队列最大值
		//--------------------------------------------------------------------------------------------------------//
		xgc_void CThreadPool::SetMaxWaitCount( size_t nMax )
		{
			m_nMaxListCount = nMax;
		}

		//--------------------------------------------------------------------------------------------------------//
		// 设置超时时间
		// dwSecond		:	超时时间
		//--------------------------------------------------------------------------------------------------------//
		xgc_void CThreadPool::SetTimeOut( xgc_uint32 dwSecond )
		{
			m_dwTimeOut = dwSecond;
		}

		//--------------------------------------------------------------------------------------------------------//
		// 设置线程超时时间
		// dwSecond		:	超时时间
		//--------------------------------------------------------------------------------------------------------//
		xgc_void CThreadPool::SetThreadTimeOut( xgc_uint32 dwSecond )
		{
			m_dwThreadTimeOut = dwSecond;
		}

		//--------------------------------------------------------------------------------------------------------//
		// 更新线程时间
		// dwThreadID	:	线程标示
		//--------------------------------------------------------------------------------------------------------//
		xgc_void CThreadPool::UpdateThreadTime()
		{
			xgc_uint32 dwThreadID = GetCurrentThreadId();
			autolock _lock( m_pImpl->m_section );
			ThreadMap::iterator iter = m_pImpl->m_threads.find( dwThreadID );
			if( iter != m_pImpl->m_threads.end() )
			{
				iter->second.fLastUpdateTime = GetTickCount();
			}
		}

		//------------------------------------------------------------------------------
		xgc_handle CThreadPool::GetShutdownHandle()
		{
			return m_hNotifyShutdown;
		}
		//------------------------------------------------------------------------------

		/* Parameters	: hThread - Handle of the thread that is invoking this function.
		   Return		: A ThreadProc function pointer. This function pointer will be 
						  executed by the actual calling thread.
						  NULL is returned if no functions list is empty.
																					  */
		xgc_bool CThreadPool::GetThreadProc(xgc_uint32 uThreadId, pfnThreadProc& Proc, xgc_lpvoid* Data, IRunObject** ppRunObject)
		{
			LPTHREAD_START_ROUTINE  lpResult = NULL;
			_FunctionData			FunctionData;
			FunctionList::iterator	iter;

			// get the first function info in the function list
			autolock lock(m_pImpl->m_section);
			
			// 抛弃所有超时的数据
			while( m_dwTimeOut && m_pImpl->m_functionList.size() )
			{
				_FunctionData& FunctionData = *m_pImpl->m_functionList.begin();
				if( GetTickCount() - FunctionData.fTimeStab < m_dwTimeOut )
				{
					break;
				}
				if( FunctionData.mRunObject && FunctionData.mRunObject->AutoDelete() )
				{
					delete FunctionData.mRunObject;
				}
				m_pImpl->m_functionList.pop_front();
			}

			iter = m_pImpl->m_functionList.begin();

			if(iter != m_pImpl->m_functionList.end())
			{
				FunctionData = (*iter);

				Proc = FunctionData.lpStartAddress;
				
				if(NULL == Proc) // is NULL for function objects
				{		
					*ppRunObject = static_cast<IRunObject*>(FunctionData.pData);
				}
				else		
				{
					// this is a function pointer
					*Data		= FunctionData.pData;
					ppRunObject	= NULL;
				}

				m_pImpl->m_functionList.pop_front(); // remove the function from the list

				return true;
			}
			else
			{
				return false;
			}
		}
		//------------------------------------------------------------------------------

		/* Parameters	: hThread - Handle of the thread that is invoking this function.
		   Description	: When ever a thread finishes executing the user function, it 
						  should notify the pool that it has finished executing.      
																					  */
		xgc_void CThreadPool::FinishNotify(xgc_uint32 dwThreadId)
		{
			ThreadMap::iterator iter;
			
			autolock lock(m_pImpl->m_section);
			iter = m_pImpl->m_threads.find(dwThreadId);

			if(iter == m_pImpl->m_threads.end())	// if search found no elements
			{			
				XGC_ASSERT(!_T("No matching thread found."));
				return;
			}
			else
			{	
				m_pImpl->m_threads[dwThreadId].bFree = true;

				if(!m_pImpl->m_functionList.empty())
				{
					// there are some more functions that need servicing, lets do that.
					// By not doing anything here we are letting the thread go back and
					// check the function list and pick up a function and execute it.
					return;
				}		
				else
				{
					// back to sleep, there is nothing that needs servicing.
					ResetEvent(m_pImpl->m_threads[dwThreadId].hWaitHandle);
				}
			}	
		}
		//------------------------------------------------------------------------------

		xgc_void CThreadPool::BusyNotify(xgc_uint32 dwThreadId)
		{
			ThreadMap::iterator iter;
			
			autolock lock(m_pImpl->m_section);

			iter = m_pImpl->m_threads.find(dwThreadId);

			if(iter == m_pImpl->m_threads.end())	// if search found no elements
			{
				XGC_ASSERT(!_T("No matching thread found."));
			}
			else
			{
				m_pImpl->m_threads[dwThreadId].bFree = false;
				m_pImpl->m_threads[dwThreadId].fLastUpdateTime = GetTickCount();
			}
		}
		//------------------------------------------------------------------------------

		/* Parameters	: pFunc - function pointer of type ThreadProc
						  pData - An xgc_lpvoid pointer
		   Decription	: This function is to be called by clients which want to make 
						  use of the thread pool.
		  *****************************************************************************/
		xgc_bool CThreadPool::Run(pfnThreadProc pFunc, xgc_lpvoid pData, ThreadPriority priority)
		{
			autolock lock(m_pImpl->m_section);
			if( m_nMaxListCount && m_pImpl->m_functionList.size() >= m_nMaxListCount )
			{
				return false;
			}

			_FunctionData funcdata;

			funcdata.lpStartAddress = pFunc;
			funcdata.pData			= pData;
			funcdata.fTimeStab		= GetTickCount();

			// add it to the list
			if(priority == Low)
			{
				m_pImpl->m_functionList.push_back(funcdata);
			}
			else
			{
				m_pImpl->m_functionList.push_front(funcdata);
			}

			// See if any threads are free
			ThreadMap::iterator iter;
			_ThreadData ThreadData;

			for(iter = m_pImpl->m_threads.begin(); iter != m_pImpl->m_threads.end(); iter++)
			{
				ThreadData = (*iter).second;
				
				if(ThreadData.bFree)
				{
					// here is a free thread, put it to work
					// m_pImpl->m_threads[ThreadData.uThreadId].bFree = false;
					// m_pImpl->m_threads[ThreadData.uThreadId].fLastUpdateTime = GetTickCount();
					SetEvent(ThreadData.hWaitHandle); 
					// this thread will now call GetThreadProc() and pick up the next
					// function in the list.
					break;
				}
			}
			return true;
		}
		//------------------------------------------------------------------------------

		/* Parameters	: runObject - Pointer to an instance of class which implements
									  IRunObject interface.
						  priority  - Low or high. Based on this the object will be
									  added to the front or back of the list.
			return		: true - function has been push to queue
						  false - queue was full;
		   Decription	: This function is to be called by clients which want to make 
						  use of the thread pool.
		  *****************************************************************************/
		xgc_bool CThreadPool::Run(IRunObject* runObject, ThreadPriority priority)
		{
			XGC_ASSERT(runObject != NULL);
			
			autolock lock(m_pImpl->m_section);
			if( m_nMaxListCount && m_pImpl->m_functionList.size() >= m_nMaxListCount )
			{
				return false;
			}

			_FunctionData funcdata;

			funcdata.lpStartAddress = NULL; // NULL indicates a function object is being
											// used instead.
			funcdata.pData			= runObject; // the function object

			funcdata.fTimeStab		= GetTickCount();
			// add it to the list
			if(priority == Low)
			{
				m_pImpl->m_functionList.push_back(funcdata);
			}
			else
			{
				m_pImpl->m_functionList.push_front(funcdata);
			}

			// See if any threads are free
			ThreadMap::iterator iter;
			_ThreadData ThreadData;

			for(iter = m_pImpl->m_threads.begin(); iter != m_pImpl->m_threads.end(); iter++)
			{
				ThreadData = (*iter).second;
				
				if(ThreadData.bFree)
				{
					// here is a free thread, put it to work
					// m_pImpl->m_threads[ThreadData.uThreadId].bFree = false;
					// m_pImpl->m_threads[ThreadData.uThreadId].fLastUpdateTime = GetTickCount();
					SetEvent(ThreadData.hWaitHandle); 
					// this thread will now call GetThreadProc() and pick up the next
					// function in the list.
					break;
				}
			}
			return true;
		}
		//------------------------------------------------------------------------------

		/* Parameters	: ThreadId - the id of the thread for which the wait handle is 
									 being requested.
		   Returns		: NULL if no mathcing thread id is present.
						  The xgc_handle which can be used by WaitForXXXObject API.
		  *****************************************************************************/
		xgc_handle CThreadPool::GetWaitHandle(xgc_uint32 uThreadId)
		{
			xgc_handle hWait;
			ThreadMap::iterator iter;
			
			autolock lock(m_pImpl->m_section);
			iter = m_pImpl->m_threads.find(uThreadId);
			
			if(iter == m_pImpl->m_threads.end())	// if search found no elements
			{
				return NULL;
			}
			else
			{
				XGC_ASSERT( iter->second.uThreadId == uThreadId );
				hWait = iter->second.hWaitHandle;
			}
			return hWait;
		}
		//------------------------------------------------------------------------------
	}
}
