///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file MainFrame.h
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 应用程序主窗口
///
///////////////////////////////////////////////////////////////
#pragma once

#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/dataview.h>
#include <wx/sizer.h>
#include <wx/statusbr.h>
#include <wx/frame.h>

#define wxID_MAIN_MENU 1000
#define wxID_STATUS_BAR 1001
#define wxID_MENUITEM_START 1002
#define wxID_MENUITEM_STOP 1003
#define wxID_MENUITEM_SET_SCRIPT 1006
#define wxID_MENUITEM_SET_ACCOUNT 1007
#define wxID_GEN_FILELIST 1009
#define wxID_TIMER 2000

class ClientDataViewModel;

class CMainFrame : public wxFrame
{
	DECLARE_EVENT_TABLE()
public:
	CMainFrame( wxWindow* parent=(wxWindow *)NULL );
	~CMainFrame();

protected:
	wxMenuBar* mMainMenu;
	wxMenu* mFileMenu;
	wxDataViewCtrl* mClientDataViewCtrl;
	wxDataViewColumn* mColAddress;
	wxDataViewColumn* mColPID;
	wxDataViewColumn* mColTID;
	wxDataViewColumn* mColCPU;
	wxDataViewColumn* mColMEM;
	wxDataViewColumn* mColStatus;
	wxDataViewColumn* mColUser;
	wxDataViewColumn* mColScript;
	wxDataViewColumn* mColProgress;
	wxStatusBar* mStatusBar;
	wxMenu* mContextMenuNode;
	wxMenu* mContextMenuData;

private:
	void OnItemContextMenu( wxDataViewEvent& event );
	void OnGenFileList( wxCommandEvent& event );
	void OnMenuItem_Start( wxCommandEvent& event );
	void OnMenuItem_Stop( wxCommandEvent& event );
	void OnMenuItem_SetScript( wxCommandEvent& event );
	void OnMenuItem_AddAccount( wxCommandEvent& event );
	void OnTimer( wxTimerEvent & event );
private:
	/// 定时器
	wxTimer mTimer;
};

void OnFileChanged( xgc_lpcstr path, xgc_lpcstr filename, xgc_ulong action );

#endif // _MAINFRAME_H_