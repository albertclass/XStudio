///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file case_manager.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// ��������������
///
///////////////////////////////////////////////////////////////
#pragma once

#ifndef _CASE_MANAGER_H_
#define _CASE_MANAGER_H_

///
/// \brief ���Խ���
///
/// \author albert.xu
/// \date 2016/03/14 14:48
///
class case_executer
{
public:
	///
	/// \brief ����״̬
	///
	/// \author albert.xu
	/// \date 2016/04/11 11:01
	///
	enum { alive, terminate, exit };

	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2016/03/14 16:19
	///
	case_executer( const xgc_string &username );

	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2016/03/14 16:19
	///
	~case_executer();

	///
	/// \brief ��������
	///
	/// \author albert.xu
	/// \date 2016/04/07 15:38
	///
	xgc_bool start( xgc_size pipe_size, xgc_lpcstr path, xgc_lpcstr cmdline );

	///
	/// \brief �������������
	///
	/// \author albert.xu
	/// \date 2016/03/14 17:14
	///
	xgc_size recv();

	///
	/// \brief ����ָ�����
	///
	/// \author albert.xu
	/// \date 2016/04/08 13:26
	///
	xgc_size send( xgc_lpcstr buffer, xgc_size buffer_size );

	///
	/// \brief ������յ������ڵ�����
	///
	/// \author albert.xu
	/// \date 2016/04/08 13:29
	///
	xgc_void update();

	///
	/// \brief ��ȡ����״̬
	///
	/// \author albert.xu
	/// \date 2016/04/11 10:35
	///
	xgc_long status();

	///
	/// \brief ��ȡ�˳�����
	///
	/// \author albert.xu
	/// \date 2016/04/11 10:54
	///
	xgc_bool exitcode( xgc_long* code );

	///
	/// \brief ��ȡ����id
	///
	/// \author albert.xu
	/// \date 2016/04/18 15:10
	///
	xgc_ulong get_pid()const;

	///
	/// \brief ��ȡ�߳�id
	///
	/// \author albert.xu
	/// \date 2016/04/18 15:10
	///
	xgc_ulong get_tid()const;
private:
	/// ���̾��
	HANDLE process_h;
	/// �ض������
	int fd_w[2] = { -1, -1 };
	/// �ض�������
	int fd_r[2] = { -1, -1 };
	/// �û���
	xgc_string name;
	/// ���̺�
	xgc_ulong pid;
	/// �̺߳�
	xgc_ulong tid;
	/// �����л���
	xgc_char buffer[1024 * 4];
	/// ����ƫ��
	xgc_size buffer_bytes;
	/// ��ʱ�����
	timer_t t;
};

///
/// \brief ��������������
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
	/// \brief �������Խ���
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
	/// \brief ��ֹ����
	///
	/// \author albert.xu
	/// \date 2016/03/14 15:48
	///
	void stop( const std::string &username );

	///
	/// \brief ������½ű��ļ�
	///
	/// \author albert.xu
	/// \date 2016/03/14 18:20
	///
	void request_loader();

	///
	/// \brief ������½ű��ļ�
	///
	/// \author albert.xu
	/// \date 2016/03/14 18:20
	///
	void request_module( const std::string &module_name );

protected:
	///
	/// \brief ��Ӳ��Խ��̵�������
	///
	/// \author albert.xu
	/// \date 2016/03/14 16:14
	///
	xgc_bool add( const xgc_string &username );

	///
	/// \brief ɾ�����Խ��̴ӹ�����
	///
	/// \author albert.xu
	/// \date 2016/03/14 16:15
	///
	xgc_bool del( const xgc_string &username );

	///
	/// \brief �����˺Ż�ȡ����ָ��
	///
	/// \author albert.xu
	/// \date 2016/04/11 11:10
	///
	case_executer* get( const xgc_string &username );

private:
	/// ���Խ���ӳ���
	std::unordered_map< std::string, case_executer* > case_map;
	/// �ỰID
	network_t session;
};

///
/// \brief ��ȡ���Թ�����
///
/// \author albert.xu
/// \date 2016/03/14 18:51
///
manager &get_case_manager();

///
/// \brief ��ȡ��ʱ������
///
/// \author albert.xu
/// \date 2016/03/14 18:51
///
timer &get_timer();
#endif // _CASE_MANAGER_H_