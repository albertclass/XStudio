///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file ProxyListModel.cpp
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 列表数据模型
///
///////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "ClientDataViewModel.h"
#include "PythonScript.h"

ClientDataViewModel gDataViewModel;

ClientDataViewModel::ClientDataViewModel()
{
	//ClientDataNode *pNode = XGC_NEW ClientDataNode( 65534, "0.0.0.0", 1000 );
	//InsertNode( pNode );
}

ClientDataViewModel::~ClientDataViewModel()
{
	for each(auto pNode in mDataStore)
		delete pNode;
	
	mDataStore.clear();
}

bool ClientDataViewModel::InsertData( ClientDataNode* pNode, ClientData* pData )
{
	wxASSERT( pNode );
	if( InsertNode( pNode ) )
	{
		pNode->Insert( pData );

		ItemAdded( wxDataViewItem( pNode ), wxDataViewItem( pData ) );
	}
	return true;
}

bool ClientDataViewModel::InsertNode( ClientDataNode * pNode )
{
	auto it = std::find_if( mDataStore.begin(), mDataStore.end(), [pNode]( const ClientDataNode* Ptr )->bool{
		return pNode->GetSessionID() == Ptr->GetSessionID();
	} );

	if( it != mDataStore.end() )
		return true;

	mDataStore.push_back( pNode );

	ItemAdded( wxDataViewItem( 0 ), wxDataViewItem( pNode ) );
	return true;
}

void ClientDataViewModel::DeleteData( network_t nSessionID )
{
	auto it = mDataStore.begin(); 
	while( it != mDataStore.end() )
	{
		auto pNode = *it;
		wxASSERT( pNode );
		auto pData = pNode->Remove( nSessionID );
		if( pData )
		{
			ItemDeleted( wxDataViewItem( pNode ), wxDataViewItem( pData ) );
			delete pData;
		}

		if( pNode->GetChildCount() != 0 )
		{
			++it;
			continue;
		}

		it = mDataStore.erase( it );
		ItemDeleted( wxDataViewItem( 0 ), wxDataViewItem( pNode ) );
		delete pNode;
	}
}

ClientDataNode * ClientDataViewModel::GetNode( network_t nSessionID )
{
	for( auto pNode : mDataStore )
	{
		if( pNode->GetSessionID() == nSessionID )
			return pNode;
	}

	return nullptr;
}

void ClientDataViewModel::SetClientDataScript( const wxDataViewItemArray &items, const wxString &strScriptPath )
{
	for( auto i = 0; i < items.Count(); ++i )
	{
		wxDataViewItem& item = items[i];
		ClientDataType* pType = (ClientDataType*) item.GetID();
		switch( pType->type )
		{
			case 0:
			case 1:
			ClientData* pData = (ClientData*) pType;
			pData->script = strScriptPath;
			break;
		}
	}
}

void ClientDataViewModel::SetClientNodeAccounts( const wxDataViewItem & item, std::list< AppConfig::account* > &accounts )
{
	wxASSERT( item.IsOk() );
	ClientDataNode* pNode = (ClientDataNode*) item.GetID();
	wxASSERT( pNode && pNode->type == 0 );

	// 建立界面显示
	for( auto pAccount : accounts )
	{
		ClientData* pData = XGC_NEW ClientData();
		wxASSERT( pData );
		pData->cpu = 0;
		pData->mem = 0;
		pData->pid = 0;
		pData->tid = 0;
		pData->status = "等待";
		pData->username = pAccount->username;
		pData->password = pAccount->password;
		pData->script = pAccount->script;
		pData->progress = 0;
		pData->progress_max = 100;
		InsertData( pNode, pData );
	}
}

void ClientDataViewModel::Start( const wxDataViewItem & item )
{
	ClientDataType * pType = (ClientDataType*) item.GetID();
	switch( pType->type )
	{
		case 0: // Node
		{
			ClientDataNode* pNode = (ClientDataNode*) pType;
			wxDataViewItemArray items;
			pNode->GetChildren( items );
			for each(auto &it in items)
				Start( it );
		}
		break;
		case 1: // Data
		{
			ClientData* pData = (ClientData*) pType;
			ServerStartReq stReq;
			stReq.username = pData->username.ToStdString();
			stReq.password = pData->password.ToStdString();
			stReq.script = pData->script.ToStdString();
			getScript().GetLoaderVersion( stReq.loader_version );
			getScript().GetModuleVersion( stReq.script, stReq.module_version );
			stReq.address = getConfig().GetServerAddress( 0 );

			Send( pData->parent->GetSessionID(), stReq );
		}
		break;
	}
}

void ClientDataViewModel::Stop( const wxDataViewItem & item )
{
	ClientDataType * pType = (ClientDataType*) item.GetID();
	switch( pType->type )
	{
		case 0: // Node
		{
			ClientDataNode* pNode = (ClientDataNode*) pType;
			wxDataViewItemArray items;
			pNode->GetChildren( items );
			for each(auto &it in items)
				Start( it );
		}
		break;
		case 1: // Data
		{
			ClientData* pData = (ClientData*) pType;
			ServerStopNtf stNtf;
			stNtf.username = pData->username.ToStdString();

			Send( pData->parent->GetSessionID(), stNtf );
		}
		break;
	}
}

void ClientDataViewModel::UpdatePush()
{
	ServerFileList stLoaderNtf;
	stLoaderNtf.id = 0;
	stLoaderNtf.filelist = getConfig().GetFilelist( 0 );

	ServerFileList stModuleNtf;
	stModuleNtf.id = 1;
	stModuleNtf.filelist = getConfig().GetFilelist( 1 );

	for each( auto child in mDataStore )
	{
		Send( child->GetSessionID(), stLoaderNtf );
		Send( child->GetSessionID(), stModuleNtf );
	}
}

int ClientDataViewModel::Compare( const wxDataViewItem & item1, const wxDataViewItem & item2, unsigned int column, bool ascending ) const
{
	wxASSERT(item1.IsOk() && item2.IsOk());
	// should never happen

	if (IsContainer(item1) && IsContainer(item2))
	{
		wxVariant value1, value2;
		GetValue( value1, item1, 0 );
		GetValue( value2, item2, 0 );

		wxString str1 = value1.GetString();
		wxString str2 = value2.GetString();
		int res = str1.Cmp( str2 );
		if (res) return res;

		// items must be different
		wxUIntPtr litem1 = (wxUIntPtr) item1.GetID();
		wxUIntPtr litem2 = (wxUIntPtr) item2.GetID();

		return litem1-litem2;
	}

	return wxDataViewModel::Compare( item1, item2, column, ascending );
}


// return type as reported by wxVariant

wxString ClientDataViewModel::GetColumnType( unsigned int col ) const
{
	switch( col )
	{
		case Col_Address: return wxT( "string" );
		case Col_PID: return wxT( "long" );
		case Col_TID: return wxT( "long" );
		case Col_CPU: return wxT( "double" );
		case Col_MEM: return wxT( "double" );
		case Col_Status: return wxT( "string" );
		case Col_Progress: return wxT( "long" );
		case Col_Account: return wxT( "string" );
		case Col_Script: return wxT( "string" );
	}

	return wxT( "string" );
}

void ClientDataViewModel::GetValue( wxVariant & variant, const wxDataViewItem & item, unsigned int col ) const
{
	wxASSERT(item.IsOk());

	ClientDataType* pType = (ClientDataType*) item.GetID();
	switch( col )
	{
		case Col_PID:
		variant = wxString::Format( "%u", pType->pid );
		break;
		case Col_TID:
		variant = wxString::Format( "%u", pType->tid );
		break;
		case Col_CPU:
		variant = wxString::Format( "%.2f%%", pType->cpu );
		break;
		case Col_MEM:
		variant = wxString::Format( "%.2f%%", pType->mem );
		break;
		case Col_Status:
		variant = pType->status;
		break;
		case Col_Progress:
		variant = pType->progress_max ? pType->progress * 100L / pType->progress_max : 100;
		break;
		default:
		switch( pType->type )
		{
			case 0:
			{
				ClientDataNode* pNode = (ClientDataNode*) pType;
				switch( col )
				{
					case Col_Address:
					variant = pNode->GetAddress();
					break;
				}
			}
			break;
			case 1:
			{
				ClientData* pData = (ClientData*) item.GetID();
				switch( col )
				{
					case Col_Account:
					variant = pData->username;
					break;
					case Col_Script:
					variant = pData->script;
					break;
				}
			}
		}
		break;
	}
}

bool ClientDataViewModel::SetValue( const wxVariant & variant, const wxDataViewItem & item, unsigned int col )
{
	ClientData* pData = (ClientData*) item.GetID();
	if( col == Col_Script )
	{
		pData->script = variant.GetString();
		return true;
	}

	return false;
}

bool ClientDataViewModel::IsEnabled( const wxDataViewItem & item, unsigned int col ) const
{
	wxASSERT(item.IsOk());

	return true;
}

wxDataViewItem ClientDataViewModel::GetParent( const wxDataViewItem & item ) const
{
	// the invisible root node has no parent
	if (!item.IsOk())
		return wxDataViewItem(0);

	ClientDataType *pNode = (ClientDataType*) item.GetID();
	if( pNode->type == 0 )
		return wxDataViewItem(0);
	
	ClientData* pData = (ClientData*) pNode;
	wxASSERT( pData );

	return wxDataViewItem( (void*) pData->parent );
}

bool ClientDataViewModel::IsContainer( const wxDataViewItem & item ) const
{
	if (!item.IsOk())
		return true;

	ClientDataType *node = (ClientDataType*) item.GetID();
	if( node->type == 0 )
	{
		ClientDataNode* pNode = (ClientDataNode*)node;
		return pNode->GetChildCount() != 0;
	}

	return false;
}

unsigned int ClientDataViewModel::GetChildren( const wxDataViewItem & parent, wxDataViewItemArray & array ) const
{
	ClientDataType* pType = (ClientDataType*) parent.GetID();
	if( !pType )
	{
		for each(auto pNode in mDataStore)
		{
			array.Add( wxDataViewItem( (void*) pNode ) );
		}

		return array.Count();
	}

	if( pType->type == 0 )
	{
		ClientDataNode* pNode = (ClientDataNode*) pType;
		wxASSERT( pNode );

		return pNode->GetChildren( array );
	}

	return 0;
}

ClientDataNode::ClientDataNode( network_t nSessionID, const wxString & strAddress, wxUint32 nPID, wxUint32 nTID )
	: ClientDataType { 0, nPID, nTID, 0, 0, 0, 100 }
	, mAddress( strAddress )
	, mSessionID( nSessionID )
{
	// type = 0;
}

size_t ClientDataNode::GetChildren( wxDataViewItemArray & array )
{
	for each( auto pNode in mChildren )
	{
		array.Add( wxDataViewItem( (void*) pNode ) );
	}

	return array.Count();
}

ClientData * ClientDataNode::GetClientData( const wxString &username )
{
	auto it = std::find_if( mChildren.begin(), mChildren.end(), [username]( const ClientData* pData )->bool{
		if( pData->username == username )
			return true;

		return false;
	} );

	if( it != mChildren.end() )
		return *it;

	return nullptr;
}

ClientData * ClientDataNode::GetClientData( wxUint32 pid, wxUint32 tid )
{
	auto it = std::find_if( mChildren.begin(), mChildren.end(), [pid, tid]( const ClientData* pData )->bool{
		if( pData->pid == pid && pData->tid == tid )
			return true;

		return false;
	} );

	if( it != mChildren.end() )
		return *it;

	return nullptr;
}

void ClientDataNode::Insert( ClientData * pData )
{
	pData->type = 1;
	pData->session = mSessionID;
	pData->parent = this;
	mChildren.push_back( pData );
}

ClientData* ClientDataNode::Remove( wxUint32 nPID, wxUint32 nTID )
{
	for( auto it = mChildren.begin(); it != mChildren.end(); ++it )
	{
		ClientData* pData = *it;
		wxASSERT( pData );

		if( pData->pid == nPID && 
			pData->tid == nTID )
		{
			mChildren.erase( it );
			return pData;
		}
	}

	return nullptr;
}

ClientData* ClientDataNode::Remove( network_t nSessionID )
{
	for( auto it = mChildren.begin(); it != mChildren.end(); ++it )
	{
		ClientData* pData = *it;
		wxASSERT( pData );

		if( pData->session == nSessionID )
		{
			mChildren.erase( it );
			return pData;
		}
	}

	return nullptr;
}

void ClientDataNode::AddToSendQueue( unsigned int root, const fs::path & relative )
{
	if( root >= _countof(mSenderQueue) )
		return;

	FileSender &sender = mSenderQueue[root];

	// 若没有将该目录添加到下载队列中的话，则加进去
	if( sender.IsExist( relative ) )
		return;

	sender.Push( relative );
	if( mFileSender && !mFileSender->IsSending() )
		SendFiles( 0 );
}

void ClientDataNode::SendFiles( xgc_long nReceivedBytes )
{
	if( mFileSender == nullptr )
		mFileSender = mSenderQueue;

	size_t i = 0;
	for( ; i < _countof( mSenderQueue ); ++i )
	{
		if( mFileSender->SendDone( mSessionID, nReceivedBytes ) )
			break;

		mFileSender++;

		if( mFileSender >= mSenderQueue + _countof(mSenderQueue) )
			mFileSender = mSenderQueue;
	}

	status = mFileSender->GetTips();
	progress = mFileSender->GetProgress();

	gDataViewModel.ItemChanged( wxDataViewItem( this ) );
}

FileSender::FileSender( const fs::path & root, unsigned int id )
	: mRootPath( root )
	, mSendFileSize( 0 )
	, mRecvFileSize( 0 )
	, mFileSize( 0 )
	, mStream( -1 )
	, mPathID( id )
	, mFileCount( 0 )
{
}

FileSender::~FileSender()
{
}

xgc_bool FileSender::PushDirectory( const fs::path &directory, xgc_vector< fs::path > ignore_files )
{
	// 忽略列表
	auto ignore_files_count = ignore_files.size();

	auto ignore = mRootPath / directory / "ignore_files";
	while( fs::exists( ignore ) )
	{
		std::fstream file;

		file.open( ignore, std::ios_base::in );
		if( !file.is_open() )
			break;

		while( !file.eof() )
		{
			xgc_char filename[_MAX_PATH] = { 0 };
			file.getline( filename, sizeof( filename ) );
			if( filename[0] == '*' )
				ignore_files.push_back( filename + 1 );
			else
				ignore_files.push_back( directory / filename );
		}

		file.close();
		break;
	}

	for( const auto &entry : fs::directory_iterator { mRootPath / directory } )
	{
		fs::path entry_path = entry.path();
		// 查找忽略文件列表中的文件
		auto it = std::find_if( ignore_files.begin(), ignore_files.end(), [&entry_path,this]( const fs::path &ignore ){
			return mRootPath / ignore == entry_path;
		} );

		if( it != ignore_files.end() )
			continue;

		if( fs::is_directory( entry_path ) )
		{
			PushDirectory( directory / entry_path.filename(), ignore_files );
		}
		else
		{
			PushFile( directory / entry_path.filename() );
		}
	}

	ignore_files.resize( ignore_files_count );
	return true;
}

xgc_bool FileSender::PushFile( const fs::path & filepath )
{
	mFileList.push_back( filepath );
	++mFileCount;
	return true;
}

wxString FileSender::GetTips() const
{
	std::stringstream ss;
	if( IsSendFinished() )
		ss << "脚本更新完成。";
	else
		ss << mFileList.front();

	return ss.str();
}

xgc_long FileSender::GetProgress() const
{
	return mFileCount ? 100 - mFileList.size() * 100 / mFileCount : 100;
}

bool FileSender::SendOnceMore( network_t nSessionID )
{
	while( mFileList.size() )
	{
		// 文件流处于关闭状态，则将等待列表中的文件打开
		if( -1 == mStream )
		{
			auto relative_path = mFileList.front().string();
			auto absolute_path = ( mRootPath / mFileList.front() ).string();
			errno_t err = _sopen_s( &mStream, absolute_path.c_str(), O_RDONLY | O_BINARY, SH_DENYWR, S_IREAD );
			// 对文件打开状态做判断
			if( err != 0 )
			{
				mFileList.pop_front();
				continue;
			}

			// 文件处理进度归零
			mSendFileSize = 0;
			mRecvFileSize = 0;

			// 获取文件大小
			struct _stat fst;
			_fstat( mStream, &fst );
			mFileSize = fst.st_size;

			ClientNewFileNtf stFile;
			stFile.path_id = mPathID;
			stFile.file_size = mFileSize;
			stFile.buffer = serialization_buffer( (xgc_lpvoid)relative_path.c_str(), relative_path.length() + 1 );

			Send( nSessionID, stFile );
			return true;
		}

		if( mRecvFileSize != _tell( mStream ) )
			_lseek( mStream, mRecvFileSize, SEEK_SET );

		// 若文件已经到尾部，则关闭发送流
		if( !_eof( mStream ) )
			break;

		// 关闭当前文件，并重设状态
		_close( mStream );
		mStream = -1;
		mFileList.pop_front();
	}

	// 已没有文件等待发送了。
	if( mFileList.empty() )
	{
		mFileCount = 0;
		return false;
	}

	char buffer[4096];
	// 继续发送剩下的数据
	size_t readbytes = _read( mStream, buffer, sizeof( buffer ) );
	if( readbytes > 0 )
	{
		ClientFileDataNtf stFile;
		stFile.offset = mRecvFileSize;
		stFile.buffer = serialization_buffer( buffer, readbytes );

		Send( nSessionID, stFile );

		mSendFileSize = _tell( mStream );
	}

	return true;
}

bool FileSender::SendDone( network_t nSessionID, xgc_long nReceivedBytes )
{
	if( nReceivedBytes == -1 )
	{
		_close( mStream );
		mStream = -1;
	}
	else
	{
		mRecvFileSize = nReceivedBytes;
	}

	return SendOnceMore( nSessionID );
}
