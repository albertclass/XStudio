///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file case_manager.h
/// \brief 文件简介
/// \author xufeng04
/// \date 三月 2016
///
/// 测试用例管理器
///
///////////////////////////////////////////////////////////////
#pragma once

#ifndef _CASE_MANAGER_H_
#define _CASE_MANAGER_H_

///
/// \brief 测试进程
///
/// \author albert.xu
/// \date 2016/03/14 14:48
///
class case_executer
{
public:
	///
	/// \brief 进程状态
	///
	/// \author albert.xu
	/// \date 2016/04/11 11:01
	///
	enum { alive, terminate, exit };

	///
	/// \brief 构造
	///
	/// \author albert.xu
	/// \date 2016/03/14 16:19
	///
	case_executer( const xgc_string &username );

	///
	/// \brief 析构
	///
	/// \author albert.xu
	/// \date 2016/03/14 16:19
	///
	~case_executer();

	///
	/// \brief 启动进程
	///
	/// \author albert.xu
	/// \date 2016/04/07 15:38
	///
	xgc_bool start( xgc_size pipe_size, xgc_lpcstr path, xgc_lpcstr cmdline );

	///
	/// \brief 接收输出的内容
	///
	/// \author albert.xu
	/// \date 2016/03/14 17:14
	///
	xgc_size recv();

	///
	/// \brief 发送指令到进程
	///
	/// \author albert.xu
	/// \date 2016/04/08 13:26
	///
	xgc_size send( xgc_lpcstr buffer, xgc_size buffer_size );

	///
	/// \brief 处理接收到缓冲内的内容
	///
	/// \author albert.xu
	/// \date 2016/04/08 13:29
	///
	xgc_void update();

	///
	/// \brief 获取进程状态
	///
	/// \author albert.xu
	/// \date 2016/04/11 10:35
	///
	xgc_long status();

	///
	/// \brief 获取退出代码
	///
	/// \author albert.xu
	/// \date 2016/04/11 10:54
	///
	xgc_bool exitcode( xgc_long* code );

	///
	/// \brief 获取进程id
	///
	/// \author albert.xu
	/// \date 2016/04/18 15:10
	///
	xgc_ulong get_pid()const;

	///
	/// \brief 获取线程id
	///
	/// \author albert.xu
	/// \date 2016/04/18 15:10
	///
	xgc_ulong get_tid()const;
private:
	/// 进程句柄
	HANDLE process_h;
	/// 重定向输出
	int fd_w[2] = { -1, -1 };
	/// 重定向输入
	int fd_r[2] = { -1, -1 };
	/// 用户名
	xgc_string name;
	/// 进程号
	xgc_ulong pid;
	/// 线程号
	xgc_ulong tid;
	/// 命令行缓冲
	xgc_char buffer[1024 * 4];
	/// 缓冲偏移
	xgc_size buffer_bytes;
	/// 定时器句柄
	timer_t t;
};

///
/// \brief 测试用例管理器
///
/// \author albert.xu
/// \date 2016/03/14 14:48
///
class manager
{
	friend class case_executer;
public:
	manager();
	~manager();

	///
	/// \brief 启动测试进程
	///
	/// \author albert.xu
	/// \date 2016/03/14 14:50
	///
	xgc_long start( const std::string &username,
					const std::string &password,
					const std::string &module,
					const std::string &loader_version,
					const std::string &module_version,
					const std::string &address );

	///
	/// \brief 终止进程
	///
	/// \author albert.xu
	/// \date 2016/03/14 15:48
	///
	void stop( const std::string &username );

	///
	/// \brief 请求更新脚本文件
	///
	/// \author albert.xu
	/// \date 2016/03/14 18:20
	///
	void request_loader();

	///
	/// \brief 请求更新脚本文件
	///
	/// \author albert.xu
	/// \date 2016/03/14 18:20
	///
	void request_module( const std::string &module_name );

protected:
	///
	/// \brief 添加测试进程到管理器
	///
	/// \author albert.xu
	/// \date 2016/03/14 16:14
	///
	xgc_bool add( const xgc_string &username );

	///
	/// \brief 删除测试进程从管理器
	///
	/// \author albert.xu
	/// \date 2016/03/14 16:15
	///
	xgc_bool del( const xgc_string &username );

	///
	/// \brief 根据账号获取进程指针
	///
	/// \author albert.xu
	/// \date 2016/04/11 11:10
	///
	case_executer* get( const xgc_string &username );

private:
	/// 测试进程映射表
	std::unordered_map< std::string, case_executer* > case_map;
	/// 会话ID
	network_t session;
};

///
/// \brief 获取测试管理器
///
/// \author albert.xu
/// \date 2016/03/14 18:51
///
manager &get_case_manager();

///
/// \brief 获取定时器对象
///
/// \author albert.xu
/// \date 2016/03/14 18:51
///
timer &get_timer();
#endif // _CASE_MANAGER_H_