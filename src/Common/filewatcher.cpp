#include "filewatcher.h"
#include "xsystem.h"
#include "datetime.h"

#include <mutex>
#include <thread>

namespace xgc
{
	namespace common
	{
		#define MAX_BUFF_SIZE (1024*4)
		/// ���һ�δ���Ĵ���
		static xgc_ulong  last_error = 0;
		/// ��ɶ˿ھ��
		static HANDLE iocp_handle = xgc_nullptr;
		/// �ػ��߳��Ƿ����蹤��
		static xgc_bool hardwork = true;
		/// 线程锁
		static std::mutex thread_guard;
		/// �����߳�����
		static xgc_vector< std::thread > work_threads;

		///
		/// \brief 事件句柄
		///
		/// \author albert.xu
		/// \date 2016/08/10 16:27
		///
		struct filewatcher_handler
		{
			OVERLAPPED	ol;
			/// 文件句柄
			HANDLE fh;
			/// 事件掩码
			xgc_uint32 events;
			/// 路径地址
			xgc_char path[_MAX_PATH];
			/// 缓冲文件
			xgc_char file[_MAX_FNAME];
			/// 消息缓冲
			/// 注意，此处缓冲首地址一定要保证四字节对齐，否则无法收到通知消息
			xgc_char notify_infomation[MAX_BUFF_SIZE];

			/// 回调地址
			filewatcher_notifier invoke;

			xgc_ulong actions;
			/// 最后一次修改时间
			xgc_time64 lasttick;
			/// 是否监控整个目录树
			xgc_bool watch_subtree;
		};

		/// 所有的事件句柄都在这里保存
		static std::unordered_map< xgc_handle, filewatcher_handler* > event_handles;

		/// ACTION 映射表
		static xgc_ulong filewatcher_action_map[] = 
		{
			#if defined( WIN32 ) || defined( WIN64 )
			0,
			FILE_WATCHER_ACTION_ADDED,
			FILE_WATCHER_ACTION_REMOVED,
			FILE_WATCHER_ACTION_MODIFIED,
			FILE_WATCHER_ACTION_RENAMED_OLD_NAME,
			FILE_WATCHER_ACTION_RENAMED_NEW_NAME,
			#else
			#endif
		};

		xgc_bool init_filewatcher( xgc_ulong thread_count, xgc_ulong interval /*= 0xffffffff*/ )
		{
			if( !privilege( SE_BACKUP_NAME ) )
			{
				last_error = GetLastError();
				return false;
			}

			if( !privilege( SE_RESTORE_NAME ) )
			{
				last_error = GetLastError();
				return false;
			}

			if( !privilege( SE_CHANGE_NOTIFY_NAME ) )
			{
				last_error = GetLastError();
				return false;
			}

			iocp_handle = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );

			if( !iocp_handle )
			{
				last_error = GetLastError();
				return false;
			}

			for( xgc_ulong i = 0; i < thread_count; ++i )
				work_threads.push_back( std::thread( [interval](){ 
					while( hardwork ){
						do_filewatcher_notify( interval );
					}
			} ) );

			return true;
		}

		xgc_void fini_filewatcher()
		{
			hardwork = false;
			for( std::thread &t : work_threads )
				PostQueuedCompletionStatus( iocp_handle, 0, 0, nullptr );

			for( std::thread &t : work_threads )
				t.join();

			if( iocp_handle != INVALID_HANDLE_VALUE )
			{
				CloseHandle( (HANDLE) iocp_handle );
				iocp_handle = INVALID_HANDLE_VALUE;
			}
		}

		xgc_long add_filewatcher( xgc_lpcstr path, xgc_ulong filter, xgc_bool watch_subtree, const filewatcher_notifier &notify_fn, xgc_bool auto_merger/* = true*/ )
		{
			// Open handle to the directory to be monitored, note the FILE_FLAG_OVERLAPPED
			HANDLE hWatcher = CreateFile( path,
									FILE_LIST_DIRECTORY,
									FILE_SHARE_READ | FILE_SHARE_WRITE,
									NULL,
									OPEN_EXISTING,
									FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
									NULL );

			if( hWatcher == INVALID_HANDLE_VALUE )
			{
				last_error = GetLastError();
				return FILE_WATCHER_ERROROPENFILE;
			}

			// Associate directory handle with the IO completion port

			if( CreateIoCompletionPort( hWatcher, iocp_handle, (ULONG_PTR) hWatcher, 0 ) == NULL )
			{
				last_error = GetLastError();

				CloseHandle( hWatcher );
				return FILE_WATCHER_ERRORADDTOIOCP;
			}

			// Start monitoring for changes

			DWORD dwBytesReturned = 0;

			// Allocate notification buffers (will be filled by the system when a
			// notification occurs
			auto e = XGC_NEW filewatcher_handler;
			if( xgc_nullptr == e )
				return FILE_WATCHER_ERROROUTOFMEM;

			memset( e, 0, sizeof( *e ) );

			e->fh       = hWatcher;
			e->events   = 0;
			e->actions  = auto_merger ? 1 : 0;
			e->lasttick = 0;

			e->events = XGC_ADD_FLAGS( e->events, XGC_CHK_FLAGS( filter, FILE_WATCHER_NOTIFY_SECURITY ) ? FILE_NOTIFY_CHANGE_SECURITY : 0 );
			e->events = XGC_ADD_FLAGS( e->events, XGC_CHK_FLAGS( filter, FILE_WATCHER_NOTIFY_CREATION ) ? FILE_NOTIFY_CHANGE_CREATION : 0 );
			e->events = XGC_ADD_FLAGS( e->events, XGC_CHK_FLAGS( filter, FILE_WATCHER_NOTIFY_LASTACCESS ) ? FILE_NOTIFY_CHANGE_LAST_ACCESS : 0 );
			e->events = XGC_ADD_FLAGS( e->events, XGC_CHK_FLAGS( filter, FILE_WATCHER_NOTIFY_LASTWRITE ) ? FILE_NOTIFY_CHANGE_LAST_WRITE : 0 );
			e->events = XGC_ADD_FLAGS( e->events, XGC_CHK_FLAGS( filter, FILE_WATCHER_NOTIFY_SIZE ) ? FILE_NOTIFY_CHANGE_SIZE : 0 );
			e->events = XGC_ADD_FLAGS( e->events, XGC_CHK_FLAGS( filter, FILE_WATCHER_NOTIFY_ATTRIBUTES ) ? FILE_NOTIFY_CHANGE_ATTRIBUTES : 0 );
			e->events = XGC_ADD_FLAGS( e->events, XGC_CHK_FLAGS( filter, FILE_WATCHER_NOTIFY_DIRNAME ) ? FILE_NOTIFY_CHANGE_DIR_NAME : 0 );
			e->events = XGC_ADD_FLAGS( e->events, XGC_CHK_FLAGS( filter, FILE_WATCHER_NOTIFY_FILENAME ) ? FILE_NOTIFY_CHANGE_FILE_NAME : 0 );

			e->watch_subtree = watch_subtree;

			strcpy_s( e->path, path );

			e->invoke = notify_fn;

			if( !ReadDirectoryChangesW( hWatcher,
										e->notify_infomation,
										sizeof( e->notify_infomation ),
										e->watch_subtree,
										e->events,
										&dwBytesReturned,
										&e->ol,
										NULL ) )
			{
				last_error = GetLastError();

				CloseHandle( hWatcher );
				SAFE_DELETE( e );

				return FILE_WATCHER_ERRORREADDIR;
			}

			//success, add to directories vector

			std::unique_lock< std::mutex > lock( thread_guard );

			auto ib = event_handles.insert( std::make_pair( e->fh, e ) );

			return FILE_WATCHER_SUCCESS;
		}

		xgc_void del_filewatcher( xgc_lpcstr path )
		{
			std::unique_lock< std::mutex > locker( thread_guard );

			auto it = std::find_if( event_handles.begin(), event_handles.end(), [path]( const std::pair< xgc_handle, filewatcher_handler* > &o )->bool{
				return _stricmp( o.second->path, path ) == 0;
			} );

			if( it != event_handles.end() )
			{
				CancelIo( (HANDLE) it->first );
				CloseHandle( (HANDLE) it->first );

				SAFE_DELETE( it->second );

				event_handles.erase( it );
			}
		}

		xgc_long do_filewatcher_notify( xgc_ulong timeout )
		{
			DWORD		dwBytesXFered = 0;
			DWORD		dwTimeout = (DWORD) timeout;
			ULONG_PTR	ulKey = 0;
			OVERLAPPED*	pOl;

			if( !GetQueuedCompletionStatus( iocp_handle, &dwBytesXFered, &ulKey, &pOl, dwTimeout ) )
			{
				if( ( last_error = GetLastError() ) != WAIT_TIMEOUT )
					return FILE_WATCHER_ERRORDEQUE;

				std::unique_lock< std::mutex > lock( thread_guard );

				for( auto pair : event_handles )
				{
					auto e = pair.second;
					auto t = datetime::now().to_milliseconds();

					// the first bit mean enable auto merger
					if( e && e->invoke && e->actions > 1 && t - e->lasttick > 1000 )
					{
						for( int i = 1; i < 6; ++i )
						{
							if( XGC_CHKBIT( e->actions, i ) )
								e->invoke( e->path, e->file, i );
						}

						e->actions = 1;
					}
				}

				return FILE_WATCHER_NOCHANGE;
			}

			std::unique_lock< std::mutex > lock( thread_guard );

			auto it = event_handles.find( (xgc_handle) ulKey );
			if( it == event_handles.end() ) //not found
				return FILE_WATCHER_ERRORUNKNOWN;

			filewatcher_handler* e = it->second;

			FILE_NOTIFY_INFORMATION* info = (FILE_NOTIFY_INFORMATION*) e->notify_infomation;

			while( info )
			{
				info->FileName[info->FileNameLength / sizeof( WCHAR )] = 0;

				char file[_MAX_FNAME] = { 0 };
				size_t numberofconverted = 0;
				auto err = wcstombs_s( &numberofconverted, file, info->FileName, _TRUNCATE );

				// 文件名不一致了
				if( e->actions >= 1 && _stricmp( file, e->file ) != 0 )
				{
					for( int i = 1; i < 6; ++i )
					{
						if( XGC_CHKBIT( e->actions, i ) )
							e->invoke( e->path, e->file, i );
					}

					e->actions = 1;

					strcpy_s( e->file, file );
				}

				if( err == 0 )
				{
					if( XGC_CHKBIT( e->actions, 0 ) )
					{
						e->actions = XGC_SETBIT( e->actions, filewatcher_action_map[info->Action] );
					}
					else
					{
						e->invoke( e->path, file, filewatcher_action_map[info->Action] );
					}
				}

				if( info->NextEntryOffset == 0UL )
					break;

				info = (PFILE_NOTIFY_INFORMATION) ( (LPBYTE) info + info->NextEntryOffset );

				if( (char*) info > e->notify_infomation + sizeof( e->notify_infomation ) )
					break;
			}

			// Continue reading for changes

			DWORD dwBytesReturned = 0;

			if( !ReadDirectoryChangesW( (HANDLE) e->fh,
										e->notify_infomation,
										sizeof( e->notify_infomation ),
										e->watch_subtree,
										e->events,
										&dwBytesReturned,
										&e->ol,
										NULL ) )
			{
				last_error = GetLastError();

				CloseHandle( (HANDLE) e->fh );
				SAFE_DELETE( e );

				return FILE_WATCHER_ERRORREADDIR;
			}

			return FILE_WATCHER_SUCCESS;
		}
	}
}
