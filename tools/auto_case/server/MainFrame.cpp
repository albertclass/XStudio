#include "StdAfx.h"

#include "ClientDataViewModel.h"
#include "DataViewFileSelecterRenderer.h"
#include "MainFrame.h"
#include "MessageHandler.h"
#include "ScriptSelectorDlg.h"
#include "UserSelectorDlg.h"
using namespace xgc::message;

BEGIN_EVENT_TABLE( CMainFrame, wxFrame )
	EVT_DATAVIEW_ITEM_CONTEXT_MENU( wxID_ANY, CMainFrame::OnItemContextMenu )
	EVT_MENU( wxID_GEN_FILELIST, CMainFrame::OnGenFileList )
	EVT_MENU( wxID_MENUITEM_START, CMainFrame::OnMenuItem_Start )
	EVT_MENU( wxID_MENUITEM_STOP, CMainFrame::OnMenuItem_Stop )
	EVT_MENU( wxID_MENUITEM_SET_SCRIPT, CMainFrame::OnMenuItem_SetScript )
	EVT_MENU( wxID_MENUITEM_SET_ACCOUNT, CMainFrame::OnMenuItem_AddAccount )
	EVT_TIMER( wxID_TIMER, CMainFrame::OnTimer )
END_EVENT_TABLE()

#define MainFrameStyle (wxCAPTION|wxCLOSE_BOX|wxDEFAULT_FRAME_STYLE|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxSYSTEM_MENU|wxTAB_TRAVERSAL)

CMainFrame::CMainFrame( wxWindow* parent )
	: wxFrame( parent, wxID_ANY, wxT("AutoCase"), wxDefaultPosition, wxSize( 800,600 ), MainFrameStyle )
	, mTimer( this )
{
	// Set the icon for the frame.
	SetIcon(wxICON(AutoCase));

	SetSizeHints( wxSize( 200,150 ), wxDefaultSize );

	// MenuBar
	mMainMenu = XGC_NEW wxMenuBar( 0 );
	mFileMenu = XGC_NEW wxMenu();
	wxMenuItem* mFileExit;
	mFileExit = XGC_NEW wxMenuItem( mFileMenu, wxID_EXIT, wxString( wxT("E&xit") ) + wxT('\t') + wxT("Ctrl-Q"), wxEmptyString, wxITEM_NORMAL );
	mFileMenu->Append( wxID_GEN_FILELIST, "&Gen filelist", nullptr );
	mFileMenu->Append( mFileExit );
	mMainMenu->Append( mFileMenu, wxT("&File") ); 
	this->SetMenuBar( mMainMenu );

	// Client Area
	wxBoxSizer* mainSizer;
	mainSizer = XGC_NEW wxBoxSizer( wxVERTICAL );

	mClientDataViewCtrl = XGC_NEW wxDataViewCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_MULTIPLE|wxDV_ROW_LINES );

	mColAddress  = mClientDataViewCtrl->AppendTextColumn( wxT("Address"), 0, wxDATAVIEW_CELL_INERT, 120, wxALIGN_CENTRE );
	mColPID      = mClientDataViewCtrl->AppendTextColumn( wxT("PID"), 1, wxDATAVIEW_CELL_INERT, 50, wxALIGN_CENTRE );
	mColTID      = mClientDataViewCtrl->AppendTextColumn( wxT("TID"), 2, wxDATAVIEW_CELL_INERT, 50, wxALIGN_CENTRE );
	mColCPU      = mClientDataViewCtrl->AppendTextColumn( wxT("CPU %"), 3, wxDATAVIEW_CELL_INERT, 60, wxALIGN_CENTRE );
	mColMEM      = mClientDataViewCtrl->AppendTextColumn( wxT("MEM %"), 4, wxDATAVIEW_CELL_INERT, 60, wxALIGN_CENTRE );
	mColStatus   = mClientDataViewCtrl->AppendTextColumn( wxT("Status"), 5, wxDATAVIEW_CELL_INERT, 180, wxALIGN_CENTRE );
	mColProgress = mClientDataViewCtrl->AppendProgressColumn( wxT("Progress"), 6, wxDATAVIEW_CELL_INERT, 120 );
	mColUser     = mClientDataViewCtrl->AppendTextColumn( wxT("Account"), 7, wxDATAVIEW_CELL_INERT, 80, wxALIGN_CENTRE );
	mColScript   = mClientDataViewCtrl->AppendTextColumn( wxT("Script"), 8, wxDATAVIEW_CELL_INERT, 260 );
	
	mClientDataViewCtrl->AssociateModel( &gDataViewModel );

	mainSizer->Add( mClientDataViewCtrl, 1, wxEXPAND, 5 );

	this->SetSizer( mainSizer );
	this->Layout();
	mStatusBar = this->CreateStatusBar( 3, wxST_SIZEGRIP, wxID_STATUS_BAR );

	//////////////////////////////////////////////////////////////////////////
	// data menu
	mContextMenuData = XGC_NEW wxMenu();
	// menu items
	wxMenuItem* menuItem_Start;
	menuItem_Start = XGC_NEW wxMenuItem( mContextMenuData, wxID_MENUITEM_START, wxString( wxT("Start") ) , wxEmptyString, wxITEM_NORMAL );
	mContextMenuData->Append( menuItem_Start );

	wxMenuItem* menuItem_Stop;
	menuItem_Stop = XGC_NEW wxMenuItem( mContextMenuData, wxID_MENUITEM_STOP, wxString( wxT("Stop") ) , wxEmptyString, wxITEM_NORMAL );
	mContextMenuData->Append( menuItem_Stop );

	wxMenuItem* menuItem_SetScript;
	menuItem_SetScript = XGC_NEW wxMenuItem( mContextMenuData, wxID_MENUITEM_SET_SCRIPT, wxString( wxT("Set Script") ) , wxEmptyString, wxITEM_NORMAL );
	mContextMenuData->Append( menuItem_SetScript );
	// data menu
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// node menu
	mContextMenuNode = XGC_NEW wxMenu();
	// menu items
	// menu items
	wxMenuItem* menuItem_StartAll;
	menuItem_StartAll = XGC_NEW wxMenuItem( mContextMenuNode, wxID_MENUITEM_START, wxString( wxT("Start") ) , wxEmptyString, wxITEM_NORMAL );
	mContextMenuNode->Append( menuItem_StartAll );

	wxMenuItem* menuItem_StopAll;
	menuItem_StopAll = XGC_NEW wxMenuItem( mContextMenuNode, wxID_MENUITEM_STOP, wxString( wxT("Stop") ) , wxEmptyString, wxITEM_NORMAL );
	mContextMenuNode->Append( menuItem_StopAll );

	wxMenuItem* menuItem_SetUser;
	menuItem_SetUser = XGC_NEW wxMenuItem( mContextMenuNode, wxID_MENUITEM_SET_ACCOUNT, wxString( wxT("Set Accounts") ) , wxEmptyString, wxITEM_NORMAL );
	mContextMenuNode->Append( menuItem_SetUser );
	// node menu
	//////////////////////////////////////////////////////////////////////////
	this->Centre( wxBOTH );
	mTimer.Start( 100 );

	xgc_string watch_path = getConfig().GetLoaderPath().string();
	add_filewatcher( watch_path.c_str(), FILE_WATCHER_NOTIFY_LASTWRITE, true, std::bind( OnFileChanged, _1, _2, _3 ) );
}

void OnFileChanged( xgc_lpcstr path, xgc_lpcstr filename, xgc_ulong action )
{
	switch( action )
	{
		case FILE_WATCHER_ACTION_ADDED:
		break;
		case FILE_WATCHER_ACTION_REMOVED:
		break;
		case FILE_WATCHER_ACTION_MODIFIED:
		break;
		case FILE_WATCHER_ACTION_RENAMED_OLD_NAME:
		break;
		case FILE_WATCHER_ACTION_RENAMED_NEW_NAME:
		break;
	}
}

CMainFrame::~CMainFrame()
{
	SAFE_DELETE( mContextMenuNode ); 
	SAFE_DELETE( mContextMenuData );

	mClientDataViewCtrl->AssociateModel( xgc_nullptr );
}


void CMainFrame::OnItemContextMenu( wxDataViewEvent& event )
{
	wxPoint pt = event.GetPosition();
	ClientDataType* pType = (ClientDataType*) event.GetItem().GetID();
	if( nullptr == pType )
		return;

	switch( pType->type )
	{
		case 0:
		mClientDataViewCtrl->PopupMenu( mContextMenuNode, pt );
		break;
		case 1:
		mClientDataViewCtrl->PopupMenu( mContextMenuData, pt );
		break;
	}
}

void CMainFrame::OnGenFileList( wxCommandEvent & event )
{
	getConfig().GenerateFileist();
	gDataViewModel.UpdatePush();
}

inline void CMainFrame::OnMenuItem_Start( wxCommandEvent & event )
{
	wxDataViewItemArray sels;
	if( mClientDataViewCtrl->GetSelections( sels ) )
	{
		for each(auto &item in sels)
		{
			gDataViewModel.Start( item );
		}
	}
}

inline void CMainFrame::OnMenuItem_Stop( wxCommandEvent & event )
{
	wxDataViewItemArray sels;
	if( mClientDataViewCtrl->GetSelections( sels ) )
	{
		for each(auto &item in sels)
		{
			gDataViewModel.Stop( item );
		}
	}
}

inline void CMainFrame::OnMenuItem_SetScript( wxCommandEvent & event )
{
	ScriptSelectorDlg dlg(this);
	if( wxID_OK == dlg.ShowModal() )
	{
		wxDataViewItemArray sel;
		mClientDataViewCtrl->GetSelections( sel );

		gDataViewModel.SetClientDataScript( sel, dlg.GetSelectScript() );
		mClientDataViewCtrl->Refresh( false );
	}
}

inline void CMainFrame::OnMenuItem_AddAccount( wxCommandEvent& event )
{
	UserSelectorDlg dlg( this );
	if( wxID_OK == dlg.ShowModal() )
	{
		std::list<AppConfig::account*> accounts;
		dlg.GetSelectAccounts( accounts );

		wxDataViewItem item = mClientDataViewCtrl->GetSelection();
		gDataViewModel.SetClientNodeAccounts( item, accounts );
		mClientDataViewCtrl->Expand( item );
	}
}

void CMainFrame::OnTimer( wxTimerEvent & event )
{
}

