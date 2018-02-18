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

namespace xgc
{
	namespace message
	{
		extern network_t g_session;

		///
		/// \brief 连接是否断开
		///
		/// \author albert.xu
		/// \date 2016/03/14 18:31
		///
		xgc_bool is_disconnect();

		///
		/// \brief 初始化消息映射表
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:37
		///
		xgc_void init_deliver_table();

		///
		/// \brief 客户端建立连接
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnConnect( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

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
		/// \brief 服务器通知文件列表
		///
		/// \author albert.xu
		/// \date 2016/08/05 15:51
		///
		xgc_void OnFileList( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief 开始测试
		///
		/// \author albert.xu
		/// \date 2016/03/14 14:38
		///
		xgc_void OnStart( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief 停止测试
		///
		/// \author albert.xu
		/// \date 2016/03/14 14:38
		///
		xgc_void OnStop( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief 下载文件
		///
		/// \author albert.xu
		/// \date 2016/04/12 21:47
		///
		xgc_void OnNewFile( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief 下载文件
		///
		/// \author albert.xu
		/// \date 2016/04/12 21:47
		///
		xgc_void OnFileData( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );
	}
}
#endif // _PROCESSOR_H_