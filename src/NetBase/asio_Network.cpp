///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file asio_Network.cpp
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 网络API封装
///
///////////////////////////////////////////////////////////////
#include "asio_Header.h"

#include "ServerSession.h"
#include "ClientSession.h"

namespace xgc
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

		xgc_bool asio_InitNetwork( int workthreads )
		{
			getSocketMgr().Initialize();
			getNetwork().insert_workthread( workthreads );
			return true;
		}

		xgc_void asio_FiniNetwork()
		{
			getSocketMgr().Final( 10000 );
			getNetwork().exit();
		}

		xgc_uintptr asio_StartServer( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 ping_invent, xgc_uint16 timeout, xgc_uint16 acceptor_count, MessageQueuePtr &queue_ptr )
		{
			if( queue_ptr == xgc_nullptr )
			{
				queue_ptr = MessageQueuePtr( XGC_NEW CMessageQueue, std::mem_fun( &CMessageQueue::Release ) );
			}

			if( queue_ptr == xgc_nullptr )
				return 0;

			asio_ServerBase *server = XGC_NEW asio_Server( getNetwork().Ref(), protocal, acceptor_count, ping_invent, timeout, queue_ptr );
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

		xgc_void asio_CloseServer( xgc_uintptr server_h )
		{
			asio_ServerBase *server = (asio_ServerBase*) server_h;
			server->StopServer();

			SAFE_DELETE( server );
		}

		xgc_bool asio_ConnectServer( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 timeout, MessageQueuePtr &queue_ptr )
		{
			if( queue_ptr == xgc_nullptr )
			{
				queue_ptr = MessageQueuePtr( XGC_NEW CMessageQueue, std::mem_fun( &CMessageQueue::Release ) );
			}

			if( !Connect( getNetwork().Ref(), address, port, &protocal, queue_ptr, timeout ) )
			{
				return false;
			}

			return true;
		}

		xgc_bool asio_ConnectServerAsync( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 timeout, MessageQueuePtr &queue_ptr )
		{
			if( queue_ptr == xgc_nullptr )
			{
				queue_ptr = MessageQueuePtr( XGC_NEW CMessageQueue, std::mem_fun( &CMessageQueue::Release ) );
			}

			return ConnectAsync( getNetwork().Ref(), address, port, &protocal, queue_ptr, timeout );
		}

		//////////////////////////////////////////////////////////////////////////
		// 发送消息包
		xgc_void asio_SendPacket( network_t handle, xgc_lpvoid data, xgc_size size )
		{
			asio_SocketPtr pSocket = getSocketMgr().getSocket( handle );

			if( pSocket )
			{
				pSocket->send( data, size );
			}
		}

		xgc_void asio_SendLastPacket( network_t handle, xgc_lpvoid data, xgc_size size )
		{
			asio_SocketPtr pSocket = getSocketMgr().getSocket( handle );
			if( pSocket )
			{
				pSocket->send( data, size, true );
			}
		}

		xgc_void asio_SendPackets( network_t *handle, xgc_uint32 count, xgc_lpvoid data, xgc_size size )
		{
			for( xgc_uint32 i = 0; i < count; ++i )
			{
				asio_SocketPtr pSocket = getSocketMgr().getSocket( handle[i] );
				if( pSocket )
				{
					pSocket->send( data, size );
				}
			}
		}

		xgc_void asio_SendToGroup( group_t group, xgc_lpvoid data, xgc_size size, xgc_bool toself )
		{
			asio_SocketMgr::CSocketGroup* pGroup = getSocketMgr().FetchHandleGroup( group );
			if( pGroup )
			{
				if( toself )
				{
					asio_SocketPtr pSocket = getSocketMgr().getSocket( pGroup->network_ );
					if( pSocket )
					{
						pSocket->send( data, size );
					}
				}

				asio_SocketMgr::CSocketGroup::iterator i = pGroup->begin();
				while( i != pGroup->end() )
				{
					asio_SocketPtr pSocket = getSocketMgr().getSocket( *i );
					if( pSocket )
					{
						pSocket->send( data, size );
					}
					++i;
				}
				getSocketMgr().FreeHandleGroup( pGroup );
			}
		}

		xgc_void asio_CloseLink( network_t handle )
		{
			asio_SocketPtr pSocket = getSocketMgr().getSocket( handle );
			if( pSocket )
			{
				pSocket->close();
			}
		}

		xgc_uintptr asio_ExecuteState( xgc_uint32 operate_code, xgc_uintptr param )
		{
			XGC_ASSERT_RETURN( param, -1 );
			switch( operate_code )
			{
				case Operator_NewGroup:
				{
					Param_NewGroup* pParam = (Param_NewGroup*) param;
					return getSocketMgr().NewGroup( pParam->self_handle );
				}
				break;
				case Operator_EnterGroup:
				{
					Param_EnterGroup* pParam = (Param_EnterGroup*) param;
					return getSocketMgr().EnterGroup( pParam->group, pParam->handle );
				}
				break;
				case Operator_LeaveGroup:
				{
					Param_LeaveGroup* pParam = (Param_LeaveGroup*) param;
					return getSocketMgr().LeaveGroup( pParam->group, pParam->handle );
				}
				break;
				case Operator_RemoveGroup:
				{
					Param_RemoveGroup* pParam = (Param_RemoveGroup*) param;
					return getSocketMgr().RemoveGroup( pParam->group );
				}
				break;
				case Operator_GetUserdata:
				{
					Param_GetUserdata* pParam = (Param_GetUserdata*) param;
					asio_SocketPtr pSocket = getSocketMgr().getSocket( pParam->handle );
					if( pSocket )
					{
						pParam->userdata_ptr = pSocket->get_userdata();
						return 0;
					}
					return -1;
				}
				break;
				case Operator_SetUserdata:
				{
					Param_SetUserdata* pParam = (Param_SetUserdata*) param;
					asio_SocketPtr pSocket = getSocketMgr().getSocket( pParam->handle );
					if( pSocket )
					{
						pSocket->set_userdata( pParam->userdata_ptr );
						return 0;
					}
					return -1;
				}
				break;
				case Operator_SetTimeout:
				{
					Param_SetTimeout* pParam = (Param_SetTimeout*) param;
				}
				break;
				case Operator_QueryHandleInfo:
				{
					Param_QueryHandleInfo* pParam = (Param_QueryHandleInfo*) param;
					xgc_uintptr ret = -1;
					asio_SocketPtr pSocket = getSocketMgr().getSocket( pParam->handle );
					if( pSocket )
					{
						ret = pSocket->get_socket_info( pParam->mask, pParam->data );
					}

					return ret;
				}
				break;
				case Operator_SetBufferSize:
				{
					Param_SetBufferSize* pParam = (Param_SetBufferSize*) param;
					set_send_buffer_size( pParam->send_buffer_size );
					set_recv_buffer_size( pParam->recv_buffer_size );
				}
				break;
			}
			return 0;
		}

	}
}
