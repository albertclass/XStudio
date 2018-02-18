#pragma once
class AppConfig
{
public:
	struct account
	{
		xgc_string username;
		xgc_string password;
		xgc_string script;

		bool used;
	};

public:
	AppConfig();
	~AppConfig();

	///
	/// \brief 获取预置账号数量
	///
	/// \author albert.xu
	/// \date 2016/04/06 19:40
	///
	xgc_size GetAccountCount()const;

	///
	/// \brief 获取账号
	///
	/// \author albert.xu
	/// \date 2016/04/06 19:40
	///
	account* GetAccount( size_t i );

	///
	/// \brief 获取可用服务器数
	///
	/// \author albert.xu
	/// \date 2016/04/11 17:22
	///
	xgc_size GetServerCount()const;

	///
	/// \brief 根据索引获取服务器名
	///
	/// \author albert.xu
	/// \date 2016/04/11 17:22
	///
	xgc_lpcstr GetServerName( xgc_size nIndex )const;

	///
	/// \brief 根据索引获取服务器连接地址
	///
	/// \author albert.xu
	/// \date 2016/04/11 17:22
	///
	xgc_lpcstr GetServerAddress( xgc_size nIndex )const;

	///
	/// \brief 获取测试路径
	///
	/// \author albert.xu
	/// \date 2016/04/06 19:41
	///
	const fs::path& GetModulePath()const;

	///
	/// \brief 获取框架路径
	///
	/// \author albert.xu
	/// \date 2016/04/06 19:42
	///
	const fs::path& GetLoaderPath()const;

	///
	/// \brief 生成文件MD5列表
	///
	/// \author albert.xu
	/// \date 2016/08/05 15:13
	///
	void GenerateFileist();

	///
	/// \brief 获取文件列表
	///
	/// \author albert.xu
	/// \date 2016/08/05 15:39
	///
	const std::unordered_map< std::string, std::string >& GetFilelist( xgc_ulong id );
protected:
	///
	/// \brief 加载配置
	///
	/// \author albert.xu
	/// \date 2016/04/06 20:59
	///
	xgc_void Load();

	///
	/// \brief 生成文件MD5列表
	///
	/// \author albert.xu
	/// \date 2016/08/05 15:11
	///
	void GenerateMd5FileList( const fs::path &root, const fs::path &relative, std::unordered_map< xgc_string, xgc_string > &filelist, std::unordered_set< std::string > ignore_files );

private:
	/// 账号集合
	std::vector< account > accounts;

	/// 服务器列表
	std::vector< xgc_pair< xgc_string, xgc_string > > servers;

	/// 文件MD5映射表
	std::unordered_map< xgc_string, xgc_string > loader_filelist;

	/// 文件MD5映射表
	std::unordered_map< xgc_string, xgc_string > module_filelist;

	/// 框架的保存地址
	fs::path loader_path;
	/// 测试脚本的保存地址
	fs::path module_path;
};

inline xgc_size AppConfig::GetAccountCount()const
{
	return accounts.size();
}

inline AppConfig::account* AppConfig::GetAccount( size_t i )
{
	if( i < accounts.size() )
		return &accounts[i];

	return nullptr;
}

///
/// \brief 获取可用服务器数
///
/// \author albert.xu
/// \date 2016/04/11 17:22
///
inline xgc_size AppConfig::GetServerCount()const
{
	return servers.size();
}

///
/// \brief 根据索引获取服务器名
///
/// \author albert.xu
/// \date 2016/04/11 17:22
///
inline xgc_lpcstr AppConfig::GetServerName( xgc_size nIndex )const
{
	if( nIndex < servers.size() )
		return servers[nIndex].first.c_str();

	return nullptr;
}

///
/// \brief 根据索引获取服务器连接地址
///
/// \author albert.xu
/// \date 2016/04/11 17:22
///
inline xgc_lpcstr AppConfig::GetServerAddress( xgc_size nIndex )const
{
	if( nIndex < servers.size() )
		return servers[nIndex].second.c_str();

	return nullptr;
}

///
/// \brief 获取测试路径
///
/// \author albert.xu
/// \date 2016/04/06 19:41
///
inline const fs::path& AppConfig::GetModulePath()const
{
	return module_path;
}

///
/// \brief 获取框架路径
///
/// \author albert.xu
/// \date 2016/04/06 19:42
///
inline const fs::path& AppConfig::GetLoaderPath()const
{
	return loader_path;
}

///
/// \brief 生成文件MD5列表
///
/// \author albert.xu
/// \date 2016/08/05 15:13
///


///
/// \brief 获取应用程序配置
///
/// \author albert.xu
/// \date 2016/04/06 20:56
///
AppConfig& getConfig();