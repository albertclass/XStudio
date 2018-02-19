///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file theApp.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// wxWidget Ӧ�ó�����
///
///////////////////////////////////////////////////////////////
#pragma once
#include "wx/app.h"             // wxApp

class theApp :	public wxApp
{
public:
	theApp();
	~theApp();

	// override base class (pure) virtuals
	virtual bool Initialize(int& argc, wxChar **argv);

	// Override base class virtuals:
	// wxApp::OnInit() is called on application startup and is a good place
	// for the app initialization (doing it here and not in the ctor
	// allows to have an error return: if OnInit() returns false, the
	// application terminates)
	virtual bool OnInit();

	// This is only called if OnInit() returned true so it's a good place to do
	// any cleanup matching the initializations done there.
	virtual int OnExit();

	// Clean All
	virtual void CleanUp();

	virtual bool ProcessIdle();

private:
	/// ���������
	xgc_uintptr mServerHandle;
	/// ��Ϣ����
	MessageQueuePtr mMessageQueue;
};

