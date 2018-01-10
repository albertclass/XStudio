#include "ServerDefines.h"
#include "ServerParams.h"
#include "ServerDatabase.h"

const char* params_version = "2017.12.11";

/// @var ������ȫ�ֲ���
static xgc::unordered_map< xgc::string, stGlobalParam > G_Params;

/// @var ȫ�ֻص�
static PF_NotifyCallback G_NotifyCallback = xgc_nullptr;

/// @var ģʽ
static xgc_long G_ParamsMode = 0;

/// @var ���������
static xgc_lpvoid G_Server = xgc_nullptr;

/// @var �ͻ��˾��
static xgc::set< network_t > G_Clients;

/// @var ȫ��������λ��
static xgc::string G_ParamsFile;

///
/// \brief ���ӻỰ�ӿ�
///
/// \author albert.xu
/// \date 2017/03/01 10:41
///
class CParamsSession : public INetworkSession
{
private:
	/// ���Ӿ��
	network_t handle_;

public:
	///
	/// \brief ���� 
	/// \date 12/5/2017
	/// \author xufeng04
	///
	CParamsSession()
	{

	}

	///
	/// \brief ����
	/// \date 12/5/2017
	/// \author xufeng04
	///
	~CParamsSession()
	{

	}

	///
	/// \brief ���ݰ��Ƿ�
	/// \return	���ݰ���С
	///
	virtual int OnParsePacket( const void* data, xgc_size size )
	{
		if( size < sizeof( xgc_uint16 ) )
			return 0;

		return ntohs( *(xgc_uint16*)data );
	}

	///
	/// \brief ���ӽ���
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnAccept( net::network_t handle )
	{
		handle_ = handle;
		G_Clients.insert( handle );
		SendCommand( "version %s", params_version );
	}

	///
	/// \brief ���ӽ���
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnConnect( net::network_t handle )
	{
		handle_ = handle;
	}

	///
	/// \brief ���Ӵ���
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:09
	///
	virtual xgc_void OnError( xgc_int16 error_type, xgc_int16 error_code )
	{
		SYS_ERR( "Params Socket Error: type = %d, code = %d", error_type, error_code );
	}

	///
	/// \brief ���ӹر�
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:10
	///
	virtual xgc_void OnClose()
	{

	}

	///
	/// \brief ��������
	///
	/// \author albert.xu
	/// \date 2017/02/28 11:10
	///
	virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size )
	{
		auto len = *(xgc_uint16 *)data;

		ParseCommand( (char*)data + sizeof( xgc_uint16 ), size - sizeof( xgc_uint16 ) );
	}

	///
	/// \brief ���籣���¼�
	///
	/// \author albert.xu
	/// \date 2017/03/03 10:41
	///
	virtual xgc_void OnAlive()
	{
		SendCommand( "ping %llu", datetime::now().to_milliseconds() );
	}

	///
	/// \brief ������Ϣ 
	/// \date 12/5/2017
	/// \author xufeng04
	///
	xgc_void SendCommand( xgc_lpcstr fmt, ... )
	{
		char buf[1024] = { 0 };
		va_list ap;
		va_start( ap, fmt );
		int cpy = vsprintf_s( buf, fmt, ap );
		va_end( ap );

		auto len = xgc_uint16( cpy );

		SendPacketChains( handle_, {
			{ MakeBuffer( len ) },
			{ MakeBuffer( buf, len + 1 ) },
		} );
	}

	///
	/// \brief ���������� 
	/// \date 12/5/2017
	/// \author xufeng04
	///
	xgc_void ParseCommand( xgc_lpstr str, xgc_size len )
	{
		auto args = string_split( str, ", " );
		if( args.size() < 1 )
			return;

		auto &command = args[0];

		if( G_ParamsMode == 1 )
		{
			if( command == "sync" )
			{
				char szDateTime[64] = { 0 };

				for( auto &kv : G_Params )
				{
					auto &info = kv.second;
					SendCommand( "set %s %s %08X %s", info.key.c_str(), info.val.c_str(), info.mask, szDateTime );
				}
			}
		}
		else if( G_ParamsMode == 2 )
		{
			if( command == "version" )
			{
				if( args[1] != params_version )
					CloseLink( handle_ );
			}
		}
		
		if( command == "set" )
		{
			auto it = G_Params.find( args[1] );
			if( it != G_Params.end() )
			{
				datetime update = datetime::convert( args[4].c_str() );
				stGlobalParam &info = it->second;
				if( update > info.update )
				{
					info.val = args[2];
					info.mask = str2numeric< xgc_uint32 >( args[3].c_str(), xgc_nullptr, 16 );
					info.update = update;

					G_NotifyCallback( info, false );
				}
			}
			else
			{
				stGlobalParam info;
				info.key = args[1];
				info.val = args[2];
				info.mask = str2numeric< xgc_uint32 >( args[3].c_str(), xgc_nullptr, 16 );
				info.update = datetime::convert( args[4].c_str() );

				G_NotifyCallback( info, true );
			}
		}
		else if( command == "get" )
		{
			auto it = G_Params.find( args[1] );
			if( it != G_Params.end() )
			{
				char szDateTime[64] = { 0 };

				auto &info = it->second;
				info.update.to_string( szDateTime );
				SendCommand( "set %s %s %08X %s", info.key.c_str(), info.val.c_str(), info.mask, szDateTime );
			}
		}
	}
};

/// @var �ͻ��˶���
static CParamsSession *GP_Client = XGC_NEW CParamsSession();

///
/// ����ͬ���ص�
/// [12/19/2014] create by albert.xu
///
xgc_void SetGlobalParameterNotifier( PF_NotifyCallback pfnNotifier )
{
	G_NotifyCallback = pfnNotifier;
}

///
/// �������ļ�����
/// [12/19/2014] create by albert.xu
///
static xgc_bool Load( xgc_lpcstr file )
{
	csv_reader csv;
	csv.load( file, ',', false );

	auto rows = csv.get_rows();
	
	for( decltype( rows ) row = 0; row < rows; ++row )
	{
		// �����������ȡ�����������Թ�
		xgc_lpcstr lpKey = csv.get_value( row, size_t(0), xgc_nullptr );
		xgc_lpcstr lpVal = csv.get_value( row, size_t(1), xgc_nullptr );

		XGC_ASSERT_CONTINUE( lpKey && lpVal );

		// �����ֱ����������ض���
		// �ж��Ƿ��Ѷ�ȡ�÷�����������
		xgc_uint32 nMask = csv.get_value( row, 2ULL, 0U );
		
		// XGC_ASSERT_MESSAGE( nMask, "ȫ�ֱ���δ�������ԣ��⽫���¸����Խ��ڱ��ط�������Ч��" );
		SetGlobalParameter( lpKey, lpVal, nMask );
	}
	return true;
}

static xgc_void Save( xgc_lpcstr file )
{
	std::fstream fs( file, std::ios_base::out | std::ios_base::trunc );

	for( auto &kv : G_Params )
	{
		if( XGC_CHK_FLAGS( kv.second.mask, GLOBAL_PARAM_MASK_HOLDON ) )
		{
			fs
				<< kv.second.key << ","
				<< kv.second.val << ","
				<< kv.second.mask << std::endl;
		}
	}

	fs.flush();

	fs.close();
}

///
/// ��ʼ��ȫ�ֱ�����
/// [12/19/2014] create by albert.xu
///
xgc_bool InitGlobalParams( ini_reader& ini )
{
	if( false == ini.is_exist_section( "ServerParams" ) )
		return false;

	auto host = ini.get_item_value( "ServerParams", "host", "127.0.0.1" );
	auto port = ini.get_item_value( "ServerParams", "port", 6379 );

	auto mode = ini.get_item_value( "Serverparams", "mode", "client" );
	auto file = ini.get_item_value( "Serverparams", "file", "params" );

	if( strcasecmp( mode, "server" ) == 0 )
	{
		char path[XGC_MAX_PATH] = { 0 };

		G_ParamsMode = 1;
		get_absolute_path( path, "%s", file );

		if( false == Load( path ) )
			return false;

		G_ParamsFile = path;

		server_options options;
		memset( &options, 0, sizeof( options ) );
		options.acceptor_count = 10;
		options.acceptor_smart = true;
		options.heartbeat_interval = 1000;

		options.recv_buffer_size = 4096;
		options.send_buffer_size = 4096;

		options.recv_packet_max = 1024;
		options.send_packet_max = 1024;
		
		G_Server = StartServer( host, port, &options, [](){ return XGC_NEW CParamsSession(); } );
	}
	else if( strcasecmp( mode, "client" ) == 0 )
	{
		G_ParamsMode = 2;

		connect_options options;
		options.is_async = true;
		options.is_reconnect_passive = true;
		options.is_reconnect_timeout = true;
		options.recv_buffer_size = 4096;
		options.send_buffer_size = 4096;

		options.recv_packet_max = 1024;
		options.send_packet_max = 1024;

		Connect( host, port, GP_Client, &options );
	}

	return true;
}

///
/// Ĭ�ϵ�Fillter
/// [1/29/2015] create by albert.xu
///
xgc_bool GetGlobalParameter_DefaultFillter( const stGlobalParam& st, xgc_lpcstr lpKey, xgc_uint32 nMask )
{
	if( lpKey )
	{
		if( xgc_nullptr == strstr( st.key.c_str(), lpKey ) )
			return false;
	}

	if( nMask )
	{
		if( 0 == ( st.mask & nMask ) )
			return false;
	}

	return true;
}

///
/// ��ȡ����������ȫ�ֱ���
/// [1/16/2015] create by albert.xu
///
xgc::vector< stGlobalParam > GetGlobalParameter( const std::function< xgc_bool( const stGlobalParam& ) > &pfnFillter )
{
	xgc::vector< stGlobalParam > Container;
	FUNCTION_BEGIN;
	for ( auto const &it : G_Params )
	{
		if( pfnFillter( it.second ) )
			Container.push_back( it.second );
	}

	FUNCTION_END;
	return Container;
}

///
/// ��ȡ�ַ����͵�ȫ�ֱ���
/// [8/27/2014] create by albert.xu
///
xgc_lpcstr GetGlobalParameter( xgc_lpcstr lpKey, xgc_lpcstr lpDefault )
{
	auto it = G_Params.find( lpKey );
	if( it != G_Params.end() )
		lpDefault = it->second.val.c_str();

	return lpDefault;
}

///
/// ��ȡ������ȫ�ֱ���
/// [8/27/2014] create by albert.xu
///
xgc_bool GetGlobalParameter( xgc_lpcstr lpKey, xgc_bool bDefault )
{
	xgc_lpcstr lpValue = GetGlobalParameter( lpKey, xgc_nullptr );
	if( lpValue == xgc_nullptr )
		return bDefault;

	if( strcasecmp( "true", lpValue ) == 0 )
		return true;

	if( str2numeric< int >( lpValue ) != 0 )
		return true;

	return false;
}

///
/// �����ַ����͵�ȫ�ֱ�������������������
/// [8/27/2014] create by albert.xu
///
xgc_bool SetGlobalParameter( xgc_lpcstr lpKey, xgc_lpcstr lpValue, xgc_uint32 nMask )
{
	auto bNew = false;
	auto it = G_Params.find( lpKey );
	if( it == G_Params.end() )
	{
		bNew = true;

		if( xgc_nullptr == lpValue )
			return false;

		// û�������
		stGlobalParam stParam;
		stParam.key = lpKey;
		stParam.val = lpValue;
		stParam.mask = nMask & 0xffff;
		stParam.update = datetime::now();

		auto ib = G_Params.insert( std::make_pair( lpKey, stParam ) );
		XGC_ASSERT_RETURN( ib.second, false );
		it = ib.first;
	}

	it->second.val = lpValue;
	it->second.update = datetime::now();

	if( XGC_CHK_FLAGS( nMask, GLOBAL_PARAM_MASK_NOTIFY ) && G_NotifyCallback )
		G_NotifyCallback( it->second, bNew );

	if( G_ParamsMode == 1)
	{
		char szDateTime[64] = { 0 };
		datetime::now( szDateTime );

		Save( G_ParamsFile.c_str() );

		// Server Mode, Sync Param to every client.
		for( auto handle : G_Clients )
		{
			Param_GetSession netOperatorParams;
			netOperatorParams.handle = handle;
			netOperatorParams.session = xgc_nullptr;

			if( 0 == ExecuteState( Operator_GetSession, &netOperatorParams ) )
			{
				CParamsSession* pSession = (CParamsSession*)netOperatorParams.session;
				if( pSession )
					pSession->SendCommand( "set %s %s %08X %s", lpKey, lpValue, nMask, szDateTime );
			}
		}
	}
	
	return true;
}

///
/// ���ÿ�����ȫ�ֱ���
/// [8/27/2014] create by albert.xu
///
xgc_bool SetGlobalParameter( xgc_lpcstr lpKey, xgc_bool bValue, xgc_uint32 nMask )
{
	return SetGlobalParameter( lpKey, bValue ? "true" : "false", nMask );
}
