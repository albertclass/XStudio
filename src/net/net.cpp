#include "Header.h"
#include "net.h"

namespace xgc
{
	namespace net
	{
		XGC_DECLSPEC_THREAD xgc_lpvoid packet_base = 0;

		//////////////////////////////////////////////////////////////////////////
		xgc_bool CreateNetwork( int workthreads )
		{
			getSocketMgr().Initialize();
			getNetwork().insert_workthread( workthreads );
			return true;
		}

		xgc_void DestroyNetwork()
		{
			getSocketMgr().Final( -1 );
			getNetwork().exit();
		}

		xgc_lpvoid StartServer( xgc_lpcstr address, xgc_uint16 port, server_options *options, SessionCreator creator )
		{
			if( false == IsValidSocketMgr() )
				return xgc_nullptr;

			asio_Server *pServer = XGC_NEW asio_Server( getNetwork().Ref(), creator, options );

			if( pServer )
				pServer->StartServer( address, port );

			return pServer;
		}

		xgc_void CloseServer( xgc_lpvoid server )
		{
			asio_Server *pServer = (asio_Server*) server;
			if( pServer )
			{
				pServer->StopServer();

				SAFE_DELETE( pServer );
			}
		}

		network_t Connect( xgc_lpcstr address, xgc_uint16 port, INetworkSession* session, connect_options *options )
		{
			if( false == IsValidSocketMgr() )
				return INVALID_NETWORK_HANDLE;

			asio_SocketPtr pSocket = std::make_shared< asio_Socket >( getNetwork().Ref(), 0, session, xgc_nullptr );

			if( xgc_nullptr == pSocket )
				return INVALID_NETWORK_HANDLE;

			LinkUp( pSocket );

			if ( pSocket->connect( address, port, options ) )
				return pSocket->get_handle();

			return INVALID_NETWORK_HANDLE;
		}

		//////////////////////////////////////////////////////////////////////////
		// 发送消息包
		xgc_void SendPacket( network_t handle, xgc_lpvoid data, xgc_size size )
		{
			asio_SocketPtr pSocket = getSocketMgr().getSocket( handle );

			if( pSocket )
			{
				pSocket->send( data, size );
			}
		}

		xgc_void SendPacketChains( network_t handle, const BufferChains& buffers )
		{
			asio_SocketPtr pSocket = getSocketMgr().getSocket( handle );

			if( pSocket )
			{
				pSocket->send( buffers );
			}
		}

		xgc_void SendPackets( network_t *handle, xgc_uint32 count, xgc_lpvoid data, xgc_size size )
		{
			for( xgc_uint32 i = 0; i < count; ++i )
			{
				SendPacket( handle[i], data, size );
			}
		}

		xgc_void SendToGroup( group_t group, xgc_lpvoid data, xgc_size size, xgc_bool toself )
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

		xgc_void CloseLink( network_t handle )
		{
			asio_SocketPtr pSocket = getSocketMgr().getSocket( handle );
			if( pSocket )
			{
				pSocket->close();
			}
		}

		///
		/// \brief 关闭连接
		///
		/// \author albert.xu
		/// \date 2017/05/24 15:06
		///
		xgc_void NewTimer( xgc_uint32 id, xgc_real64 period, xgc_real64 after, const std::function< void() > &on_timer )
		{
			getSocketMgr().NewTimer( id, period, after, on_timer );
		}

		///
		/// \brief 关闭连接
		///
		/// \author albert.xu
		/// \date 2017/05/24 15:06
		///
		xgc_void DelTimer( xgc_uint32 id )
		{
			getSocketMgr().DelTimer( id );
		}

		///
		/// \brief 处理网络事件
		///
		/// \author albert.xu
		/// \date 2017/03/02 13:54
		///
		xgc_long ProcessNetEvent( xgc_long step )
		{
			return getSocketMgr().Exec( step );
		}

		xgc_ulong ExecuteState( xgc_uint32 operate_code, xgc_lpvoid param )
		{
			XGC_ASSERT_RETURN( param, -1 );
			switch( operate_code )
			{
				case Operator_NewGroup:
				{
					Param_NewGroup* pParam = (Param_NewGroup*)param;
					return getSocketMgr().NewGroup( pParam->self_handle );
				}
				break;
				case Operator_EnterGroup:
				{
					Param_EnterGroup* pParam = (Param_EnterGroup*)param;
					return getSocketMgr().EnterGroup( pParam->group, pParam->handle );
				}
				break;
				case Operator_LeaveGroup:
				{
					Param_LeaveGroup* pParam = (Param_LeaveGroup*)param;
					return getSocketMgr().LeaveGroup( pParam->group, pParam->handle );
				}
				break;
				case Operator_RemoveGroup:
				{
					Param_RemoveGroup* pParam = (Param_RemoveGroup*)param;
					return getSocketMgr().RemoveGroup( pParam->group );
				}
				break;
				case Operator_GetSession:
				{
					Param_GetSession* pParam = (Param_GetSession*)param;
					asio_SocketPtr pSocket = getSocketMgr().getSocket( pParam->handle );
					if( pSocket )
					{
						pParam->session = (INetworkSession*)pSocket->get_userdata();
						return 0;
					}
					return -1;
				}
				break;
				case Operator_QueryHandleInfo:
				{
					Param_QueryHandleInfo* pParam = (Param_QueryHandleInfo*)param;

					asio_SocketPtr pSocket = getSocketMgr().getSocket( pParam->handle );
					if( pSocket )
					{
						return pSocket->get_socket_info( pParam->addr, pParam->port );
					}

					return -1;
				}
				break;
				case Operator_SetBufferSize:
				{
					Param_SetBufferSize* pParam = (Param_SetBufferSize*)param;
					set_send_buffer_size( pParam->send_buffer_size );
					set_recv_buffer_size( pParam->recv_buffer_size );

					return 0;
				}
				break;
				case Operator_SetPacketSize:
				{
					Param_SetPacketSize* pParam = (Param_SetPacketSize*)param;
					set_send_packet_size( pParam->send_packet_size );
					set_recv_packet_size( pParam->recv_packet_size );

					return 0;
				}
				break;
			}

			return -1;
		}
	}
}
