#include "config.h"
#include "pipe_manager.h"
#include "pipe_session.h"

namespace net_module
{
	/// �ܵ���������Ϣ
	struct PipeHandlerInfo
	{
		/// �ܵ�����ID
		xgc_string network_id;

		/// �ܵ���Ϣ������
		PipeMsgHandler pipe_msg_handler;
		/// �ܵ��¼�������
		PipeEvtHandler pipe_evt_handler;

		/// �����׽�����Ϣ������
		SockMsgHandler sock_msg_handler;
		/// �����׽����¼�������
		SockEvtHandler sock_evt_handler;
	};

	/// �ܵ������ʶӳ���
	static std::unordered_map< NETWORK_ID, CPipeSession* >	mPipeMap;

	/// �ܵ�������ӳ���
	static xgc_list< PipeHandlerInfo >	mPipeHandlerInfos;

	xgc_bool InitPipeManager()
	{
		return true;
	}

	xgc_void FiniPipeManager()
	{
		for( auto & it : mPipeMap )
			SAFE_DELETE( it.second );

		mPipeMap.clear();
	}

	///
	/// \brief �����ܵ�����
	///
	/// \author albert.xu
	/// \date 2017/07/17 18:19
	///
	xgc_bool RegistPipeHandler( xgc_lpcstr lpNetworkId, PipeMsgHandler fnMsgHandler, PipeEvtHandler fnEvtHandler )
	{
		auto it = std::find_if( mPipeHandlerInfos.begin(), mPipeHandlerInfos.end(), [&lpNetworkId]( const PipeHandlerInfo &info )->bool{
			return string_match( info.network_id.c_str(), lpNetworkId, 0 );
		});

		if( it != mPipeHandlerInfos.end() )
		{
			(*it).pipe_msg_handler = fnMsgHandler;
			(*it).pipe_evt_handler = fnEvtHandler;
		}
		else
		{
			mPipeHandlerInfos.emplace_back( PipeHandlerInfo { lpNetworkId, fnMsgHandler, fnEvtHandler, xgc_nullptr, xgc_nullptr } );
		}
		return true;
	}

	///
	/// \brief �����ܵ�����
	///
	/// \author albert.xu
	/// \date 2017/07/17 18:19
	///
	xgc_void RegistSockHandler( xgc_lpcstr lpNetworkId, SockMsgHandler fnMsgHandler, SockEvtHandler fnEvtHandler )
	{
		auto it = std::find_if( mPipeHandlerInfos.begin(), mPipeHandlerInfos.end(), [&lpNetworkId]( const PipeHandlerInfo &info )->bool{
			return string_match( info.network_id.c_str(), lpNetworkId, 0 );
		});

		if( it != mPipeHandlerInfos.end() )
		{
			(*it).sock_msg_handler = fnMsgHandler;
			(*it).sock_evt_handler = fnEvtHandler;
		}
		else
		{
			mPipeHandlerInfos.emplace_back( PipeHandlerInfo { lpNetworkId, xgc_nullptr, xgc_nullptr, fnMsgHandler, fnEvtHandler } );
		}
	}

	xgc_bool PipeConnect( NETWORK_ID nID, xgc_lpcstr pAddr, xgc_uint16 nPort, xgc_uint32 nTimeout )
	{
		// �ܵ��Ƿ�������
		auto it1 = mPipeMap.find( nID );
		if( it1 != mPipeMap.end() )
			return true;

		// �ܵ��Ƿ���ע��
		char szNetworkId[32] = { 0 };
		_NetworkID2Str( nID, szNetworkId );

		// ��ѯ��ע��Ĺܵ�������
		auto it2 = std::find_if( mPipeHandlerInfos.begin(), mPipeHandlerInfos.end(), 
			[szNetworkId]( const PipeHandlerInfo &info )->bool{
			return string_match( info.network_id.c_str(), szNetworkId, 0 );
		});

		// ���ƥ�䣬δƥ�������������
		if( it2 == mPipeHandlerInfos.end() )
		{
			SYS_WARNING( "δƥ�䵽��ע��Ĺܵ���������" );
			return false;
		}

		// ���ɹܵ�����
		auto pPipe = XGC_NEW CPipeSession();

		pPipe->SetPipeHandler( it2->pipe_msg_handler, it2->pipe_evt_handler );
		pPipe->SetSockHandler( it2->sock_msg_handler, it2->sock_evt_handler );

		// ��ֹ�ظ�����
		mPipeMap[nID] = pPipe;

		xgc_uint16 nOptions = NET_CONNECT_OPTION_ASYNC | NET_CONNECT_OPTION_RECONNECT | NET_CONNECT_OPTION_TIMEOUT;
		net::Connect( pAddr, nPort, nOptions, nTimeout, pPipe );
		return true;
	}

	xgc_bool OnPipeConnect( NETWORK_ID nID, CPipeSession * pPipe )
	{
		// ����ע��Ĺܵ�
		auto it1 = mPipeMap.find( nID );
		XGC_ASSERT_RETURN( it1 == mPipeMap.end(), true );

		char szNetworkId[32] = { 0 };
		_NetworkID2Str( nID, szNetworkId );

		// ��ѯ��ע��Ĺܵ�������
		auto it2 = std::find_if( mPipeHandlerInfos.begin(), mPipeHandlerInfos.end(),
			[szNetworkId]( const PipeHandlerInfo &info )->bool{
			return string_match( info.network_id.c_str(), szNetworkId, 0 );
		} );

		// ���ƥ�䣬δƥ�������������
		if( it2 == mPipeHandlerInfos.end() )
		{
			SYS_WARNING( "δƥ�䵽��ע��Ĺܵ���������" );
			return false;
		}

		pPipe->SetPipeHandler( it2->pipe_msg_handler, it2->pipe_evt_handler );
		pPipe->SetSockHandler( it2->sock_msg_handler, it2->sock_evt_handler );

		// ��ֹ�ظ�����
		mPipeMap[nID] = pPipe;

		return true;
	}

	CPipeSession * GetPipe( NETWORK_ID nID )
	{
		auto it = mPipeMap.find( nID );
		if( it != mPipeMap.end() )
			return it->second;

		return xgc_nullptr;
	}
}