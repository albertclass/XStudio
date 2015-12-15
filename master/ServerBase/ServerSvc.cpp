/*******************************************************************/
//! \file ServerGD.cpp
//! \brief GD指令
//! 2015/01/13 by wuhailin.jerry
/*******************************************************************/
#include "stdafx.h"
#include "ServerSvc.h"
#include "ServerCommand.h"

#include "svccommoninfo.h"
#include "sotscommondefine.h"

#include "ServerAsyncEvt.h"

#include "sotsapi.h"

namespace ServerSvc
{
	SOTSCLIENTINFO cltinfo;

	// GD 指令使用
	typedef xgc_int32( *FunRmInitialize )( xgc_lpcstr lpFileName, pSotsMsgHandler pfnHandler );
	typedef xgc_int32( *FunRmTerminate )( );
	typedef xgc_long( *FunRmAsynCall )( xgc_lpcstr lpServiceName, xgc_lpvoid lpData, xgc_int32 nDataLen, xgc_uint32 nFlags );
	typedef xgc_int32( *FunRmGetSotsUniqueId )( xgc_lpstr lpOrderId, xgc_int32 nLength );

	FunRmInitialize          pRmInitialize = xgc_nullptr;
	FunRmTerminate           pRmTerminate = xgc_nullptr;
	FunRmAsynCall            pRmAsynCall = xgc_nullptr;
	FunRmGetSotsUniqueId     pRmGetSotsUniqueId = xgc_nullptr;

	///
	/// 异步调用服务功能
	/// [4/27/2015] create by albert.xu
	///
	xgc_long SvcAsynCall( xgc_lpcstr lpServiceName, xgc_lpvoid lpData, xgc_int32 nDataLen, xgc_uint32 nFlags )
	{
		return pRmAsynCall( lpServiceName, lpData, nDataLen, nFlags );
	}

	///
	/// 获取一个唯一的服务事件索引
	/// [4/27/2015] create by albert.xu
	///
	xgc_int32 SvcGetSotsUniqueId( xgc_lpstr lpOrderId, xgc_int32 nLength )
	{
		return pRmGetSotsUniqueId( lpOrderId, nLength );
	}

	///
	/// 服务描述
	/// [4/27/2015] create by albert.xu
	///
	struct ServiceDescript
	{
		xgc_char		 mServiceName[64];
		fnServiceHandler mServiceHandler;
	};

	/// @var 服务注册表
	static xgc_vector< ServiceDescript > gServices;

	///
	/// 注册服务处理消息
	/// [4/27/2015] create by albert.xu
	///
	xgc_void SvcRegist( xgc_lpcstr lpSvcName, fnServiceHandler lpHandler )
	{
		XGC_ASSERT_RETURN( lpSvcName && lpHandler, XGC_NONE );

		ServiceDescript desc;
		desc.mServiceHandler = lpHandler;
		strcpy_s( desc.mServiceName, lpSvcName );

		gServices.push_back( desc );
	}

	struct GdPackage
	{
		xgc_ulong mHandle;
		xgc_char  mService[64];
		xgc_int32 mSize;
		xgc_char  mData[1];
	};

#pragma pack (push,1)  /// 指定按 1 字节对齐
	struct GdNormalC2SReqDef
	{
		xgc_int32  gameIdSrc;
		xgc_int32  areaIdSrc;
		xgc_int32  groupIdSrc;
		xgc_int32  hostIdSrc;
		xgc_int32  gameIdDest;
		xgc_int32  areaIdDest;
		xgc_int32  groupIdDest;
		xgc_int32  hostIdDest;
		xgc_char   orderId[MAX_ORDERID_LEN + 1];
		xgc_int32  gdType;
		xgc_char   sendTime[MAX_TIME_LEN + 1];
		xgc_char   batchId[MAX_BATCHID_LEN + 1];
		xgc_char   begTime[MAX_TIME_LEN + 1];
		xgc_char   endTime[MAX_TIME_LEN + 1];
		xgc_int32  sendType;
		xgc_int32  sendIntervalTimes;
		xgc_int32  msgLen;
		xgc_char   msgInfo[1];
	};

	struct GdNormalC2SResDef
	{
		xgc_int32  result;
		xgc_char description[MAX_DESCRIPTION_LEN + 1];
		xgc_char orderId[MAX_ORDERID_LEN + 1];
	};

	struct GdNormalS2CReqDef
	{
		xgc_int32 gameId;
		xgc_int32 areaId;
		xgc_int32 groupId;
		xgc_int32 hostId;
		xgc_int32 sequenceId;
		xgc_int32 gdType;
		xgc_char sendTime[MAX_TIME_LEN + 1];
		xgc_char batchId[MAX_BATCHID_LEN + 1];
		xgc_int32 msgLen;
		xgc_char msgInfo[1];
	};

	struct GdNormalS2CResDef
	{
		xgc_int32 result;
		xgc_char description[MAX_DESCRIPTION_LEN + 1];
		xgc_int32 sequenceId;
	};

	struct DepositS2CReqDef
	{
		xgc_int32 gameId;
		xgc_int32 areaId;
		xgc_int32 groupId;
		xgc_int32 hostId;
		xgc_int32 amount;
		xgc_int32 uidType;
		xgc_char  userId[MAX_ORDERID_LEN + 1];
		xgc_char  orderId[MAX_ORDERID_LEN + 1];
		xgc_char roleId[MAX_ORDERID_LEN + 1];
		xgc_char rolename[MAX_ORDERID_LEN + 1];
		xgc_char depositTime[MAX_TIME_LEN + 1];
	};

	struct DepositS2CResDef
	{
		xgc_int32 confirm;
		xgc_char description[MAX_DESCRIPTION_LEN + 1];
		xgc_char  orderId[MAX_ORDERID_LEN + 1];
	};

	struct PersonalC2SReqDef
	{
		xgc_int32 gameId;
		xgc_int32 areaId;
		xgc_int32 groupId;
		xgc_int32 hostId;
		xgc_char  orderId[MAX_ORDERID_LEN + 1];
		xgc_char  userId[MAX_ORDERID_LEN + 1];
		xgc_char  rolename[MAX_ORDERID_LEN + 1];

	};

	struct PersonalC2SResDef
	{
		xgc_int32 result;
		xgc_char  desc[MAX_DESCRIPTION_LEN + 1];
		xgc_char  orderId[MAX_ORDERID_LEN + 1];
		xgc_char  userId[MAX_ORDERID_LEN + 1];
		xgc_char rolename[MAX_ORDERID_LEN + 1];
		xgc_char personalinfo[MAX_PERSIONALINFO_LEN + 1];
	};

	struct GdNormalC2SInfoReqDef
	{
		SotsMsgHeadDef msgHead;
		GdNormalC2SReqDef msgBody;
	};

	struct GdNormalC2SInfoResDef
	{
		SotsMsgHeadDef msgHead;
		GdNormalC2SResDef msgBody;
	};

	struct GdNormalS2CInfoReqDef
	{
		SotsMsgHeadDef msgHead;
		GdNormalS2CReqDef msgBody;
	};

	struct GdNormalS2CInfoResDef
	{
		SotsMsgHeadDef msgHead;
		GdNormalS2CResDef msgBody;
	};

	struct DepositS2CInfoReqDef
	{
		SotsMsgHeadDef msgHead;
		DepositS2CReqDef msgBody;
	};

	struct DepositS2CInfoResDef
	{
		SotsMsgHeadDef msgHead;
		DepositS2CResDef msgBody;
	};

	struct ChallengeSotsMsgHeadDef
	{
		xgc_int32 msgLen;  /// 整个消息包的字节长度，包括消息头和消息体                    
		xgc_int32 msgType;  /// 消息类型，处理同一服务的各种来回消息的类型区分              
		xgc_char msgId[32 + 1];  /// 消息包序列号,主动发送方提供，反馈消息与请求消息msgId保持一致
		xgc_int32 callTime;  /// 发送时间                                                    
		xgc_int32 version;  /// 消息协议的版本信息                    
	};  // 奇遇值的消息头是1字节对齐的

	struct PersonalC2SReqPKG
	{
		ChallengeSotsMsgHeadDef msgHead;
		PersonalC2SReqDef msgBody;
	};

	struct PersonalC2SAckPKG
	{
		ChallengeSotsMsgHeadDef msgHead;
		PersonalC2SResDef msgBody;
	};
#pragma pack (pop) /// 取消指定对齐，恢复缺省对齐

	///
	/// 服务处理函数(回调 主线程)
	/// [4/25/2015] create by albert.xu
	///
	xgc_void HandleSotsCallback( GdPackage *pPackage );

	///
	///  服务处理函数 异步线程
	/// [6/2/2015] create by wuhailin.jerry
	///
	xgc_void OnSotsCallback( xgc_ulong handle, xgc_char* pServiceName, xgc_char *pData, xgc_int32 len );

	///
	/// 初始化GD指令 
	/// [1/13/2015] create by wuhailin.jerry
	///
	xgc_bool SvcInit( IniFile &ini )
	{
		FUNCTION_BEGIN;

		xgc_char szPath[MAX_PATH] = { 0 };

		GetNormalPath( szPath, sizeof( szPath ), "sotsapi.dll" );

		HINSTANCE hInst = ::LoadLibraryA( szPath );
		if( xgc_nullptr == hInst )
		{
			SYS_ERROR( "connot found file. dll = %s", szPath );
			return false;
		}

		pRmInitialize = (FunRmInitialize) GetProcAddress( hInst, "SotsInitialize" );
		pRmTerminate = (FunRmTerminate) GetProcAddress( hInst, "SotsTerminate" );
		pRmAsynCall = (FunRmAsynCall) GetProcAddress( hInst, "SotsAsynCall" );
		pRmGetSotsUniqueId = (FunRmGetSotsUniqueId) GetProcAddress( hInst, "GetSotsUniqueId" );

		xgc_lpcstr lpConfFile = ini.GetItemValue( "ToolGetConfig", "ConfigPath", "ToolsGetConfig.ini" );

		GetNormalPath( szPath, sizeof( szPath ), lpConfFile );

		if( pRmInitialize( szPath, OnSotsCallback ) != 1 )
		{
			SYS_ERROR( "DebugCommand::pRmInitialize failed. config file = %s", szPath );
			return false;
		}

		return true;
		FUNCTION_END;
		return false;
	}

	///
	/// 网络字节序转为主机字节序
	/// [1/13/2015] create by wuhailin.jerry
	///
	xgc_void MsgNToHInfo( SotsMsgHeadDef *pHead )
	{
		pHead->msgLen = ntohl( pHead->msgLen );
		pHead->msgType = ntohl( pHead->msgType );
		pHead->callTime = ntohl( pHead->callTime );
		pHead->version = ntohl( pHead->version );
	}

	///
	/// 网络字节序转为主机字节序
	/// [1/13/2015] create by wuhailin.jerry
	///
	xgc_void MsgNToHInfo( GdNormalS2CReqDef *pBody )
	{
		pBody->gameId = ntohl( pBody->gameId );
		pBody->areaId = ntohl( pBody->areaId );
		pBody->groupId = ntohl( pBody->groupId );
		pBody->hostId = ntohl( pBody->hostId );
		pBody->sequenceId = ntohl( pBody->sequenceId );
		pBody->gdType = ntohl( pBody->gdType );
		pBody->msgLen = ntohl( pBody->msgLen );
	}

	///
	/// 网络字节序转为主机字节序
	/// [1/13/2015] create by wuhailin.jerry
	///
	xgc_void MsgNToHInfo( GdNormalS2CInfoReqDef *pReq )
	{
		MsgNToHInfo( &pReq->msgHead );
		MsgNToHInfo( &pReq->msgBody );
	}

	///
	/// 主机字节序转为网络字节序 
	/// [1/14/2015] create by wuhailin.jerry
	///
	xgc_void MsgHToNInfo( SotsMsgHeadDef *p )
	{
		p->msgLen = htonl( p->msgLen );
		p->msgType = htonl( p->msgType );
		p->callTime = htonl( p->callTime );
		p->version = htonl( p->version );
	}

	///
	/// 主机字节序转为网络字节序 
	/// [1/14/2015] create by wuhailin.jerry
	///
	xgc_void MsgHToNInfo( GdNormalS2CResDef *p )
	{
		p->result = htonl( p->result );
		p->sequenceId = htonl( p->sequenceId );
	}

	///
	/// 主机字节序转为网络字节序 
	/// [1/14/2015] create by wuhailin.jerry
	///
	xgc_void MsgHToNInfo( GdNormalS2CInfoResDef * p )
	{
		MsgHToNInfo( &p->msgHead );
		MsgHToNInfo( &p->msgBody );
	}

	xgc_void OnSotsCallback( xgc_ulong handle, xgc_char* pServiceName, xgc_char *pData, xgc_int32 nLen )
	{
		FUNCTION_BEGIN;

		GdPackage *pPackage = ( GdPackage *)malloc( sizeof(GdPackage) + nLen );
		pPackage->mHandle = handle;
		strcpy_s( pPackage->mService, pServiceName );
		pPackage->mSize = nLen;
		memcpy_s( pPackage->mData, nLen, pData, nLen );

		PostServerEvent( std::bind( HandleSotsCallback, pPackage ) );

		FUNCTION_END;
	}

	xgc_void HandleSotsCallback( GdPackage *pData )
	{
		std::unique_ptr< GdPackage, decltype(free)* > pPackage( pData, free );

		FUNCTION_BEGIN;

		xgc_ulong hService     = pPackage->mHandle;
		xgc_lpstr pServiceName = pPackage->mService;
		xgc_lpstr pData        = pPackage->mData;
		xgc_int32 nSize        = pPackage->mSize;

		if( xgc_nullptr == pServiceName || xgc_nullptr == pData )
			return;

		SYS_INFO( "handle[%u] servicename[%s] len[%d] start", hService, pServiceName, nSize );

		//
		// Get configuration. Don't Change.
		//
		if( 0 == strcmp( pServiceName, SVCSOTSSYSTEM ) )
		{
			xgc_char msgInfo[2048] = { 0 };

			SotsClientInfoDef *p = (SotsClientInfoDef *) msgInfo;
			memcpy( msgInfo, pData, nSize );
			if( p->msgHead.msgType == TYPESOTSCLIENTINFO )
			{
				memcpy( &cltinfo, &p->msgBody, sizeof( SOTSCLIENTINFO ) );
			}

			return;
		}

		// 查找已注册的服务
		for each( auto &it in gServices )
		{
			if( 0 == strcmp( pServiceName, it.mServiceName ) )
			{
				SYS_INFO( "收到服务[%s]的回应", it.mServiceName );
				it.mServiceHandler( pData, nSize );
				return;
			}
		}

		xgc_char chMsg[4096] = { 0 };
		// 未找到注册的服务,认为是GD指令
		GdNormalS2CInfoReqDef *msgInfoReq = (GdNormalS2CInfoReqDef *) pData;

		// 处理接收消息
		MsgNToHInfo( msgInfoReq );

		XGC_ASSERT_RETURN( msgInfoReq->msgHead.msgLen < sizeof( chMsg ), XGC_NONE );

		GdNormalS2CReqDef &stReq = msgInfoReq->msgBody;

		memcpy_s( chMsg, sizeof( chMsg ), (xgc_char *) stReq.msgInfo, stReq.msgLen );

		SYS_INFO( "gameId[%d] areaId[%d] groupId[%d] hostId[%d] sequenceId[%d] gdType[%d] sendTime[%s] batchId[%s] msgLen[%d] msgInfo[%s], chMsg:[%s]",
			stReq.gameId,
			stReq.areaId,
			stReq.groupId,
			stReq.hostId,
			stReq.sequenceId,
			stReq.gdType,
			stReq.sendTime,
			stReq.batchId,
			stReq.msgLen,
			stReq.msgInfo,
			chMsg );

		/// 处理反馈消息
		xgc_bool bRes = false;
		xgc_string strToken = "#";
		xgc_vector<xgc_string> strParams = split( chMsg, strToken );

		//暂时屏蔽掉
		for( auto &iter : strParams )
		{
			if( !iter.empty() )
			{
				DebugCommand::CommandOperator Current;
				Current.user  = xgc_nullptr;
				Current.name  = "GD";
				Current.group = 1; // 超级权限
				Current.print = xgc_nullptr;

				SYS_INFO( "执行GD指令:[%s]", iter.c_str() );
				bRes = DebugCommand::ProcessDebugCmd( "GD", &Current, iter.c_str() );
			}

			SYS_INFO( "msginfo[%s], result[%s]", iter.c_str(), bRes ? "成功" : "失败" );
		}

		GdNormalS2CInfoResDef msgInfoRpn;
		memset( &msgInfoRpn, 0, sizeof( msgInfoRpn ) );

		msgInfoRpn.msgHead.msgLen = sizeof( GdNormalS2CInfoResDef );
		msgInfoRpn.msgHead.msgType = GDNORMALS2CRES;
		msgInfoRpn.msgHead.version = msgInfoReq->msgHead.version;

		msgInfoRpn.msgHead.callTime = (xgc_int32) current_time();
		strcpy_s( msgInfoRpn.msgHead.msgId, msgInfoReq->msgHead.msgId );

		msgInfoRpn.msgBody.result = bRes;
		msgInfoRpn.msgBody.sequenceId = msgInfoReq->msgBody.sequenceId;
		strcpy_s( msgInfoRpn.msgBody.description, "" );

		SYS_INFO( "send response : msgLen:[%d], msgType:[%d], msgId:[%s], callTime:[%d], version:[%d]"
			"result:[%d],sequenceId:[%d], description:[%s]",
			msgInfoRpn.msgHead.msgLen,
			msgInfoRpn.msgHead.msgType,
			msgInfoRpn.msgHead.msgId,
			msgInfoRpn.msgHead.callTime,
			msgInfoRpn.msgHead.version,
			msgInfoRpn.msgBody.result,
			msgInfoRpn.msgBody.sequenceId,
			msgInfoRpn.msgBody.description );

		auto nLen = msgInfoRpn.msgHead.msgLen;
		MsgHToNInfo( &msgInfoRpn );

		xgc_ulong hRpc = pRmAsynCall( SVCGD, &msgInfoRpn, nLen, 0 );
		SYS_INFO( "GD with hRpc:%ul, msgLen:%d", hRpc, nLen );
		FUNCTION_END;
	}
}