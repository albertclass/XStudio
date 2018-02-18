///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file ProxyListModel.h
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 数据模型
///
///////////////////////////////////////////////////////////////
#pragma once

#ifndef _PROXYLISTMODEL_H_
#define _PROXYLISTMODEL_H_
#include "wx/dataview.h"

struct ClientDataType
{
	/// 数据类型， 0 - 节点， 1 - 数据
	u_short type;
	/// 进程ID
	u_long	pid;
	/// 线程ID
	u_long	tid;
	/// 远程机上的cpu占用率
	float cpu;
	/// 远程机上的mem占用率
	float mem;
	/// 进度
	long progress;
	/// 进度
	long progress_max;
	/// 远程机状态
	wxString status;
};

///
/// \brief 服务器节点
///
/// \author albert.xu
/// \date 2016/03/07 14:53
///
struct ClientDataNode;

///
/// \brief 文件下发管理器
///
/// \author albert.xu
/// \date 2016/03/10 12:40
///
class FileSender
{
public:
	///
	/// \brief 构造
	///
	/// \author albert.xu
	/// \date 2016/04/13 15:14
	///
	FileSender( const fs::path &root, unsigned int id );

	///
	/// \brief 析构
	///
	/// \author albert.xu
	/// \date 2016/04/13 15:14
	///
	~FileSender();

	///
	/// \brief 获取根路径
	///
	/// \author albert.xu
	/// \date 2016/04/13 14:39
	///
	const fs::path& GetRootPath()const
	{
		return mRootPath;
	}

	///
	/// \brief 获取正在发送中的文件
	///
	/// \author albert.xu
	/// \date 2016/04/13 18:23
	///
	wxString GetTips()const;

	///
	/// \brief 获取正在发送中的文件
	///
	/// \author albert.xu
	/// \date 2016/04/13 18:23
	///
	xgc_long GetProgress()const;

	///
	/// \brief 是否存在于发送队列中
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
	/// \brief 继续发送一部分数据
	///
	/// \return 是否还需要再次发送
	/// \author albert.xu
	/// \date 2016/03/10 14:12
	///
	bool SendOnceMore( network_t nSessionID );

	///
	/// \brief 发送成功
	///
	/// \author albert.xu
	/// \date 2016/04/13 15:43
	///
	bool SendDone( network_t nSessionID, xgc_long nReceivedBytes );

	///
	/// \brief 是否完成发送
	///
	/// \author albert.xu
	/// \date 2016/03/10 15:58
	///
	bool IsSendFinished()const
	{
		return mFileList.empty();
	}

	///
	/// \brief 发送状态
	///
	/// \author albert.xu
	/// \date 2016/03/10 16:15
	///
	bool IsSending() const
	{
		return mStream != -1;
	}

	///
	/// \brief 发送到队列
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
	/// \brief 发送整个目录
	///
	/// \author albert.xu
	/// \date 2016/03/10 12:42
	///
	xgc_bool PushDirectory( const fs::path &directory, xgc_vector< fs::path > ignore_files );

	///
	/// \brief 发送单个文件
	///
	/// \author albert.xu
	/// \date 2016/08/03 11:38
	///
	xgc_bool PushFile( const fs::path &filepath );

private:
	fs::path mRootPath;
	/// 待下发文件列表
	xgc_list< fs::path > mFileList;
	/// 当前正在传输的文件
	int mStream;
	/// 发送的文件大小
	size_t mFileSize;
	/// 已发送的字节数
	xgc_long mSendFileSize;
	/// 已接收的字节数
	xgc_long mRecvFileSize;
	/// 路径索引
	xgc_long mPathID;
	/// 需要发送的文件数量
	xgc_long mFileCount;
};

///
/// \brief 代理数据
///
/// \author albert.xu
/// \date 2016/02/23 11:51
///
struct ClientData : public ClientDataType
{
	/// 网络会话标识
	network_t	session;
	/// 账号
	wxString	username;
	/// 密码
	wxString	password;
	/// 远程机上的脚本
	wxString	script;
	/// 父节点
	ClientDataNode *parent;
};

///
/// \brief 服务器节点
///
/// \author albert.xu
/// \date 2016/03/04 11:23
///
struct ClientDataNode : public ClientDataType
{
private:
	/// 物理机地址
	wxString mAddress;
	/// 会话ID
	network_t mSessionID;
	/// 子项
	wxVector< ClientData* > mChildren;
	/// 待发送测试脚本列表
	FileSender mSenderQueue[2] = 
	{ 
		{ getConfig().GetLoaderPath(), 0 }, 
		{ getConfig().GetModulePath(), 1 } 
	};
	/// 当前正在发送的
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
	/// \brief 获取节点所对应物理机的网络地址
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:36
	///
	const wxString& GetAddress()const
	{
		return mAddress;
	}

	///
	/// \brief 获取会话ID
	///
	/// \author albert.xu
	/// \date 2016/03/07 16:48
	///
	network_t GetSessionID()const
	{
		return mSessionID;
	}

	///
	/// \brief 子节点数量
	///
	/// \author albert.xu
	/// \date 2016/03/07 14:42
	///
	size_t GetChildCount() const
	{
		return mChildren.size();
	}

	///
	/// \brief 获取子对象
	///
	/// \author albert.xu
	/// \date 2016/03/07 15:12
	///
	size_t GetChildren( wxDataViewItemArray &array );

	///
	/// \brief 获取节点
	///
	/// \author albert.xu
	/// \date 2016/04/18 19:32
	///
	ClientData* GetClientData( const wxString &username );

	///
	/// \brief 获取节点
	///
	/// \author albert.xu
	/// \date 2016/03/14 11:44
	///
	ClientData* GetClientData( wxUint32 pid, wxUint32 tid );

	///
	/// \brief 插入一条记录
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:36
	///
	void Insert( ClientData* pData );

	///
	/// \brief 删除一条记录
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:37
	///
	ClientData* Remove( wxUint32 nPID, wxUint32 nTID );

	///
	/// \brief 删除一条记录
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:37
	///
	ClientData* Remove( network_t nSessionID );

	///
	/// \brief 将文件添加到发送队列
	///
	/// \author albert.xu
	/// \date 2016/03/10 15:40
	///
	void AddToSendQueue( unsigned int root, const fs::path & relative );

	///
	/// \brief 发送文件
	///
	/// \author albert.xu
	/// \date 2016/03/10 15:54
	///
	void SendFiles( xgc_long nReceivedBytes );
};

///
/// \brief 代理存储数据
///
/// \author albert.xu
/// \date 2016/02/23 11:48
///
class ClientDataViewModel : public wxDataViewModel
{
public:
	///
	/// \brief 列枚举
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
	/// 数据存储
	xgc_vector< ClientDataNode* > mDataStore;
public:
	ClientDataViewModel();
	~ClientDataViewModel();

	///
	/// \brief 新增一个客户端
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:06
	///
	bool InsertData( ClientDataNode* pNode, ClientData* pData );

	///
	/// \brief 新增一个客户端
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:06
	///
	bool InsertNode( ClientDataNode* pNode );

	///
	/// \brief 删除一个节点
	///
	/// \author albert.xu
	/// \date 2016/03/04 12:07
	///
	void DeleteData( network_t nSessionID );

	///
	/// \brief 获取客户节点
	///
	/// \author albert.xu
	/// \date 2016/03/10 15:21
	///
	ClientDataNode* GetNode( network_t nSessionID );

	///
	/// \brief 设置已选项的脚本
	///
	/// \author albert.xu
	/// \date 2016/03/10 12:14
	///
	void SetClientDataScript( const wxDataViewItemArray &items, const wxString &strScriptPath );

	///
	/// \brief 设置节点的账号信息
	///
	/// \author albert.xu
	/// \date 2016/03/11 10:54
	///
	void SetClientNodeAccounts( const wxDataViewItem &item, std::list< AppConfig::account* > &accounts );

	///
	/// \brief 开始测试
	///
	/// \author albert.xu
	/// \date 2016/03/14 11:54
	///
	void Start( const wxDataViewItem &item );

	///
	/// \brief 终止测试
	///
	/// \author albert.xu
	/// \date 2016/03/14 11:55
	///
	void Stop( const wxDataViewItem &item );

	///
	/// \brief 更新推送
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