#include "asio_Network.h"
#include "asio_Header.h"

namespace XGC
{
	namespace net
	{
		asio_Network::asio_Network( void )
			: service_( XGC_NEW asio::io_service() )
		{
		}

		asio_Network::~asio_Network( void )
		{
		}

		xgc_void asio_Network::run()
		{
			io_service::work w( *service_ );
			service_->run();
		}

		xgc_bool asio_Network::insert_workthread( xgc_int32 insert_workthread )
		{
			for( int i = 0; i < insert_workthread; ++i )
			{
				workthreads_.push_back( std::shared_ptr<thread>( XGC_NEW asio::thread( std::bind( &asio_Network::run, this ) ) ) );
			}

			return true;
		}

		xgc_void asio_Network::exit()
		{
			service_->stop();

			for( size_t i = 0; i < workthreads_.size(); ++i )
			{
				workthreads_[i]->join();
			}

			SAFE_DELETE( service_ );
		}

		///////////////////////////////////////////////////////
		asio_Network& getNetwork()
		{
			static asio_Network asio_network;
			return asio_network;
		}

		//---------------------------------------------------//
		// [7/15/2009 Albert]
		// Description:	初始化网络
		//---------------------------------------------------//
		xgc_bool asio_InitNetwork( int workthreads )
		{
			getHandleManager().Initialize();
			getNetwork().insert_workthread( workthreads );
			return true;
		}

		//---------------------------------------------------//
		// [7/15/2009 Albert]
		// Description:	关闭网络
		//---------------------------------------------------//
		xgc_void asio_FiniNetwork()
		{
			getHandleManager().Final();
			getNetwork().exit();
		}

		//---------------------------------------------------//
		// [7/15/2009 Albert]
		// Description:	开启服务器
		// address:		服务器监听地址
		// port:		服务器监听端口
		// msg_queue:	服务器消息队列
		// return:		server 句柄，关闭服务器时需要提供相应句柄
		//---------------------------------------------------//
		xgc_uintptr asio_StartServer( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 ping_invent, xgc_uint16 timeout, xgc_uint16 acceptor_count, IMessageQueue** msg_queue_ptr )
		{
			if( msg_queue_ptr != xgc_nullptr && *msg_queue_ptr == xgc_nullptr )
			{
				*msg_queue_ptr = XGC_NEW CMessageQueue();
			}

			if( msg_queue_ptr == xgc_nullptr )
				return NULL;

			asio_ServerBase *server = XGC_NEW asio_Server( getNetwork().Ref(), protocal, acceptor_count, ping_invent, timeout, *msg_queue_ptr );
			if( server )
				server->StartServer( address, port );

			return (xgc_uintptr) server;
		}

		xgc_uintptr asio_StartServerEx( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 ping_invent, xgc_uint16 timeout, xgc_uint16 acceptor_count, create_handler_func call, xgc_uintptr param )
		{
			asio_ServerBase *server = XGC_NEW asio_ServerEx( getNetwork().Ref(), protocal, acceptor_count, ping_invent, timeout, call, param );
			if( server )
				server->StartServer( address, port );

			return (xgc_uintptr) server;
		}

		//---------------------------------------------------//
		// [7/15/2009 Albert]
		// Description:	关闭服务器
		//---------------------------------------------------//
		xgc_void asio_CloseServer( xgc_uintptr server_h )
		{
			asio_ServerBase *server = (asio_ServerBase*) server_h;
			server->StopServer();
		}

		xgc_bool asio_ConnectServer( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 timeout, IMessageQueue** msg_queue )
		{
			IMessageQueue* msg_queue_ptr = *msg_queue;
			if( msg_queue != xgc_nullptr && *msg_queue == xgc_nullptr )
			{
				msg_queue_ptr = XGC_NEW CMessageQueue();
			}

			if( !Connect( getNetwork().Ref(), address, port, &protocal, msg_queue_ptr, timeout ) )
			{
				return false;
			}

			*msg_queue = msg_queue_ptr;
			return true;
		}

		xgc_bool asio_ConnectServerAsync( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 timeout, IMessageQueue** msg_queue )
		{
			IMessageQueue* msg_queue_ptr = *msg_queue;
			if( msg_queue != xgc_nullptr && *msg_queue == xgc_nullptr )
			{
				msg_queue_ptr = *msg_queue = XGC_NEW CMessageQueue();
			}

			return ConnectAsync( getNetwork().Ref(), address, port, &protocal, *msg_queue, timeout );
		}

		//////////////////////////////////////////////////////////////////////////
		// 发送消息包
		xgc_void asio_SendPacket( network_t handle, const void *data, size_t size )
		{
			asio_NetworkHandler* pHandle = getHandleManager().FetchHandle( handle );

			if( pHandle )
			{
				pHandle->SendPacket( data, size );
				getHandleManager().FreeHandle( pHandle );
			}
		}

		xgc_void asio_SendLastPacket( network_t handle, const void* data, size_t size )
		{
			asio_NetworkHandler* pHandle = getHandleManager().FetchHandle( handle );
			if( pHandle )
			{
				pHandle->SendPacket( data, size, true );
				getHandleManager().FreeHandle( pHandle );
			}
		}

		xgc_void asio_SendPackets( network_t *handle, xgc_uint32 count, const void* data, size_t size )
		{
			for( xgc_uint32 i = 0; i < count; ++i )
			{
				asio_NetworkHandler* pHandle = getHandleManager().FetchHandle( handle[i] );
				if( pHandle )
				{
					pHandle->SendPacket( data, size );
					getHandleManager().FreeHandle( pHandle );
				}
			}
		}

		xgc_void asio_SendToGroup( group_t group, const void* data, size_t size, xgc_bool toself )
		{
			CHandlerManager< asio_NetworkHandler >::CHandleGroup* pGroup = getHandleManager().FetchHandleGroup( group );
			if( pGroup )
			{
				if( toself )
				{
					asio_NetworkHandler* pHandle = getHandleManager().FetchHandle( pGroup->network_ );
					if( pHandle )
					{
						pHandle->SendPacket( data, size );
						getHandleManager().FreeHandle( pHandle );
					}
				}

				CHandlerManager< asio_NetworkHandler >::CHandleGroup::iterator i = pGroup->begin();
				while( i != pGroup->end() )
				{
					asio_NetworkHandler* pHandle = getHandleManager().FetchHandle( *i );
					if( pHandle )
					{
						pHandle->SendPacket( data, size );
						getHandleManager().FreeHandle( pHandle );
					}
					++i;
				}
				getHandleManager().FreeHandleGroup( pGroup );
			}
		}

		xgc_void asio_CloseLink( network_t handle )
		{
			getHandleManager().CloseHandler( handle );
		}

		xgc_uintptr asio_ExecuteState( xgc_uint32 operate_code, xgc_uintptr param )
		{
			XGC_ASSERT_RETURN( param, -1 );
			switch( operate_code )
			{
			case Operator_NewGroup:
			{
				Param_NewGroup* pParam = (Param_NewGroup*) param;
				return getHandleManager().NewGroup( pParam->self_handle );
			}
			break;
			case Operator_EnterGroup:
			{
				Param_EnterGroup* pParam = (Param_EnterGroup*) param;
				return getHandleManager().EnterGroup( pParam->group, pParam->handle );
			}
			break;
			case Operator_LeaveGroup:
			{
				Param_LeaveGroup* pParam = (Param_LeaveGroup*) param;
				return getHandleManager().LeaveGroup( pParam->group, pParam->handle );
			}
			break;
			case Operator_RemoveGroup:
			{
				Param_RemoveGroup* pParam = (Param_RemoveGroup*) param;
				return getHandleManager().RemoveGroup( pParam->group );
			}
			break;
			case Operator_GetUserdata:
			{
				Param_GetUserdata* pParam = (Param_GetUserdata*) param;
				asio_NetworkHandler* pHandle = getHandleManager().FetchHandle( pParam->handle );
				if( pHandle )
				{
					pParam->userdata_ptr = pHandle->GetUserdata();
					getHandleManager().FreeHandle( pHandle );
					return 0;
				}
				return -1;
			}
			break;
			case Operator_SetUserdata:
			{
				Param_SetUserdata* pParam = (Param_SetUserdata*) param;
				asio_NetworkHandler* pHandle = getHandleManager().FetchHandle( pParam->handle );
				if( pHandle )
				{
					pHandle->SetUserdata( pParam->userdata_ptr );
					getHandleManager().FreeHandle( pHandle );
					return 0;
				}
				return -1;
			}
			break;
			case Operator_SetTimeout:
			{
				Param_SetTimeout* pParam = (Param_SetTimeout*) param;
				asio_NetworkHandler* pHandle = getHandleManager().FetchHandle( pParam->handle );
				if( pHandle )
				{
					getHandleManager().FreeHandle( pHandle );
				}
			}
			break;
			case Operator_QueryHandleInfo:
			{
				Param_QueryHandleInfo* pParam = (Param_QueryHandleInfo*) param;
				xgc_uintptr ret = -1;
				asio_NetworkHandler* pHandle = getHandleManager().FetchHandle( pParam->handle );
				if( pHandle )
				{
					ret = pHandle->GetHandleInfo( pParam->mask, pParam->data );
					getHandleManager().FreeHandle( pHandle );
				}

				return ret;
			}
			break;
			}
			return 0;
		}

	}
}
