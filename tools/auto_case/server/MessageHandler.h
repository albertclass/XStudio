///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file Processor.h
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 消息处理
///
///////////////////////////////////////////////////////////////
#pragma once

#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

using namespace xgc::net;

namespace xgc
{
	namespace message
	{
		///
		/// \brief 初始化消息映射表
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:37
		///
		xgc_void InitProcessor();

		///
		/// \brief 客户端建立连接
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnAccept( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief 客户端断开连接
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClose( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief 客户端连接发生错误
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnError( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief 客户端定时同步
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClientInfoRpt( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );
		
		///
		/// \brief 客户端开启回应
		///
		/// \author albert.xu
		/// \date 2016/04/18 19:30
		///
		xgc_void OnClientStart( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );
		
		///
		/// \brief 同步任务的资源使用情况
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClientResNtf( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );
		
		///
		/// \brief 同步任务状态
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClientStatusNtf( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );
		
		///
		/// \brief 同步任务进度
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnProgressNtf( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief 文件下载请求
		///
		/// \author albert.xu
		/// \date 2016/04/13 17:28
		///
		xgc_void OnFilesReq( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief 文件创建回应
		///
		/// \author albert.xu
		/// \date 2016/03/15 13:48
		///
		xgc_void OnNewFileAck( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief 文件传输回应
		///
		/// \author albert.xu
		/// \date 2016/04/13 17:23
		///
		xgc_void OnFileDataAck( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

	}
}
#endif // _PROCESSOR_H_