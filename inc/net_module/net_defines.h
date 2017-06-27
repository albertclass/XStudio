#pragma once
#ifndef _NET_DEFINES_H_ 
#define _NET_DEFINES_H_ 
typedef xgc_uint64 NETWORK_ID;
typedef xgc_uint16 NETWORK_REGION;

#define NETWORK_REGION_N 4
#define NETWORK_REGION_S 32

#define NETWORK_AREA( ID )	(((NETWORK_REGION*)ID)[0])
#define NETWORK_GROUP( ID )	(((NETWORK_REGION*)ID)[1])
#define NETWORK_CLASS( ID )	(((NETWORK_REGION*)ID)[2])
#define NETWORK_INDEX( ID )	(((NETWORK_REGION*)ID)[3])

#if defined( _WINDOWS )
#	ifdef _LIB_EXPORTS
#	 ifdef _DLL
#		define NET_MODULE_API __declspec(dllexport)
#		define __LZO_EXPORT1 __declspec(dllexport)
#	 elif defined( _LIB )
#		define NET_MODULE_API
#	 endif
#	else
#	 ifdef _NET_MODULE_STATIC
#		define NET_MODULE_API
#	 else
#		define NET_MODULE_API __declspec(dllimport)
#		define __LZO_EXPORT1 __declspec(dllimport)
#	 endif
#	endif
#elif defined( __GNUC__ )
#	if defined( _LIB_EXPORTS ) && defined( _DLL )
#		define NET_MODULE_API __attribute__((__visibility__("default")))
#		define __LZO_EXPORT1 __attribute__((__visibility__("default")))
#	else
#		define NET_MODULE_API
#	endif
#else 
#	define NET_MODULE_API
#endif

namespace net_module
{
	class CClientSession;
	class CRelaySession;
	class CRelayCliSession;
	class CPipeSession;

	struct net_module_data;

	///
	/// 消息回调
	/// [11/27/2014] create by albert.xu
	///
	typedef xgc_ulong( *ClientMsgHandler )(CClientSession*, xgc_lpvoid, xgc_size, xgc_uint32);

	///
	/// 消息回调
	/// [11/27/2014] create by albert.xu
	///
	typedef xgc_ulong( *PipeMsgHandler )(CPipeSession*, xgc_lpvoid, xgc_size, xgc_uint32);

	///
	/// 消息回调
	/// [11/27/2014] create by albert.xu
	///
	typedef xgc_ulong( *SockMsgHandler )(CRelaySession*, xgc_lpvoid, xgc_size, xgc_uint32);

	///
	/// 事件回调
	/// [11/27/2014] create by albert.xu
	///
	typedef xgc_ulong( *ClientEvtHandler )(CClientSession*, xgc_uint32, xgc_uint64);

	///
	/// 事件回调
	/// [11/27/2014] create by albert.xu
	///
	typedef xgc_ulong( *PipeEvtHandler )(CPipeSession*, xgc_uint32, xgc_uint64);

	///
	/// 事件回调
	/// [11/27/2014] create by albert.xu
	///
	typedef xgc_ulong( *SockEvtHandler )(CRelaySession*, xgc_uint32, xgc_uint64);

	///
	/// \brief 获取程序运行时间
	///
	/// \author albert.xu
	/// \date 2017/03/10 17:45
	///
	xgc_time64 tick();
}

#endif // _NET_DEFINES_H_ 