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
	/// \brief ��ȡԤ���˺�����
	///
	/// \author albert.xu
	/// \date 2016/04/06 19:40
	///
	xgc_size GetAccountCount()const;

	///
	/// \brief ��ȡ�˺�
	///
	/// \author albert.xu
	/// \date 2016/04/06 19:40
	///
	account* GetAccount( size_t i );

	///
	/// \brief ��ȡ���÷�������
	///
	/// \author albert.xu
	/// \date 2016/04/11 17:22
	///
	xgc_size GetServerCount()const;

	///
	/// \brief ����������ȡ��������
	///
	/// \author albert.xu
	/// \date 2016/04/11 17:22
	///
	xgc_lpcstr GetServerName( xgc_size nIndex )const;

	///
	/// \brief ����������ȡ���������ӵ�ַ
	///
	/// \author albert.xu
	/// \date 2016/04/11 17:22
	///
	xgc_lpcstr GetServerAddress( xgc_size nIndex )const;

	///
	/// \brief ��ȡ����·��
	///
	/// \author albert.xu
	/// \date 2016/04/06 19:41
	///
	const fs::path& GetModulePath()const;

	///
	/// \brief ��ȡ���·��
	///
	/// \author albert.xu
	/// \date 2016/04/06 19:42
	///
	const fs::path& GetLoaderPath()const;

	///
	/// \brief �����ļ�MD5�б�
	///
	/// \author albert.xu
	/// \date 2016/08/05 15:13
	///
	void GenerateFileist();

	///
	/// \brief ��ȡ�ļ��б�
	///
	/// \author albert.xu
	/// \date 2016/08/05 15:39
	///
	const std::unordered_map< std::string, std::string >& GetFilelist( xgc_ulong id );
protected:
	///
	/// \brief ��������
	///
	/// \author albert.xu
	/// \date 2016/04/06 20:59
	///
	xgc_void Load();

	///
	/// \brief �����ļ�MD5�б�
	///
	/// \author albert.xu
	/// \date 2016/08/05 15:11
	///
	void GenerateMd5FileList( const fs::path &root, const fs::path &relative, std::unordered_map< xgc_string, xgc_string > &filelist, std::unordered_set< std::string > ignore_files );

private:
	/// �˺ż���
	std::vector< account > accounts;

	/// �������б�
	std::vector< xgc_pair< xgc_string, xgc_string > > servers;

	/// �ļ�MD5ӳ���
	std::unordered_map< xgc_string, xgc_string > loader_filelist;

	/// �ļ�MD5ӳ���
	std::unordered_map< xgc_string, xgc_string > module_filelist;

	/// ��ܵı����ַ
	fs::path loader_path;
	/// ���Խű��ı����ַ
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
/// \brief ��ȡ���÷�������
///
/// \author albert.xu
/// \date 2016/04/11 17:22
///
inline xgc_size AppConfig::GetServerCount()const
{
	return servers.size();
}

///
/// \brief ����������ȡ��������
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
/// \brief ����������ȡ���������ӵ�ַ
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
/// \brief ��ȡ����·��
///
/// \author albert.xu
/// \date 2016/04/06 19:41
///
inline const fs::path& AppConfig::GetModulePath()const
{
	return module_path;
}

///
/// \brief ��ȡ���·��
///
/// \author albert.xu
/// \date 2016/04/06 19:42
///
inline const fs::path& AppConfig::GetLoaderPath()const
{
	return loader_path;
}

///
/// \brief �����ļ�MD5�б�
///
/// \author albert.xu
/// \date 2016/08/05 15:13
///


///
/// \brief ��ȡӦ�ó�������
///
/// \author albert.xu
/// \date 2016/04/06 20:56
///
AppConfig& getConfig();