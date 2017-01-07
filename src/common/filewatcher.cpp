#include "defines.h"
#include "exports.h"
#include <functional>
#include <mutex>
#include <thread>
#include <algorithm>

#include "filewatcher.h"
#include "xsystem.h"
#include "datetime.h"
#include "logger.h"

#if defined(_WINDOWS)
namespace xgc
{
	namespace common
	{
		#define MAX_BUFF_SIZE (1024*4)
		/// 最后一次错误的代码
		static xgc_ulong  last_error = 0;
		/// 完成端口句柄
		static HANDLE iocp_handle = xgc_nullptr;
		/// 守护线程是否仍需工作
		static xgc_bool hardwork = true;
		/// 线程互斥量
		static std::mutex thread_guard;
		/// 工作线程数组
		static xgc_vector< std::thread > work_threads;

		///
		/// \brief 事件句柄
		///
		/// \author albert.xu
		/// \date 2016/08/10 16:27
		///
		struct filewatch_handler
		{
			OVERLAPPED	ol;
			/// 文件句柄
			HANDLE fh;
			/// 事件掩码
			xgc_uint32 events;
			/// 监视路径
			xgc_char path[XGC_MAX_PATH];
			/// 缓冲文件
			xgc_char file[XGC_MAX_FNAME];
			/// 消息缓冲
			/// 注意，保证四字节对齐，否则无法收到通知消息
			xgc_char notify_infomation[MAX_BUFF_SIZE];

			/// 回调地址
			filewatch_notifier invoke;

			xgc_ulong actions;
			/// 最后一次修改时间
			xgc_time64 lasttick;
			/// 是否监控整个树目录
			xgc_bool watch_subtree;
		};

		/// 所有的事件句柄都在这里保存
		static std::unordered_map< xgc_handle, filewatch_handler* > event_handles;

		/// ACTION 映射表
		static xgc_ulong filewatcher_action_map[] =
		{
			0,
			FILE_WATCHER_ACTION_ADDED,
			FILE_WATCHER_ACTION_REMOVED,
			FILE_WATCHER_ACTION_MODIFIED,
			FILE_WATCHER_ACTION_RENAMED_OLD_NAME,
			FILE_WATCHER_ACTION_RENAMED_NEW_NAME,
			FILE_WATCHER_ACTION_COUNT,
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

		xgc_long add_filewatcher( xgc_lpcstr path, xgc_ulong filter, xgc_bool watch_subtree, const filewatch_notifier &notify_fn, xgc_bool auto_merger/* = true*/ )
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
			auto e = XGC_NEW filewatch_handler;
			if( xgc_nullptr == e )
				return FILE_WATCHER_ERROROUTOFMEM;

			memset( e, 0, sizeof( *e ) );

			e->fh       = hWatcher;
			e->events   = 0;
			e->actions  = auto_merger ? 0x80000000 : 0;
			e->lasttick = 0;

			e->events 	= filter;
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

			auto it = std::find_if( event_handles.begin(), event_handles.end(), [path]( const std::pair< xgc_handle, filewatch_handler* > &o )->bool{
				return strcasecmp( o.second->path, path ) == 0;
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
					if( e && e->invoke && XGC_CHK_FLAGS( e->actions, 0x80000000 ) && t - e->lasttick > 1000 )
					{
						e->invoke( e->path, e->file, e->actions );
						e->actions = 1;
					}
				}

				return FILE_WATCHER_NOCHANGE;
			}

			std::unique_lock< std::mutex > lock( thread_guard );

			auto it = event_handles.find( (xgc_handle) ulKey );
			if( it == event_handles.end() ) //not found
				return FILE_WATCHER_ERRORUNKNOWN;

			filewatch_handler* e = it->second;

			FILE_NOTIFY_INFORMATION* info = (FILE_NOTIFY_INFORMATION*) e->notify_infomation;

			while( info )
			{
				info->FileName[info->FileNameLength / sizeof( WCHAR )] = 0;

				char file[_MAX_FNAME] = { 0 };
				size_t numberofconverted = 0;
				auto err = wcstombs_s( &numberofconverted, file, info->FileName, _TRUNCATE );

				// 文件名不一致了
				if( XGC_CHK_FLAGS( e->actions, 0x80000000 ) && strcasecmp( file, e->file ) != 0 )
				{
					e->invoke( e->path, e->file, e->actions );
					e->actions = 1;

					strcpy_s( e->file, file );
				}

				if( err == 0 )
				{
					if( XGC_CHK_FLAGS( e->actions, 0x80000000 ) )
					{
						e->actions = XGC_SET_BIT( e->actions, info->Action );
					}
					else
					{
						e->invoke( e->path, file, XGC_BIT( info->Action ) );
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
#elif defined(_LINUX)
#include <sys/inotify.h>  
#include <sys/epoll.h>  
#include <sys/ioctl.h>  
namespace xgc
{
	namespace common
	{
		#define MAX_BUFF_SIZE (1024*4)

		/// notify fd
		static int inotify_fd = -1;
		/// epoll fd
		static int epoll_fd = -1;

		/// 线程互斥量
		static std::mutex thread_guard;
		/// 工作线程数组
		static xgc_vector< std::thread > work_threads;

		///
		/// \brief 事件句柄
		///
		/// \author albert.xu
		/// \date 2016/08/10 16:27
		///
		struct filewatch_handler
		{
			/// 文件句柄
			int fd;
			/// 事件掩码
			xgc_uint32 events;
			/// 监视路径
			xgc_char path[XGC_MAX_PATH];
			/// 缓冲文件
			xgc_char file[XGC_MAX_FNAME];
			/// 回调地址
			filewatch_notifier invoke;

			xgc_ulong actions;
			/// 最后一次修改时间
			xgc_time64 lasttick;
			/// 是否监控整个树目录
			xgc_bool watch_subtree;
		};

		/// 所有的事件句柄都在这里保存
		static std::unordered_map< int, filewatch_handler* > event_handles;

		///
		/// \brief 初始化文件监视器
		///
		/// \prarm thread_count 线程数量
		/// \param thread_interval 线程等待间隔，线程数为0时无效
		///
		/// \author albert.xu
		/// \date 2016/08/10 15:45
		///
		xgc_bool init_filewatcher( xgc_ulong thread_count, xgc_ulong thread_interval /*= 0xffffffff*/ )
		{
			inotify_fd = inotify_init();

			if( inotify_fd < 0 )
			{
				SYS_ERROR( "inotify_init error!" );
				return false;
			}

			epoll_fd = epoll_create(1024);
			if( epoll_fd < 0 )
			{
				SYS_ERROR( "epoll_create error!" );
				return false;
			}

			// set inotify fd to no block
			int nb = 1;  
    		ioctl( inotify_fd, FIONBIO, &nb );

			// associate epoll and inotify
			struct epoll_event evt;  
			evt.events = EPOLLIN|EPOLLET;  
			evt.data.fd = inotify_fd;

			int rs = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inotify_fd, &evt);
			if( rs < 0 )
			{
				SYS_ERROR( "Could not add Inotify event in EPOLL %d", rs );
				return false;
			}

			return true;
		}

		///
		/// \brief 清理文件监视器
		///
		/// \author albert.xu
		/// \date 2016/08/10 15:45
		///
		xgc_void fini_filewatcher()
		{
			if( inotify_fd > 0 )
			{
				close( inotify_fd );
				inotify_fd = -1;
			}

			if( epoll_fd > 0 )
			{
				close( epoll_fd );
				epoll_fd = -1;
			}
		}

		///
		/// \brief 新增一个文件监视器
		///
		/// \author albert.xu
		/// \date 2016/08/10 15:46
		///
		xgc_long add_filewatcher( xgc_lpcstr path, xgc_ulong filter, xgc_bool watch_subtree, const filewatch_notifier &notify_fn, xgc_bool auto_merger /*= true*/ )
		{
			filewatch_handler* e = XGC_NEW filewatch_handler;

			if( xgc_nullptr == e )
				return FILE_WATCHER_ERROROUTOFMEM;

			memset( e, 0, sizeof( *e ) );

			e->fd       = -1;
			e->events   = 0;
			e->actions  = auto_merger ? 0x80000000 : 0;
			e->lasttick = 0;

			e->events = filter;

			e->watch_subtree = watch_subtree;

			strcpy_s( e->path, path );

			e->invoke = notify_fn;

			e->fd = inotify_add_watch( inotify_fd, path, filter );
			if( e->fd < 0 )
			{
				SAFE_DELETE( e );
				return FILE_WATCHER_ERRORREADDIR;
			}

			std::unique_lock< std::mutex > lock( thread_guard );
			event_handles.insert( std::make_pair( e->fd, e ) );

			return FILE_WATCHER_SUCCESS;
		}

		///
		/// \brief 删除一个文件监视器
		///
		/// \author albert.xu
		/// \date 2016/08/10 15:49
		///
		xgc_void del_filewatcher( xgc_lpcstr path )
		{
			std::unique_lock< std::mutex > locker( thread_guard );

			auto it = std::find_if( event_handles.begin(), event_handles.end(), [path]( const std::pair< int, filewatch_handler* > &o )->bool{
				return strcasecmp( o.second->path, path ) == 0;
			} );

			if( it != event_handles.end() )
			{
				auto e = it->second;
				if( -1 == inotify_rm_watch( inotify_fd, e->fd ) )
				{
					SYS_ERROR( "inotify_rm_watch error." );
					return;
				}

				close( e->fd );

				SAFE_DELETE( e );

				event_handles.erase( it );
			}
		}

		///
		/// \brief 处理已上报的通知
		///
		/// \author albert.xu
		/// \date 2016/08/10 16:37
		///
		xgc_long do_filewatcher_notify( xgc_ulong timeout /*= 0*/ )
		{
			struct epoll_event evts[10];
			int n = epoll_wait( epoll_fd, evts, XGC_COUNTOF(evts), timeout );
			if( n < 0 )
			{
				SYS_ERROR( "epoll_wait error." );
				return FILE_WATCHER_ERRORDEQUE;
			}

			if( n == 0 )
			{
				// timeout
				std::unique_lock< std::mutex > lock( thread_guard );

				for( auto pair : event_handles )
				{
					auto e = pair.second;
					auto t = datetime::now().to_milliseconds();

					// the first bit mean enable auto merger
					if( e && e->invoke && XGC_CHK_FLAGS(e->actions, 0x80000000) && t - e->lasttick > 1000 )
					{
						e->invoke( e->path, e->file, e->actions );
						e->actions = 0x80000000;
					}
				}

				return FILE_WATCHER_NOCHANGE;
			}

			for( int i = 0; i < n; ++i )
			{
				// check it is inotify event.
				if( evts[i].data.fd != inotify_fd )
					continue;

				if( evts[i].events & (EPOLLERR|EPOLLHUP) )
					continue;
				
				if( evts[i].events & EPOLLIN )
				{
					/// 消息缓冲
					char inotify_infomation[MAX_BUFF_SIZE];  
                    memset( inotify_infomation, 0, sizeof(inotify_infomation) );

					int total = read( evts[i].data.fd, inotify_infomation, sizeof(inotify_infomation) );
					int bytes = 0;
					while( total > bytes )
					{
						inotify_event *ievt = (inotify_event *)( inotify_infomation + bytes );

						std::unique_lock< std::mutex > lock( thread_guard );

						auto it = event_handles.find( ievt->wd );
						if( it != event_handles.end() ) //found watch fd
						{
							filewatch_handler* e = it->second;

							if( XGC_CHK_FLAGS( e->actions, 0x80000000 ) && strcasecmp( ievt->name, e->file ) != 0 )
							{
								// file name changed auto merge end.
								e->invoke( e->path, e->file, e->actions );
								e->actions = 0x80000000;

								strcpy_s( e->file, ievt->name );
							}
							
							if( XGC_CHK_FLAGS( e->actions, 0x80000000 ) )
							{
								e->actions = XGC_ADD_FLAGS( e->actions, ievt->mask );
							}
							else
							{
								e->invoke( e->path, ievt->name, ievt->mask );
							}
						}
						
						bytes += sizeof(inotify_event) + ievt->len;
					}
				}
			}
		}
	}
}
#endif // _WINDOWS