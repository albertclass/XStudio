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
	/// �����ֽ���
	std::atomic_int send;
	/// �����ֽ���
	std::atomic_int recv;
	/// ������
	std::atomic_int ipkg;

	/// ���ͻ���
	char send_buffer[1024 * 4];
	/// ���ջ���
	char recv_buffer[1024 * 4];

	/// �Ƿ��˳�
	std::atomic_bool exit;

	/// �����������߳�
	std::thread client;

	/// ͬ������
	std::mutex mtx;
	/// ͬ������
	std::condition_variable signal;

	/// ��������д洢
	list< string > response;
};

extern debuger cli;

/// �ȴ��ź���
void wait_signal();

/// ���Կͻ��˿���
void client( int port );

/// ������Ϣ��������
void request( const void* data, int size );

#endif // __LUA_DEBUGER_SOCK_H__