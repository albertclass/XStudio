/*******************************************************************/
//! \file ServerGD.h
//! \brief GD指令
//! 2015/01/13 by wuhailin.jerry
/*******************************************************************/

#ifndef _SERVER_GD_H
#define _SERVER_GD_H

#pragma once

namespace ServerSvc
{
	#define SVCGD		"SVCGD"		/// "GD"消息服务
	#define SVCAWARD	"SVCAWARD"  /// 领奖消息服务  

	#define MAX_ORDERID_LEN         32  // 订单号或者用户信息字符长度
	#define MAX_TIME_LEN            19  // 时间长， 格式为：YYYY-MM-DD HH24:MI:SS
	#define MAX_BATCHID_LEN         10  // 批次号长， 格式为：CQ007
	#define MAX_DESCRIPTION_LEN     64  // 成功失败的描述信息长度
	#define	MAX_PERSIONALINFO_LEN	256	// 玩家个性化信息最大长度

	#define GDNORMALC2SREQ			0x00000001   /// "GDNORMAL"发送请求  
	#define GDNORMALC2SRES			0x80000001   /// "GDNORMAL"发送反馈  
	#define GDNORMALS2CREQ			0x08000001   /// "GDNORMAL"下发请求  
	#define GDNORMALS2CRES			0x88000001   /// "GDNORMAL"下发反馈  

	///
	/// 服务处理句柄
	/// [4/27/2015] create by albert.xu
	///
	typedef xgc_void( *fnServiceHandler )( xgc_lpvoid lpData, xgc_size nSize );

	///
	/// 初始化GD指令 
	/// [1/13/2015] create by wuhailin.jerry
	///
	xgc_bool SvcInit( IniFile &ini );

	///
	/// 注册服务处理消息
	/// [4/27/2015] create by albert.xu
	///
	xgc_void SvcRegist( xgc_lpcstr lpSvcName, fnServiceHandler lpHandler );

	///
	/// 异步调用服务功能
	/// [4/27/2015] create by albert.xu
	///
	xgc_long SvcAsynCall( xgc_lpcstr lpServiceName, xgc_lpvoid lpData, xgc_int32 nDataLen, xgc_uint32 nFlags );

	///
	/// 获取一个唯一的服务事件索引
	/// [4/27/2015] create by albert.xu
	///
	xgc_int32 SvcGetSotsUniqueId( xgc_lpstr lpOrderId, xgc_int32 nLength );
}

#endif