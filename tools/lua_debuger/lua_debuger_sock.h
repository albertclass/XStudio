#ifndef __LUA_DEBUGER_SOCK_H__
#define __LUA_DEBUGER_SOCK_H__
#include "defines.h"
#include "xnet.h"
#include "xutility.h"

#include <thread>
#include <mutex>
#include <atomic>

using namespace xgc;
using namespace common;

// �ȴ��ź���
void wait_signal();

// ����������
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
};

extern debuger cli;

/// ���Կͻ��˿���
void client( int port );

/// ��������
void send( const void* data, int size );

#endif // __LUA_DEBUGER_SOCK_H__