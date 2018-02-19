///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file DataViewFileSelecterRenderer.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// �ļ�ѡ����Ⱦ��
///
///////////////////////////////////////////////////////////////
#pragma once

#ifndef _DATAVIEWFILESELECTERRENDERER_H_
#define _DATAVIEWFILESELECTERRENDERER_H_

#include <wx/dataview.h>
class wxDataViewFileSelecterRenderer : public wxDataViewCustomRenderer
{
public:
	wxDataViewFileSelecterRenderer( const wxString& strRootPath );
	~wxDataViewFileSelecterRenderer();

	virtual bool HasEditorCtrl() const { return true; }
	virtual wxWindow* CreateEditorCtrl( wxWindow *parent, wxRect labelRect, const wxVariant &value );
	virtual bool GetValueFromEditorCtrl( wxWindow* editor, wxVariant &value );
	virtual bool Render( wxRect rect, wxDC *dc, int state );
	virtual wxSize GetSize() const;
	virtual bool SetValue( const wxVariant &value );
	virtual bool GetValue( wxVariant &value ) const;

private:
	wxString mPathName;
};

#endif // _DATAVIEWFILESELECTERRENDERER_H_