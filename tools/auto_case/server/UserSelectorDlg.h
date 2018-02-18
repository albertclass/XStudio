/////////////////////////////////////////////////////////////////////////////
// Name:        UserSelectorDlg.h
// Purpose:     
// Author:      Albert.xu
// Modified by: 
// Created:     2016/3/10 20:28:41
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _USERSELECTORDLG_H_
#define _USERSELECTORDLG_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/listctrl.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class wxListCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_USERSELECTORDLG 10000
#define ID_LISTCTRL 10001
#define ID_BUTTON_OK 10002
#define ID_BUTTON_CANCEL 10003
#define SYMBOL_USERSELECTORDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_USERSELECTORDLG_TITLE _("User Selector")
#define SYMBOL_USERSELECTORDLG_IDNAME ID_USERSELECTORDLG
#define SYMBOL_USERSELECTORDLG_SIZE wxSize(400, 480)
#define SYMBOL_USERSELECTORDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * UserSelectorDlg class declaration
 */

class UserSelectorDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( UserSelectorDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    UserSelectorDlg();
    UserSelectorDlg( wxWindow* parent, wxWindowID id = SYMBOL_USERSELECTORDLG_IDNAME, const wxString& caption = SYMBOL_USERSELECTORDLG_TITLE, const wxPoint& pos = SYMBOL_USERSELECTORDLG_POSITION, const wxSize& size = SYMBOL_USERSELECTORDLG_SIZE, long style = SYMBOL_USERSELECTORDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_USERSELECTORDLG_IDNAME, const wxString& caption = SYMBOL_USERSELECTORDLG_TITLE, const wxPoint& pos = SYMBOL_USERSELECTORDLG_POSITION, const wxSize& size = SYMBOL_USERSELECTORDLG_SIZE, long style = SYMBOL_USERSELECTORDLG_STYLE );

    /// Destructor
    ~UserSelectorDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

	///
	/// \brief 获取选择的用户
	///
	/// \author albert.xu
	/// \date 2016/03/11 10:43
	///
	void GetSelectAccounts( std::list< AppConfig::account* > &accounts );
////@begin UserSelectorDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_OK
    void OnButtonOkClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CANCEL
    void OnButtonCancelClick( wxCommandEvent& event );

////@end UserSelectorDlg event handler declarations

////@begin UserSelectorDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end UserSelectorDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin UserSelectorDlg member variables
    wxBoxSizer* mMainSizer;
    wxListCtrl* mListCtrl;
    wxBoxSizer* mBtnSizer;
////@end UserSelectorDlg member variables
};

#endif
    // _USERSELECTORDLG_H_
