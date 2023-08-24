///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.10.1-0-g8feb16b3)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
class STD_BITMAP_BUTTON;

#include "dialog_shim.h"
#include <wx/string.h>
#include <wx/listbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/bmpbuttn.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_GRID_SETTINGS_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_GRID_SETTINGS_BASE : public DIALOG_SHIM
{
	private:

	protected:
		wxListBox* m_currentGridCtrl;
		STD_BITMAP_BUTTON* m_addGridButton;
		STD_BITMAP_BUTTON* m_removeGridButton;
		STD_BITMAP_BUTTON* m_moveUpButton;
		STD_BITMAP_BUTTON* m_moveDownButton;
		wxStaticBoxSizer* sbGridOriginSizer;
		wxStaticText* m_staticTextGridPosX;
		wxTextCtrl* m_GridOriginXCtrl;
		wxStaticText* m_TextPosXUnits;
		wxStaticText* m_staticTextGridPosY;
		wxTextCtrl* m_GridOriginYCtrl;
		wxStaticText* m_TextPosYUnits;
		wxStaticText* m_staticTextGrid1;
		wxChoice* m_grid1Ctrl;
		wxStaticText* m_grid1HotKey;
		wxStaticText* m_staticTextGrid2;
		wxChoice* m_grid2Ctrl;
		wxStaticText* m_grid2HotKey;
		wxStaticBoxSizer* sbGridOverridesSizer;
		wxCheckBox* m_checkGridOverrideConnectables;
		wxStaticText* m_staticTextConnectables;
		wxTextCtrl* m_GridOverrideConnectablesSize;
		wxStaticText* m_staticTextConnectablesUnits;
		wxCheckBox* m_checkGridOverrideWires;
		wxStaticText* m_staticTextWires;
		wxTextCtrl* m_GridOverrideWiresSize;
		wxStaticText* m_staticTextWiresUnits;
		wxCheckBox* m_checkGridOverrideVias;
		wxStaticText* m_staticTextVias;
		wxTextCtrl* m_GridOverrideViasSize;
		wxStaticText* m_staticTextViasUnits;
		wxCheckBox* m_checkGridOverrideText;
		wxStaticText* m_staticTextText;
		wxTextCtrl* m_GridOverrideTextSize;
		wxStaticText* m_staticTextTextUnits;
		wxCheckBox* m_checkGridOverrideGraphics;
		wxStaticText* m_staticTextGraphics;
		wxTextCtrl* m_GridOverrideGraphicsSize;
		wxStaticText* m_staticTextGraphicsUnits;
		wxButton* m_buttonResetSizes;
		wxStdDialogButtonSizer* m_sdbSizer;
		wxButton* m_sdbSizerOK;
		wxButton* m_sdbSizerCancel;

		// Virtual event handlers, override them in your derived class
		virtual void OnInitDlg( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void OnAddGrid( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRemoveGrid( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMoveGridUp( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMoveGridDown( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancelClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOkClick( wxCommandEvent& event ) { event.Skip(); }


	public:

		DIALOG_GRID_SETTINGS_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Grid Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );

		~DIALOG_GRID_SETTINGS_BASE();

};

