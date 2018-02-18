#include "stdafx.h"
#include "message_handle.h"
#include "proto_def.h"
#include "manager.h"

extern fs::path g_loader_path;
extern fs::path g_module_path;

namespace xgc
{
	namespace message
	{
		network_t g_session = INVALID_NETWORK_HANDLE;

		timer_t g_timer_handle = INVALID_TIMER_HANDLE;
		xgc_bool is_disconnect()
		{
			return g_session == INVALID_NETWORK_HANDLE;
		}

		///
		/// \brief 初始化消息映射表
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:37
		///
		xgc_void init_deliver_table()
		{
			InstallDeliver( SYSTEM_MESSAGE_TYPE, EVENT_CONNECT, OnConnect );
			InstallDeliver( SYSTEM_MESSAGE_TYPE, EVENT_CLOSE, OnClose );
			InstallDeliver( SYSTEM_MESSAGE_TYPE, EVENT_ERROR, OnError );
			InstallDeliver( TYPE_AUTOCASE, ID_SERVER_FILELIST, OnFileList );
			InstallDeliver( TYPE_AUTOCASE, ID_SERVER_START_REQ, OnStart );
			InstallDeliver( TYPE_AUTOCASE, ID_SERVER_STOP_NTF, OnStop );
			InstallDeliver( TYPE_AUTOCASE, ID_CLIENT_NEW_FILE_NTF, OnNewFile );
			InstallDeliver( TYPE_AUTOCASE, ID_CLIENT_FILE_DATA_NTF, OnFileData );
		}

		xgc_void OnTimer()
		{
			ClientResNtf stNtf;
			stNtf.cpu = GetSystemCpuUsage();
			uint64_t usage, total;
			uint32_t load;
			GetSystemMemoryUsage( &usage, &total, &load );
			stNtf.mem = load;
			stNtf.pid = 0;
			stNtf.tid = 0;

			Send( g_session, stNtf );
		}

		///
		/// \brief 客户端建立连接
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnConnect( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			char szDateTime[64] = { 0 };

			datetime::now().to_string( szDateTime );
			printf( "%s - connect server successful!\n", szDateTime );

			xgc_char szBuffer[256] = { 0 };

			Param_QueryHandleInfo *pInfo = (Param_QueryHandleInfo *)szBuffer;
			pInfo->handle = session;
			pInfo->mask = NET_LOCAL_ADDRESS;
			
			if( 0 == ExecuteState( Operator_QueryHandleInfo, (xgc_uintptr) pInfo ) )
			{
				CloseLink( session );

				return;
			}

			g_session = session;
			ClientInfoRpt stRpt;

			Param_QueryHandleInfo::endpoint* local_addr = (Param_QueryHandleInfo::endpoint*)pInfo->data;
			stRpt.addr = local_addr->addr;
			stRpt.port = local_addr->port;
			stRpt.pid = GetProcessId( NULL );
			stRpt.tid = GetCurrentThreadId();
			Send( g_session, stRpt );

			g_timer_handle = get_timer().insert( std::bind( OnTimer ), datetime::now(), DURATION_FOREVER, "intv:1000" );
		}

		///
		/// \brief 客户端断开连接
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClose( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			g_session = INVALID_NETWORK_HANDLE;
			get_timer().remove( g_timer_handle );
			g_timer_handle = INVALID_TIMER_HANDLE;
		}

		///
		/// \brief 客户端连接发生错误
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnError( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			int error = *(int*) data;
			char szDateTime[64] = { 0 };
			switch( error )
			{
				case NET_ERROR_CONNECT:
				break;
				case NET_ERROR_CONNECT_TIMEOUT:
				{
					char szHandleInfo[sizeof( Param_QueryHandleInfo ) + 1024] = { 0 };
					Param_QueryHandleInfo *handle_info = (Param_QueryHandleInfo *) szHandleInfo;
					handle_info->handle = session;
					handle_info->mask = NET_REMOT_ADDRESS;

					datetime::now().to_string( szDateTime );
					printf( "%s - connect server failed!\n", szDateTime );
				}
				break;
			}
		}

		xgc_void OnFileList( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ServerFileList stNtf;
			serialization ar( (xgc_lpvoid)data, size, size );

			ar >> stNtf;

			bool update = false;

			fs::path pathname;
			switch( stNtf.id )
			{
				case 0: pathname = g_loader_path / "md5.list"; break;
				case 1: pathname = g_module_path / "md5.list"; break;
			}

			if( pathname.empty() )
				return;

			std::fstream fs;
			fs.open( pathname, std::ios_base::in );
			if( fs.is_open() )
			{
				char line[1024];
				char *next = nullptr;
				char *code = nullptr;
				char *path = nullptr;
				while( !fs.eof() )
				{
					fs.getline( line, _countof(line) );
					
					code = strtok_s( line, " -", &next );
					if( code == nullptr )
						continue;
					
					path = strtok_s( nullptr, " -", &next );
					if( path == nullptr )
						continue;

					auto it = stNtf.filelist.find( path );
					if( it == stNtf.filelist.end() )
					{
						// 删除本地文件
						fs::remove( pathname / path );
					}
					else
					{
						if( it->second != code )
						{
							// 更新
							ClientFilesReq stReq;
							stReq.root = stNtf.id;
							stReq.relative = path;

							Send( session, stReq );

							update = true;
						}

						stNtf.filelist.erase( it );
					}
				}
			}

			// 新增的文件
			for each( auto file in stNtf.filelist )
			{
				ClientFilesReq stReq;
				stReq.root = stNtf.id;
				stReq.relative = file.first;

				Send( session, stReq );

				update = true;
			}

			fs.close();

			if( update )
			{
				// 更新文件列表
				ClientFilesReq stReq;
				stReq.root = stNtf.id;
				stReq.relative = "md5.list";

				Send( session, stReq );
			}
		}

		///
		/// \brief 开始测试
		///
		/// \author albert.xu
		/// \date 2016/03/14 14:38
		///
		xgc_void OnStart( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ServerStartReq stReq;
			serialization ar( (xgc_lpvoid)data, size, size );

			ar >> stReq;
			get_case_manager().start( stReq.username, 
									  stReq.password, 
									  stReq.script,
									  stReq.loader_version, 
									  stReq.module_version,
									  stReq.address );
		}

		///
		/// \brief 停止测试
		///
		/// \author albert.xu
		/// \date 2016/03/14 14:38
		///
		xgc_void OnStop( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ServerStopNtf stNtf;
			serialization ar( (xgc_lpvoid)data, size, size );

			ar >> stNtf;
			get_case_manager().stop( stNtf.username );
		}

		static int fd = -1;
		static long received_bytes = 0;
		static long file_total_bytes = 0;
		///
		/// \brief 下载文件
		///
		/// \author albert.xu
		/// \date 2016/04/12 21:47
		///
		xgc_void OnNewFile( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ClientNewFileNtf stNtf;
			serialization ar( (xgc_lpvoid)data, size, size );

			ar >> stNtf;

			if( fd != -1 )
			{
				_close( fd );
				fd = -1;
			}

			ClientNewFileAck stAck;
			stAck.result = -1;

			// 根据路径标识来决定存储的根路径
			fs::path filepath;
			switch( stNtf.path_id )
			{
				case 0: // loader
				filepath = g_loader_path;
				break;
				case 1:
				filepath = g_module_path;
			}

			// 创建目录
			filepath /= (const char*) stNtf.buffer.base();
			if( !fs::exists( filepath.parent_path() ) )
				if( !fs::create_directories( filepath.parent_path() ) )
					return Send( session, stAck );

			// 创建文件
			auto filename = filepath.string();
			if( 0 == _sopen_s( &fd, filename.c_str(), _O_WRONLY | _O_BINARY | _O_TRUNC | _O_CREAT, _SH_DENYRW, S_IWRITE ) )
			{
				received_bytes = 0;
				file_total_bytes = stNtf.file_size;
				stAck.result = 0;

				std::cout << "create file : " << filename << std::endl;
			}

			Send( session, stAck );
		}

		///
		/// \brief 下载文件
		///
		/// \author albert.xu
		/// \date 2016/04/12 21:47
		///
		xgc_void OnFileData( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ClientFileDataNtf stNtf;
			serialization ar( (xgc_lpvoid)data, size, size );

			ar >> stNtf;

			ClientFileDataAck stAck;
			stAck.received = -1;
			if( fd != -1 )
			{
				stAck.received = -1;

				if( received_bytes != stNtf.offset )
				{
					std::cout << "file offset error" << received_bytes << std::endl;
					return Send( session, stAck );
				}

				xgc_uint32 bytes = 0;
				xgc_lpcstr data = (xgc_lpcstr) stNtf.buffer.base();
				xgc_uint32 size = (xgc_uint32) stNtf.buffer.size();

				// 尝试写五次
				for( auto i = 0; i < 5 && bytes < size; ++i )
				{
					int ret = _write( fd, data + bytes, size - bytes );
					if( ret == -1 )
					{
						std::cout << "file write error, retry " << i << " bytes " << bytes << std::endl;
						continue;
					}

					bytes += ret;
				}

				if( size == bytes )
				{
					stAck.received = 
						received_bytes = 
						received_bytes + size;
				}

				std::cout << "recevie " << received_bytes << " bytes" << std::endl;

				if( received_bytes == file_total_bytes )
				{
					_close( fd );
					fd = -1;

					std::cout << "file recevied complated!" << std::endl;
				}
			}

			Send( session, stAck );
		}
	}
}