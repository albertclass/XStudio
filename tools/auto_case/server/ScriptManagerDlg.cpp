/////////////////////////////////////////////////////////////////////////////
// Name:        ScriptManagerDlg.cpp
// Purpose:     
// Author:      Albert.xu
// Modified by: 
// Created:     09/03/2016 17:01:52
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

#include "wx/dataview.h"
////@begin includes
#include "wx/imaglist.h"
////@end includes

#include "ScriptManagerDlg.h"
////@begin XPM images
////@end XPM images

/*
 * ScriptManager type definition
 */

IMPLEMENT_DYNAMIC_CLASS( ScriptManagerDlg, wxDialog )


/*
 * ScriptManager event table definition
 */

BEGIN_EVENT_TABLE( ScriptManagerDlg, wxDialog )

////@begin ScriptManager event table entries
	EVT_BUTTON( ID_BUTTON_ADD, ScriptManagerDlg::OnButtonAddClick )
	EVT_BUTTON( ID_BUTTON_DEL, ScriptManagerDlg::OnButtonDelClick )
////@end ScriptManager event table entries

END_EVENT_TABLE()


/*
 * ScriptManager constructors
 */

ScriptManagerDlg::ScriptManagerDlg()
{
    Init();
}

ScriptManagerDlg::ScriptManagerDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

void ScriptManagerDlg::OnButtonAddClick( wxCommandEvent &event )
{
	wxString path = wxDirSelector("Choose a folder to watch", "",
							  wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if( !path.empty() )
	{
		mScriptListModel.AddScriptPath( path );
	}
}

void ScriptManagerDlg::OnButtonDelClick( wxCommandEvent &event )
{
	wxDataViewItemArray sels;
	mScriptListCtrl->GetSelections( sels );
	mScriptListModel.DelScriptPath( sels );
}


/*
 * ScriptManager creator
 */

bool ScriptManagerDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ScriptManager creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end ScriptManager creation
    return true;
}


/*
 * ScriptManager destructor
 */

ScriptManagerDlg::~ScriptManagerDlg()
{
////@begin ScriptManager destruction
////@end ScriptManager destruction
	mScriptListCtrl->AssociateModel( NULL );
}


/*
 * Member initialisation
 */

void ScriptManagerDlg::Init()
{
////@begin ScriptManager member initialisation
    mScriptListCtrl = NULL;
////@end ScriptManager member initialisation
}


/*
 * Control creation for ScriptManager
 */

void ScriptManagerDlg::CreateControls()
{    
////@begin ScriptManager content construction
    ScriptManagerDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = XGC_NEW wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    mScriptListCtrl = XGC_NEW wxDataViewCtrl( itemDialog1, ID_SCRIPT_LIST, wxDefaultPosition, wxDefaultSize );
    itemBoxSizer2->Add(mScriptListCtrl, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = XGC_NEW wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, 0, 0);

    wxButton* itemButton5 = XGC_NEW wxButton( itemDialog1, ID_BUTTON_ADD, _("Add"), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = XGC_NEW wxButton( itemDialog1, ID_BUTTON_DEL, _("Del"), wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end ScriptManager content construction
	mScriptListCtrl->AppendTextColumn( "Script Path", 0, wxDATAVIEW_CELL_EDITABLE, 500 );
	mScriptListCtrl->AssociateModel( &mScriptListModel );
}


/*
 * Should we show tooltips?
 */

bool ScriptManagerDlg::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap ScriptManagerDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin ScriptManager bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end ScriptManager bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon ScriptManagerDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin ScriptManager icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end ScriptManager icon retrieval
}

ScriptDataViewModel::ScriptDataViewModel()
	: wxDataViewIndexListModel( 0 )
{
	auto &loader_path = getConfig().GetLoaderPath();
	auto &module_path = getConfig().GetModulePath();

	if( !fs::exists( loader_path ) )
		return;

	if( !fs::exists( module_path ) )
		return;

	int i = 0;
	for( const auto &entry : fs::directory_iterator { module_path } )
	{
		if( fs::is_directory( entry ) )
		{
			AddScriptPath( entry.path().string() );
		}
	}
}
