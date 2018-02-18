/////////////////////////////////////////////////////////////////////////////
// Name:        scriptselectordlg.h
// Purpose:     
// Author:      Albert.xu
// Modified by: 
// Created:     2016/3/10 11:23:49
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCRIPTSELECTORDLG_H_
#define _SCRIPTSELECTORDLG_H_


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
class wxListCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SCRIPTSELECTORDLG 10000
#define ID_LISTCTRL 10001
#define ID_BUTTON_OK 10002
#define ID_BUTTON_CANCEL 10003
#define SYMBOL_SCRIPTSELECTORDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_SCRIPTSELECTORDLG_TITLE _("Script Selector")
#define SYMBOL_SCRIPTSELECTORDLG_IDNAME ID_SCRIPTSELECTORDLG
#define SYMBOL_SCRIPTSELECTORDLG_SIZE wxSize(400, 300)
#define SYMBOL_SCRIPTSELECTORDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * ScriptSelectorDlg class declaration
 */

class ScriptSelectorDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( ScriptSelectorDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ScriptSelectorDlg();
    ScriptSelectorDlg( wxWindow* parent, wxWindowID id = SYMBOL_SCRIPTSELECTORDLG_IDNAME, const wxString& caption = SYMBOL_SCRIPTSELECTORDLG_TITLE, const wxPoint& pos = SYMBOL_SCRIPTSELECTORDLG_POSITION, const wxSize& size = SYMBOL_SCRIPTSELECTORDLG_SIZE, long style = SYMBOL_SCRIPTSELECTORDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_SCRIPTSELECTORDLG_IDNAME, const wxString& caption = SYMBOL_SCRIPTSELECTORDLG_TITLE, const wxPoint& pos = SYMBOL_SCRIPTSELECTORDLG_POSITION, const wxSize& size = SYMBOL_SCRIPTSELECTORDLG_SIZE, long style = SYMBOL_SCRIPTSELECTORDLG_STYLE );

    /// Destructor
    ~ScriptSelectorDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin ScriptSelectorDlg event handler declarations

    /// wxEVT_LEFT_DCLICK event handler for ID_LISTCTRL
    void OnLeftDClick( wxMouseEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_OK
    void OnButtonOkClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CANCEL
    void OnButtonCancelClick( wxCommandEvent& event );

////@end ScriptSelectorDlg event handler declarations

////@begin ScriptSelectorDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ScriptSelectorDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin ScriptSelectorDlg member variables
    wxListCtrl* mListCtrl;
////@end ScriptSelectorDlg member variables

	wxString mScriptPath;

	wxString GetSelectScript()const
	{
		return mScriptPath;
	}
};

#endif
    // _SCRIPTSELECTORDLG_H_
