#ifndef __LUA_DEBUGER_SOCK_H__
#define __LUA_DEBUGER_SOCK_H__
#include "defines.h"
#include "xnet.h"
#include "xutility.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>

using namespace xgc;
using namespace xgc::common;

struct debuger
{
	SOCKET sock;
	/// 发送字节数
	std::atomic_int send;
	/// 接收字节数
	std::atomic_int recv;
	/// 包长度
	std::atomic_int ipkg;

	/// 发送缓冲
	char send_buffer[1024 * 4];
	/// 接收缓冲
	char recv_buffer[1024 * 4];

	/// 是否退出
	std::atomic_bool exit;

	/// 调试器监听线程
	std::thread client;

	/// 同步对象
	std::mutex mtx;
	/// 同步条件
	std::condition_variable signal;

	/// 调试命令按行存储
	list< string > response;
};

extern debuger cli;

/// 等待信号量
void wait_signal();

/// 调试客户端开启
void client( int port );

/// 发送消息给调试器
void request( const void* data, int size );

#endif // __LUA_DEBUGER_SOCK_H__