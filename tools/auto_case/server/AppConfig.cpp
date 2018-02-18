#include "stdafx.h"
#include "AppConfig.h"

AppConfig::AppConfig()
{
	Load();
}

AppConfig::~AppConfig()
{
}

xgc_void AppConfig::Load()
{
	xgc_char abspath[_MAX_PATH] = { 0 };
	GetNormalPath( abspath, sizeof( abspath ), "./config.ini" );

	ini_reader ini;
	ini.load( abspath );

	auto account_cnt = ini.get_item_count( "accounts" );
	for( auto i = 0; i < account_cnt; ++i )
	{
		account ac;
		auto username = ini.get_item_name( "accounts", i );
		if( nullptr == username )
			continue;

		auto password = ini.get_item_value( "accounts", i, nullptr );
		if( nullptr == password )
			continue;

		auto split = string_split( password, ", " );

		ac.username = username;
		ac.password = split[0];
		if( split.size() > 1 )
			ac.script = split[1];

		ac.used = false;
		accounts.push_back( ac );
	}

	auto server_cnt = ini.get_item_count( "servers" );
	for( auto i = 0; i < account_cnt; ++i )
	{
		auto server_name = ini.get_item_name( "servers", i );
		if( nullptr == server_name )
			continue;

		auto server_addr = ini.get_item_value( "servers", i, nullptr );
		if( nullptr == server_addr )
			continue;

		servers.push_back( std::make_pair( server_name, server_addr ) );
	}

	xgc_lpcstr pmodule_path = ini.get_item_value( "config", "module_path", "./Script/" );
	module_path = GetNormalPath( abspath, sizeof( abspath ), "%s", pmodule_path );
	xgc_lpcstr ploader_path = ini.get_item_value( "config", "loader_path", "./Script/Loader" );
	loader_path = GetNormalPath( abspath, sizeof( abspath ), "%s", ploader_path );
}

void AppConfig::GenerateMd5FileList( const fs::path &root, const fs::path &relative, std::unordered_map< xgc_string, xgc_string > &filelist, std::unordered_set< std::string > ignore_files )
{
	if( !fs::exists( root / relative ) )
		return;

	char MD5_Code[16];
	char MD5_Text[1024];

	std::unordered_set< std::string > local_ignore_files;
	fs::path ignore_file = root / relative / "ignore_files";
	while( fs::exists( ignore_file ) )
	{
		std::fstream file;

		file.open( ignore_file, std::ios_base::in );
		if( !file.is_open() )
			break;

		while( !file.eof() )
		{
			xgc_char filename[_MAX_PATH] = { 0 };
			file.getline( filename, sizeof( filename ) );
			if( filename[0] == '*' )
				ignore_files.insert( filename + 1 );
			else
				local_ignore_files.insert( filename );
		}

		file.close();
		break;
	}

	for( const auto &entry : fs::directory_iterator { root / relative } )
	{
		std::string filename = entry.path().filename().string();
		if( ignore_files.find( filename ) != ignore_files.end() )
			continue;

		if( local_ignore_files.find( filename ) != local_ignore_files.end() )
			continue;

		if( fs::is_directory( entry.path() ) )
		{
			GenerateMd5FileList( root, relative / filename, filelist, ignore_files );
		}
		else
		{
			Md5_file( entry.path().string().c_str(), MD5_Code );
			Md5_text( MD5_Code, MD5_Text );
			filelist[(relative / filename).string()] = MD5_Text;
		}
	}
}

void AppConfig::GenerateFileist()
{
	{
		auto loader_path = GetLoaderPath();
		auto loader_list = loader_path / "md5.list";

		loader_filelist.clear();
		GenerateMd5FileList( loader_path, "", loader_filelist, {"ignore_files", "md5.list"} );

		std::fstream fs;
		fs.open( loader_path / "md5.list", std::ios_base::out | std::ios_base::trunc );
		for each( auto pair in loader_filelist )
			fs << pair.second << " - " << pair.first << std::endl;
	}

	{
		auto module_path = GetModulePath();
		auto module_list = module_path / "md5.list";

		module_filelist.clear();
		GenerateMd5FileList( module_path, "", module_filelist, {"ignore_files", "md5.list"} );

		std::fstream fs;
		fs.open( module_path / "md5.list", std::ios_base::out | std::ios_base::trunc );
		for each( auto pair in module_filelist )
			fs << pair.second << " - " << pair.first << std::endl;
	}
}

const std::unordered_map<std::string, std::string>& AppConfig::GetFilelist( xgc_ulong id )
{
	// TODO: insert return statement here
	switch( id )
	{
		case 0: return loader_filelist;
		case 1: return module_filelist;
	}

	return loader_filelist;
}

AppConfig& getConfig()
{
	static AppConfig inst;
	return inst;
}
