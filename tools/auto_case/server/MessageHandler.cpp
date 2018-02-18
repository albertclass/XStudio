#include "stdafx.h"
#include "MessageHandler.h"
#include "proto_def.h"
#include "ClientDataViewModel.h"
#pragma comment( lib, "ws2_32.lib" )
namespace xgc
{
	namespace message
	{
		///
		/// \brief 初始化消息映射表
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:37
		///
		xgc_void InitProcessor()
		{
			getConfig().GenerateFileist();
			InstallDeliver( SYSTEM_MESSAGE_TYPE, EVENT_ACCEPT, OnAccept );
			InstallDeliver( SYSTEM_MESSAGE_TYPE, EVENT_CLOSE, OnClose );
			InstallDeliver( SYSTEM_MESSAGE_TYPE, EVENT_ERROR, OnError );

			InstallDeliver( TYPE_AUTOCASE, ID_CLIENT_INFO_RPT, OnClientInfoRpt );
			InstallDeliver( TYPE_AUTOCASE, ID_SERVER_START_ACK, OnClientStart );
			InstallDeliver( TYPE_AUTOCASE, ID_CLIENT_RES_NTF, OnClientResNtf );
			InstallDeliver( TYPE_AUTOCASE, ID_CLIENT_STATUS_NTF, OnClientStatusNtf );
			InstallDeliver( TYPE_AUTOCASE, ID_CLIENT_PROGRESS_NTF, OnProgressNtf );
			InstallDeliver( TYPE_AUTOCASE, ID_CLIENT_FILES_REQ, OnFilesReq );
			InstallDeliver( TYPE_AUTOCASE, ID_CLIENT_NEW_FILE_ACK, OnNewFileAck );
			InstallDeliver( TYPE_AUTOCASE, ID_CLIENT_FILE_DATA_ACK, OnFileDataAck );
		}

		///
		/// \brief 客户端建立连接
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnAccept( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{

		}

		///
		/// \brief 客户端断开连接
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClose( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{

		}

		///
		/// \brief 客户端连接发生错误
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnError( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{

		}

		///
		/// \brief 客户端定时同步
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClientInfoRpt( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ClientInfoRpt stRpt;
			serialization ar( (xgc_lpvoid)data, size, size );

			ar >> stRpt;

			in_addr addr;
			addr.S_un.S_addr = ntohl( stRpt.addr );
			wxString strAddress = inet_ntoa( addr );

			ClientDataNode* pNode = XGC_NEW ClientDataNode( session, strAddress, stRpt.pid, stRpt.tid );

			gDataViewModel.InsertNode( pNode );

			ServerFileList stAck;
			stAck.id = 0;
			stAck.filelist = getConfig().GetFilelist( 0 );

			Send( session, stAck );

			stAck.id = 1;
			stAck.filelist = getConfig().GetFilelist( 1 );

			Send( session, stAck );
		}

		xgc_void OnClientStart( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ServerStartAck stAck;
			serialization ar( (xgc_lpvoid)data, size, size );

			ar >> stAck;

			auto pNode = gDataViewModel.GetNode( session );
			wxASSERT( pNode );
			if( nullptr == pNode )
				return;

			auto pData = pNode->GetClientData( stAck.username );
			wxASSERT( pData );
			if( nullptr == pData )
				return;

			pData->pid = stAck.pid;
			pData->tid = stAck.tid;

			gDataViewModel.ItemChanged( wxDataViewItem( pData ) );
		}

		///
		/// \brief 同步任务的资源使用情况
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClientResNtf( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ClientResNtf stNtf;
			serialization ar( (xgc_lpvoid) data, size, size );

			ar >> stNtf;
			auto pNode = gDataViewModel.GetNode( session );
			wxASSERT( pNode );
			if( nullptr == pNode )
				return;

			if( 0 == stNtf.pid && 0 == stNtf.tid )
			{
				pNode->cpu = stNtf.cpu;
				pNode->mem = stNtf.mem;
				gDataViewModel.ItemChanged( wxDataViewItem( pNode ) );
			}
			else
			{
				auto pData = pNode->GetClientData( stNtf.pid, stNtf.tid );
				wxASSERT( pData );
				if( nullptr == pData )
					return;

				pData->cpu = stNtf.cpu;
				pData->mem = stNtf.mem;
				gDataViewModel.ItemChanged( wxDataViewItem( pData ) );
			}
		}

		///
		/// \brief 同步任务状态
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClientStatusNtf( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ClientStatusNtf stNtf;
			serialization ar( (xgc_lpvoid) data, size, size );

			ar >> stNtf;
			auto pNode = gDataViewModel.GetNode( session );
			wxASSERT( pNode );
			if( nullptr == pNode )
				return;
			if( 0 == stNtf.pid && 0 == stNtf.tid )
			{
				pNode->status = stNtf.status;
				gDataViewModel.ItemChanged( wxDataViewItem( pNode ) );
			}
			else
			{
				auto pData = pNode->GetClientData( stNtf.pid, stNtf.tid );
				wxASSERT( pData );
				if( nullptr == pData )
					return;

				pData->status = stNtf.status;
				gDataViewModel.ItemChanged( wxDataViewItem( pData ) );
			}
		}

		///
		/// \brief 同步任务进度
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnProgressNtf( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ClientProgressNtf stNtf;
			serialization ar( (xgc_lpvoid) data, size, size );

			ar >> stNtf;
			auto pNode = gDataViewModel.GetNode( session );
			wxASSERT( pNode );
			if( nullptr == pNode )
				return;

			if( 0 == stNtf.pid && 0 == stNtf.tid )
			{
				pNode->progress = stNtf.progress;
				gDataViewModel.ItemChanged( wxDataViewItem( pNode ) );
			}
			else
			{
				auto pData = pNode->GetClientData( stNtf.pid, stNtf.tid );
				wxASSERT( pData );
				if( nullptr == pData )
					return;

				pData->progress = stNtf.progress;
				gDataViewModel.ItemChanged( wxDataViewItem( pData ) );
			}
		}

		xgc_void OnFilesReq( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ClientFilesReq stReq;
			serialization ar( (xgc_lpvoid) data, size, size );
			ar >> stReq;

			auto pNode = gDataViewModel.GetNode( session );
			wxASSERT( pNode );
			pNode->AddToSendQueue( stReq.root, stReq.relative );
		}

		xgc_void OnNewFileAck( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ClientNewFileAck stAck;
			serialization ar( (xgc_lpvoid) data, size, size );

			ar >> stAck;

			if( stAck.result == 0 )
			{
				auto pNode = gDataViewModel.GetNode( session );
				wxASSERT( pNode );
				pNode->SendFiles( 0 );
			}
		}

		xgc_void OnFileDataAck( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata )
		{
			ClientFileDataAck stAck;
			serialization ar( (xgc_lpvoid) data, size, size );
			ar >> stAck;

			auto pNode = gDataViewModel.GetNode( session );
			wxASSERT( pNode );
			pNode->SendFiles( stAck.received );
		}
	}
}