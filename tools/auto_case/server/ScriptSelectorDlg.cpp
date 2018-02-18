/////////////////////////////////////////////////////////////////////////////
// Name:        scriptselectordlg.cpp
// Purpose:     
// Author:      Albert.xu
// Modified by: 
// Created:     2016/3/10 11:23:49
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include "scriptselectordlg.h"
#include "PythonScript.h"

////@begin XPM images
////@end XPM images


/*
 * ScriptSelectorDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( ScriptSelectorDlg, wxDialog )


/*
 * ScriptSelectorDlg event table definition
 */

BEGIN_EVENT_TABLE( ScriptSelectorDlg, wxDialog )

////@begin ScriptSelectorDlg event table entries
    EVT_BUTTON( ID_BUTTON_OK, ScriptSelectorDlg::OnButtonOkClick )
    EVT_BUTTON( ID_BUTTON_CANCEL, ScriptSelectorDlg::OnButtonCancelClick )
////@end ScriptSelectorDlg event table entries

END_EVENT_TABLE()


/*
 * ScriptSelectorDlg constructors
 */

ScriptSelectorDlg::ScriptSelectorDlg()
{
    Init();
}

ScriptSelectorDlg::ScriptSelectorDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * ScriptSelectorDlg creator
 */

bool ScriptSelectorDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ScriptSelectorDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end ScriptSelectorDlg creation
    return true;
}


/*
 * ScriptSelectorDlg destructor
 */

ScriptSelectorDlg::~ScriptSelectorDlg()
{
////@begin ScriptSelectorDlg destruction
////@end ScriptSelectorDlg destruction
}


/*
 * Member initialisation
 */

void ScriptSelectorDlg::Init()
{
////@begin ScriptSelectorDlg member initialisation
    mListCtrl = NULL;
////@end ScriptSelectorDlg member initialisation
}


/*
 * Control creation for ScriptSelectorDlg
 */

void ScriptSelectorDlg::CreateControls()
{    
////@begin ScriptSelectorDlg content construction
    ScriptSelectorDlg* dlg = this;

    wxBoxSizer* mainSizer = XGC_NEW wxBoxSizer(wxVERTICAL);
    dlg->SetSizer(mainSizer);

    mListCtrl = XGC_NEW wxListCtrl( dlg, ID_LISTCTRL, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
    mainSizer->Add(mListCtrl, 1, wxGROW|wxALL, 5);

    wxBoxSizer* btnSizer = XGC_NEW wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* btnConfirm = XGC_NEW wxButton( dlg, ID_BUTTON_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    btnSizer->Add(btnConfirm, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* btnCancel = XGC_NEW wxButton( dlg, ID_BUTTON_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    btnSizer->Add(btnCancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Connect events and objects
    mListCtrl->Connect(ID_LISTCTRL, wxEVT_LEFT_DCLICK, wxMouseEventHandler(ScriptSelectorDlg::OnLeftDClick), NULL, this);
////@end ScriptSelectorDlg content construction
	mListCtrl->AppendColumn( "Name", wxLIST_FORMAT_LEFT, 80 );
	mListCtrl->AppendColumn( "Path", wxLIST_FORMAT_LEFT, 400 );
	mListCtrl->AppendColumn( "Version", wxLIST_FORMAT_LEFT, 100 );

	auto &loader_path = getConfig().GetLoaderPath();
	auto &module_path = getConfig().GetModulePath();

	if( !fs::exists( loader_path ) )
		return;

	if( !fs::exists( module_path ) )
		return;

	int i = 0;
	xgc_string version;

	for( const auto &entry : fs::directory_iterator { module_path } )
	{
		if( fs::is_directory( entry ) )
		{
			if( entry.path() == fs::path(loader_path) )
				continue;

			if( !fs::exists( entry.path() / "version.py" ) )
				continue;

			if( !getScript().GetModuleVersion( entry.path().filename().string(), version ) )
				continue;

			int item = mListCtrl->InsertItem( i++, entry.path().filename().string(), -1 );
			if( item != -1 )
			{
				mListCtrl->SetItem( item, 1, entry.path().string() );
				mListCtrl->SetItem( item, 2, version );
			}
		}
	}
}


/*
 * Should we show tooltips?
 */

bool ScriptSelectorDlg::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap ScriptSelectorDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin ScriptSelectorDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end ScriptSelectorDlg bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon ScriptSelectorDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin ScriptSelectorDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end ScriptSelectorDlg icon retrieval
}

/*
 * wxEVT_LEFT_DCLICK event handler for ID_LISTCTRL
 */

void ScriptSelectorDlg::OnLeftDClick( wxMouseEvent& event )
{
	auto item = mListCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	if( item != -1 )
	{
		mScriptPath = mListCtrl->GetItemText( item, 0 );

		EndDialog( wxID_OK );
	}
	else
	{
		EndDialog( wxID_CANCEL );
	}
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_OK
 */

void ScriptSelectorDlg::OnButtonOkClick( wxCommandEvent& event )
{
	auto item = mListCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	if( item != -1 )
	{
		mScriptPath = mListCtrl->GetItemText( item, 1 );

		EndDialog( wxID_OK );
	}
	else
	{
		EndDialog( wxID_CANCEL );
	}
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CANCEL
 */

void ScriptSelectorDlg::OnButtonCancelClick( wxCommandEvent& event )
{
	EndDialog( wxID_CANCEL );
}

