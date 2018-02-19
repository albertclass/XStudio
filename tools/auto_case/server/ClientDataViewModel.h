///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file ProxyListModel.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// ����ģ��
///
///////////////////////////////////////////////////////////////
#pragma once

#ifndef _PROXYLISTMODEL_H_
#define _PROXYLISTMODEL_H_
#include "wx/dataview.h"

struct ClientDataType
{
	/// �������ͣ� 0 - �ڵ㣬 1 - ����
	u_short type;
	/// ����ID
	u_long	pid;
	/// �߳�ID
	u_long	tid;
	/// Զ�̻��ϵ�cpuռ����
	float cpu;
	/// Զ�̻��ϵ�memռ����
	float mem;
	/// ����
	long progress;
	/// ����
	long progress_max;
	/// Զ�̻�״̬
	wxString status;
};

///
/// \brief �������ڵ�
///
/// \author albert.xu
/// \date 2016/03/07 14:53
///
struct ClientDataNode;

///
/// \brief �ļ��·�������
///
/// \author albert.xu
/// \date 2016/03/10 12:40
///
class FileSender
{
public:
	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2016/04/13 15:14
	///
	FileSender( const fs::path &root, unsigned int id );

	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2016/04/13 15:14
	///
	~FileSender();

	///
	/// \brief ��ȡ��·��
	///
	/// \author albert.xu
	/// \date 2016/04/13 14:39
	///
	const fs::path& GetRootPath()const
	{
		return mRootPath;
	}

	///
	/// \brief ��ȡ���ڷ����е��ļ�
	///
	/// \author albert.xu
	/// \date 2016/04/13 18:23
	///
	wxString GetTips()const;

	///
	/// \brief ��ȡ���ڷ����е��ļ�
	///
	/// \author albert.xu
	/// \date 2016/04/13 18:23
	///
	xgc_long GetProgress()const;

	///
	/// \brief �Ƿ�����ڷ��Ͷ�����
	///
	/// \author albert.xu
	/// \date 2016/04/13 15:01
	///
	xgc_bool IsExist( const fs::path& file )const
	{
		if( file == mRootPath )
			return true;

		auto it = std::find_if( mFileList.begin(), mFileList.end(),
								[&file,this]( const fs::path &relative ){
			return file == (file.is_absolute() ? mRootPath / relative : relative);
		} );

		return it != mFileList.end();
	}

	///
	/// \brief ��������һ��������
	///
	/// \return �Ƿ���Ҫ�ٴη���
	/// \author albert.xu
	/// \date 2016/03/10 14:12
	///
	bool SendOnceMore( network_t nSessionID );

	///
	/// \brief ���ͳɹ�
	///
	/// \author albert.xu
	/// \date 2016/04/13 15:43
	///
	bool SendDone( network_t nSessionID, xgc_long nReceivedBytes );

	///
	/// \brief �Ƿ���ɷ���
	///
	/// \author albert.xu
	/// \date 2016/03/10 15:58
	///
	bool IsSendFinished()const
	{
		return mFileList.empty();
	}

	///
	/// \brief ����״̬
	///
	/// \author albert.xu
	/// \date 2016/03/10 16:15
	///
	bool IsSending() const
	{
		return mStream != -1;
	}

	///
	/// \brief ���͵�����
	///
	/// \author albert.xu
	/// \date 2016/08/04 16:04
	///
	bool Push( const fs::path &relative )
	{
		if( fs::is_directory( mRootPath / relative ) )
			return PushDirectory( relative, {} );
		else
			return PushFile( relative );
	}

protected:
	///
	/// \brief ��������Ŀ¼
	///
	/// \author albert.xu
	/// \date 2016/03/10 12:42
	///
	xgc_bool PushDirectory( const fs::path &directory, xgc_vector< fs::path > ignore_files );

	///
	/// \brief ���͵����ļ�
	///
	/// \author albert.xu
	/// \date 2016/08/03 11:38
	///
	xgc_bool PushFile( const fs::path &filepath );

private:
	fs::path mRootPath;
	/// ���·��ļ��б�
	xgc_list< fs::path > mFileList;
	/// ��ǰ���ڴ�����ļ�
	int mStream;
	/// ���͵��ļ���С
	size_t mFileSize;
	/// �ѷ��͵��ֽ���
	xgc_long mSendFileSize;
	/// �ѽ��յ��ֽ���
	xgc_long mRecvFileSize;
	/// ·������
	xgc_long mPathID;
	/// ��Ҫ���͵��ļ�����
	xgc_long mFileCount;
};

///
/// \brief ��������
///
/// \author albert.xu
/// \date 2016/02/23 11:51
///
struct ClientData : public ClientDataType
{
	/// ����Ự��ʶ
	network_t	session;
	/// �˺�
	wxString	username;
	/// ����
	wxString	password;
	/// Զ�̻��ϵĽű�
	wxString	script;
	/// ���ڵ�
	ClientDataNode *parent;
};

///
/// \brief �������ڵ�
///
/// \author albert.xu
/// \date 2016/03/04 11:23
///
struct ClientDataNode : public ClientDataType
{
private:
	/// �������ַ
	wxString mAddress;
	/// �ỰID
	network_t mSessionID;
	/// ����
	wxVector< ClientData* > mChildren;
	/// �����Ͳ��Խű��б�
	FileSender mSenderQueue[2] = 
	{ 
		{ getConfig().GetLoaderPath(), 0 }, 
		{ getConfig().GetModulePath(), 1 } 
	};
	/// ��ǰ���ڷ��͵�
	FileSender *mFileSender = mSenderQueue;

public:
	ClientDataNode( network_t nSessionID, const wxString &strAddress, wxUint32 nPID, wxUint32 nTID );

	~ClientDataNode()
	{
		for each(auto pData in mChildren)
			delete pData;

		mChildren.clear();
	}

	///
	/// \brief ��ȡ�ڵ�����Ӧ������������ַ
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:36
	///
	const wxString& GetAddress()const
	{
		return mAddress;
	}

	///
	/// \brief ��ȡ�ỰID
	///
	/// \author albert.xu
	/// \date 2016/03/07 16:48
	///
	network_t GetSessionID()const
	{
		return mSessionID;
	}

	///
	/// \brief �ӽڵ�����
	///
	/// \author albert.xu
	/// \date 2016/03/07 14:42
	///
	size_t GetChildCount() const
	{
		return mChildren.size();
	}

	///
	/// \brief ��ȡ�Ӷ���
	///
	/// \author albert.xu
	/// \date 2016/03/07 15:12
	///
	size_t GetChildren( wxDataViewItemArray &array );

	///
	/// \brief ��ȡ�ڵ�
	///
	/// \author albert.xu
	/// \date 2016/04/18 19:32
	///
	ClientData* GetClientData( const wxString &username );

	///
	/// \brief ��ȡ�ڵ�
	///
	/// \author albert.xu
	/// \date 2016/03/14 11:44
	///
	ClientData* GetClientData( wxUint32 pid, wxUint32 tid );

	///
	/// \brief ����һ����¼
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:36
	///
	void Insert( ClientData* pData );

	///
	/// \brief ɾ��һ����¼
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:37
	///
	ClientData* Remove( wxUint32 nPID, wxUint32 nTID );

	///
	/// \brief ɾ��һ����¼
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:37
	///
	ClientData* Remove( network_t nSessionID );

	///
	/// \brief ���ļ���ӵ����Ͷ���
	///
	/// \author albert.xu
	/// \date 2016/03/10 15:40
	///
	void AddToSendQueue( unsigned int root, const fs::path & relative );

	///
	/// \brief �����ļ�
	///
	/// \author albert.xu
	/// \date 2016/03/10 15:54
	///
	void SendFiles( xgc_long nReceivedBytes );
};

///
/// \brief ����洢����
///
/// \author albert.xu
/// \date 2016/02/23 11:48
///
class ClientDataViewModel : public wxDataViewModel
{
public:
	///
	/// \brief ��ö��
	///
	/// \author albert.xu
	/// \date 2016/02/23 11:50
	///
	enum Column
	{
		Col_Address,
		Col_PID,
		Col_TID,
		Col_CPU,
		Col_MEM,
		Col_Status,
		Col_Progress,
		Col_Account,
		Col_Script,
		Col_Count,
	};

private:
	/// ���ݴ洢
	xgc_vector< ClientDataNode* > mDataStore;
public:
	ClientDataViewModel();
	~ClientDataViewModel();

	///
	/// \brief ����һ���ͻ���
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:06
	///
	bool InsertData( ClientDataNode* pNode, ClientData* pData );

	///
	/// \brief ����һ���ͻ���
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:06
	///
	bool InsertNode( ClientDataNode* pNode );

	///
	/// \brief ɾ��һ���ڵ�
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:07
	///
	void DeleteData( network_t nSessionID );

	///
	/// \brief ��ȡ�ͻ��ڵ�
	///
	/// \author albert.xu
	/// \date 2016/03/10 15:21
	///
	ClientDataNode* GetNode( network_t nSessionID );

	///
	/// \brief ������ѡ��Ľű�
	///
	/// \author albert.xu
	/// \date 2016/03/10 12:14
	///
	void SetClientDataScript( const wxDataViewItemArray &items, const wxString &strScriptPath );

	///
	/// \brief ���ýڵ���˺���Ϣ
	///
	/// \author albert.xu
	/// \date 2016/03/11 10:54
	///
	void SetClientNodeAccounts( const wxDataViewItem &item, std::list< AppConfig::account* > &accounts );

	///
	/// \brief ��ʼ����
	///
	/// \author albert.xu
	/// \date 2016/03/14 11:54
	///
	void Start( const wxDataViewItem &item );

	///
	/// \brief ��ֹ����
	///
	/// \author albert.xu
	/// \date 2016/03/14 11:55
	///
	void Stop( const wxDataViewItem &item );

	///
	/// \brief ��������
	///
	/// \author albert.xu
	/// \date 2016/08/08 16:30
	///
	void UpdatePush();
protected:
	// override sorting to always sort branches ascendingly

	int Compare( const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int column, bool ascending ) const;

	// implementation of base class virtuals to define model

	virtual unsigned int GetColumnCount() const
	{
		return 6;
	}

	// return type as reported by wxVariant
	virtual wxString GetColumnType( unsigned int col ) const;

	virtual void GetValue( wxVariant &variant, const wxDataViewItem &item, unsigned int col ) const;

	virtual bool SetValue( const wxVariant &variant, const wxDataViewItem &item, unsigned int col );

	virtual bool IsEnabled( const wxDataViewItem &item, unsigned int col ) const;

	virtual wxDataViewItem GetParent( const wxDataViewItem &item ) const;

	virtual bool IsContainer( const wxDataViewItem &item ) const;

	virtual unsigned int GetChildren( const wxDataViewItem &parent, wxDataViewItemArray &array ) const;

	virtual bool HasContainerColumns(const wxDataViewItem& WXUNUSED(item)) const { return true; }
};

extern ClientDataViewModel gDataViewModel;
#endif // _PROXYLISTMODEL_H_