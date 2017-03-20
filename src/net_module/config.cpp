#include "config.h"
namespace net_module
{
	///
	/// \brief net module 数据结构
	///
	/// \author albert.xu
	/// \date 2017/03/20 11:41
	///
	struct net_module_data
	{
		/// 服务器标识
		xgc_char szNetworkId[64] = { 0 };

		/// 服务器数值标识
		NETWORK_ID nNetworkId;
	};

	static struct net_module_data _net_module;

	///
	/// \brief 设置网络标识
	///
	/// \author albert.xu
	/// \date 2017/03/20 11:48
	///
	NETWORK_ID _SetNetworkID( xgc_lpcstr pNetworkId )
	{
		strcpy_s( _net_module.szNetworkId, pNetworkId );

		_net_module.nNetworkId = _Str2NetworkID( pNetworkId );

		return _net_module.nNetworkId;
	}

	///
	/// \brief 获取网络标识
	///
	/// \author albert.xu
	/// \date 2017/03/20 10:52
	///
	NETWORK_ID _GetNetworkID()
	{
		return _net_module.nNetworkId;
	}

	///
	/// \brief 转换网络标识为数值
	///
	/// \author albert.xu
	/// \date 2017/03/20 10:52
	///
	NETWORK_ID _Str2NetworkID( xgc_lpcstr pNetworkId )
	{
		xgc_lpcstr p = pNetworkId;
		xgc_uint32 s = 0;
		xgc_uint8 *c = (xgc_uint8*) &s;
		for( int i = 0; i < sizeof( s ) && *p; ++i, ++p, ++c )
		{
			xgc_long v = strtoul( p, (char**) &p, 10 );
			XGC_ASSERT_MESSAGE( v != 0 && v < 256U, "服务器ID设置错误%s", pNetworkId );
			*c = (xgc_byte) v;
		}

		return s;
	}

	///
	/// \brief 转换数值为网络标识
	///
	/// \author albert.xu
	/// \date 2017/03/20 10:53
	///
	xgc_lpcstr _NetworkID2Str( NETWORK_ID nNetworkId, xgc_char szOutput[NETWORK_REGION_S] )
	{
		NETWORK_ID ID = _GetNetworkID();
		NETWORK_REGION *N = (NETWORK_REGION*) &ID;

		int cpy = sprintf_s( szOutput, 16, "%d-%d-%d-%d", N[0], N[1], N[2], N[3] );
		if( cpy <= 0 )
		{
			return xgc_nullptr;
		}

		return szOutput;
	}
}