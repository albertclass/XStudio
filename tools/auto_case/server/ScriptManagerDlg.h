/////////////////////////////////////////////////////////////////////////////
// Name:        ScriptManagerDlg.h
// Purpose:     
// Author:      Albert.xu
// Modified by: 
// Created:     09/03/2016 17:01:52
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCRIPTMANAGERDLG_H_
#define _SCRIPTMANAGERDLG_H_


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
class wxDataViewListCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SCRIPTMANAGER 10000
#define ID_SCRIPT_LIST 10002
#define ID_BUTTON_ADD 10006
#define ID_BUTTON_DEL 10007
#define SYMBOL_SCRIPTMANAGER_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_SCRIPTMANAGER_TITLE _("Script Manager")
#define SYMBOL_SCRIPTMANAGER_IDNAME ID_SCRIPTMANAGER
#define SYMBOL_SCRIPTMANAGER_SIZE wxSize(600, 400)
#define SYMBOL_SCRIPTMANAGER_POSITION wxDefaultPosition
////@end control identifiers

///
/// \brief 代理存储数据
///
/// \author albert.xu
/// \date 2016/02/23 11:48
///
class ScriptDataViewModel : public wxDataViewIndexListModel
{
private:
	/// 数据存储
	wxArrayString mScriptPaths;

public:
	ScriptDataViewModel();

	~ScriptDataViewModel()
	{

	}

	void AddScriptPath( const wxString &strPath )
	{
		mScriptPaths.push_back( strPath );
		RowAppended();
	}

	void DelScriptPath( const wxString &strPath )
	{
		for( auto i = 0; i < mScriptPaths.size(); ++i )
			if( strPath == mScriptPaths[i] )
				RowDeleted( i );
	}

	void DelScriptPath( const wxDataViewItem& item )
	{
		unsigned int row = GetRow( item );

		if (row >= mScriptPaths.GetCount())
			return;

		mScriptPaths.RemoveAt( row );
		RowDeleted( row );
	}


	static int SortR( int *v1, int *v2 )
	{
		return *v2-*v1;
	}

	static int Sort( int *v1, int *v2 )
	{
		return *v1-*v2;
	}

	void DelScriptPath( const wxDataViewItemArray& items )
	{
		unsigned i;
		wxArrayInt rows;
		for (i = 0; i < items.GetCount(); i++)
		{
			unsigned int row = GetRow( items[i] );
			if (row < mScriptPaths.GetCount())
				rows.Add( row );
		}

		if (rows.GetCount() == 0)
		{
			return;
		}

		// Sort in descending order so that the last
		// row will be deleted first. Otherwise the
		// remaining indeces would all be wrong.
		rows.Sort( ScriptDataViewModel::SortR );
		for (i = 0; i < rows.GetCount(); i++)
			mScriptPaths.RemoveAt( rows[i] );

		// This is just to test if wxDataViewCtrl can
		// cope with removing rows not sorted in
		// descending order
		rows.Sort( ScriptDataViewModel::Sort );
		RowsDeleted( rows );
	}

protected:
	virtual unsigned int GetColumnCount() const
	{
		return 1;
	}

	virtual wxString GetColumnType( unsigned int col ) const
	{
		return wxT("string");
	}

	virtual void GetValueByRow( wxVariant &variant, unsigned int row, unsigned int col ) const
	{
		if( row < mScriptPaths.size() )
			variant = mScriptPaths[row];
	}

	virtual bool SetValueByRow( const wxVariant &variant, unsigned int row, unsigned int col )
	{
		if( row < mScriptPaths.size() )
			mScriptPaths[row] = variant.GetString();

		return true;
	}

};

/*!
 * ScriptManager class declaration
 */

class ScriptManagerDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( ScriptManagerDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ScriptManagerDlg();
    ScriptManagerDlg( wxWindow* parent, wxWindowID id = SYMBOL_SCRIPTMANAGER_IDNAME, const wxString& caption = SYMBOL_SCRIPTMANAGER_TITLE, const wxPoint& pos = SYMBOL_SCRIPTMANAGER_POSITION, const wxSize& size = SYMBOL_SCRIPTMANAGER_SIZE, long style = SYMBOL_SCRIPTMANAGER_STYLE );

	/// Destructor
	~ScriptManagerDlg();

	void OnButtonAddClick( wxCommandEvent &event );
	void OnButtonDelClick( wxCommandEvent &event );

private:
    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_SCRIPTMANAGER_IDNAME, const wxString& caption = SYMBOL_SCRIPTMANAGER_TITLE, const wxPoint& pos = SYMBOL_SCRIPTMANAGER_POSITION, const wxSize& size = SYMBOL_SCRIPTMANAGER_SIZE, long style = SYMBOL_SCRIPTMANAGER_STYLE );

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin ScriptManager event handler declarations

////@end ScriptManager event handler declarations

////@begin ScriptManager member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ScriptManager member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin ScriptManager member variables
    wxDataViewCtrl* mScriptListCtrl;
////@end ScriptManager member variables

	ScriptDataViewModel mScriptListModel;
};

#endif
// _SCRIPTMANAGERDLG_H_
