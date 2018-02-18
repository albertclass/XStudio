#include "stdafx.h"
#include <wx/combo.h>
#include "DataViewFileSelecterRenderer.h"

// ----------------------------------------------------------------------------
// wxComboCtrl with entirely custom button action (opens file dialog)
// ----------------------------------------------------------------------------

class wxFileSelectorCombo : public wxComboCtrl
{ 
public:
	wxFileSelectorCombo() 
		: wxComboCtrl() 
	{ 
		Init(); 
	}

	wxFileSelectorCombo(wxWindow *parent,
						 wxWindowID id = wxID_ANY,
						 const wxString& value = wxEmptyString,
						 const wxPoint& pos = wxDefaultPosition,
						 const wxSize& size = wxDefaultSize,
						 long style = 0,
						 const wxValidator& validator = wxDefaultValidator,
						 const wxString& name = wxComboBoxNameStr)
		: wxComboCtrl( parent, id, value, pos, size, style | wxCC_STD_BUTTON, validator, name )
	{
		Init();
		//Create(parent,id,value,
		//		pos,size,
		//		// Style flag wxCC_STD_BUTTON makes the button
		//		// behave more like a standard push button.
		//		style | wxCC_STD_BUTTON,
		//		validator,name);

		//
		// Prepare custom button bitmap (just '...' text)
		wxMemoryDC dc;
		wxBitmap bmp(12,16);
		dc.SelectObject(bmp);

		// Draw transparent background
		wxColour magic(255,0,255);
		wxBrush magicBrush(magic);
		dc.SetBrush( magicBrush );
		dc.SetPen( *wxTRANSPARENT_PEN );
		dc.DrawRectangle(0,0,bmp.GetWidth(),bmp.GetHeight());

		// Draw text
		wxString str = wxT("...");
		int w,h;
		dc.GetTextExtent(str, &w, &h, 0, 0);
		dc.DrawText(str, (bmp.GetWidth()-w)/2, (bmp.GetHeight()-h)/2);

		dc.SelectObject( wxNullBitmap );

		// Finalize transparency with a mask
		wxMask *mask = XGC_NEW wxMask( bmp, magic );
		bmp.SetMask( mask );

		SetButtonBitmaps(bmp,true);
	}

	~wxFileSelectorCombo()
	{

	}

	virtual void OnButtonClick()
	{
		// Show standard wxFileDialog on button click

		wxFileDialog dlg(this,
						  wxT("Choose File"),
						  wxEmptyString,
						  GetValue(),
						  wxT("All files (*.*)|*.*"),
						  wxFD_OPEN);

		if ( dlg.ShowModal() == wxID_OK )
		{
			SetValue(dlg.GetPath());
		}
	}

	// Implement empty DoSetPopupControl to prevent assertion failure.
	virtual void DoSetPopupControl(wxComboPopup* WXUNUSED(popup))
	{
	}

private:
	void Init()
	{
		// Initialize member variables here
	}
};

wxDataViewFileSelecterRenderer::wxDataViewFileSelecterRenderer( const wxString& strRootPath )
	: wxDataViewCustomRenderer( wxT("string"), wxDATAVIEW_CELL_EDITABLE, wxALIGN_LEFT )
{
}


wxDataViewFileSelecterRenderer::~wxDataViewFileSelecterRenderer()
{
}

wxWindow * wxDataViewFileSelecterRenderer::CreateEditorCtrl( wxWindow * parent, wxRect labelRect, const wxVariant & value )
{
	wxFileSelectorCombo *c = XGC_NEW wxFileSelectorCombo( parent, wxID_ANY, value,
									 labelRect.GetTopLeft(), wxSize( labelRect.GetWidth(), -1 ) );

	//wxArrayString choices;
	//choices.Add( "0" );
	//choices.Add( "1" );
	//choices.Add( "2" );

	//wxComboBox* c = XGC_NEW wxComboBox(
	//	parent,
	//	wxID_ANY,
	//	wxEmptyString,
	//	labelRect.GetTopLeft(),
	//	wxSize( labelRect.GetWidth(), -1 ),
	//	choices,
	//	wxCB_READONLY );

	//c->Move( labelRect.GetRight() - c->GetRect().width, wxDefaultCoord );
	//c->SetStringSelection( value.GetString() );

#ifdef __WXMAC__
	const wxSize size = sc->GetSize();
	wxPoint pt = sc->GetPosition();
	sc->SetSize( pt.x - 4, pt.y - 4, size.x, size.y );
#endif

	return c;
}

bool wxDataViewFileSelecterRenderer::GetValueFromEditorCtrl( wxWindow * editor, wxVariant & value )
{
	wxFileSelectorCombo *c = (wxFileSelectorCombo*) editor;
	value = c->GetValue();
	return true;
}

bool wxDataViewFileSelecterRenderer::Render( wxRect rect, wxDC * dc, int state )
{
	RenderText( mPathName, 0, rect, dc, state );
	return true;
}

wxSize wxDataViewFileSelecterRenderer::GetSize() const
{
	wxSize sz = GetTextExtent( mPathName );

	// Allow some space for the spin buttons, which is approximately the size
	// of a scrollbar (and getting pixel-exact value would be complicated).
	// Also add some whitespace between the text and the button:
	sz.x += wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
	sz.x += GetTextExtent("M").x;

	return sz;
}

bool wxDataViewFileSelecterRenderer::SetValue( const wxVariant & value )
{
	mPathName = value.GetString();
	return true;
}

bool wxDataViewFileSelecterRenderer::GetValue( wxVariant & value ) const
{
	value = mPathName;
	return true;
}
