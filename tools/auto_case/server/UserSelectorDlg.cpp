/////////////////////////////////////////////////////////////////////////////
// Name:        UserSelectorDlg.cpp
// Purpose:     
// Author:      Albert.xu
// Modified by: 
// Created:     2016/3/10 20:28:41
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

#include "UserSelectorDlg.h"

////@begin XPM images
////@end XPM images


/*
 * UserSelectorDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( UserSelectorDlg, wxDialog )


/*
 * UserSelectorDlg event table definition
 */

BEGIN_EVENT_TABLE( UserSelectorDlg, wxDialog )

////@begin UserSelectorDlg event table entries
    EVT_BUTTON( ID_BUTTON_OK, UserSelectorDlg::OnButtonOkClick )
    EVT_BUTTON( ID_BUTTON_CANCEL, UserSelectorDlg::OnButtonCancelClick )
////@end UserSelectorDlg event table entries

END_EVENT_TABLE()


/*
 * UserSelectorDlg constructors
 */

UserSelectorDlg::UserSelectorDlg()
{
    Init();
}

UserSelectorDlg::UserSelectorDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * UserSelectorDlg creator
 */

bool UserSelectorDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin UserSelectorDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end UserSelectorDlg creation
    return true;
}


/*
 * UserSelectorDlg destructor
 */

UserSelectorDlg::~UserSelectorDlg()
{
////@begin UserSelectorDlg destruction
////@end UserSelectorDlg destruction
}


/*
 * Member initialisation
 */

void UserSelectorDlg::Init()
{
////@begin UserSelectorDlg member initialisation
    mMainSizer = NULL;
    mListCtrl = NULL;
    mBtnSizer = NULL;
////@end UserSelectorDlg member initialisation
}


/*
 * Control creation for UserSelectorDlg
 */

void UserSelectorDlg::CreateControls()
{    
////@begin UserSelectorDlg content construction
    UserSelectorDlg* itemDialog1 = this;

    mMainSizer = XGC_NEW wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(mMainSizer);

    mListCtrl = XGC_NEW wxListCtrl( itemDialog1, ID_LISTCTRL, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
    mMainSizer->Add(mListCtrl, 1, wxGROW|wxALL, 5);

    mBtnSizer = XGC_NEW wxBoxSizer(wxHORIZONTAL);
    mMainSizer->Add(mBtnSizer, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton5 = XGC_NEW wxButton( itemDialog1, ID_BUTTON_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    mBtnSizer->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = XGC_NEW wxButton( itemDialog1, ID_BUTTON_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    mBtnSizer->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end UserSelectorDlg content construction
	mListCtrl->AppendColumn( "User", wxLIST_FORMAT_LEFT, 100 );
	mListCtrl->AppendColumn( "Password", wxLIST_FORMAT_LEFT, 100 );
	mListCtrl->AppendColumn( "Bind Script", wxLIST_FORMAT_LEFT, 180 );

	for( auto i = 0; i < getConfig().GetAccountCount(); ++i )
	{
		const AppConfig::account* pAccount = getConfig().GetAccount( i );
		if( pAccount->used )
			continue;

		int item = mListCtrl->InsertItem( i, pAccount->username, -1 );
		if( item != -1 )
		{
			mListCtrl->SetItem( item, 1, pAccount->password );
			mListCtrl->SetItem( item, 2, pAccount->script );

			mListCtrl->SetItemData( item, i );
		}
	}

}


/*
 * Should we show tooltips?
 */

bool UserSelectorDlg::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap UserSelectorDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin UserSelectorDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end UserSelectorDlg bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon UserSelectorDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin UserSelectorDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end UserSelectorDlg icon retrieval
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_OK
 */

void UserSelectorDlg::GetSelectAccounts( std::list<AppConfig::account*> &accounts )
{
	auto item = mListCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	while( item != -1 )
	{
		size_t nAccountID = (size_t)mListCtrl->GetItemData( item );
		auto pAccount = getConfig().GetAccount( nAccountID );
		if( pAccount )
			accounts.push_back( pAccount );

		item = mListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	}
}

void UserSelectorDlg::OnButtonOkClick( wxCommandEvent& event )
{
	EndDialog( wxID_OK );
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CANCEL
 */

void UserSelectorDlg::OnButtonCancelClick( wxCommandEvent& event )
{
	EndDialog( wxID_CANCEL );
}

