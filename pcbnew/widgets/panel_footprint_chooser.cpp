/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 Henner Zeller <h.zeller@acm.org>
 * Copyright (C) 2016-2023 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <widgets/panel_footprint_chooser.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/timer.h>
#include <wx/wxhtml.h>
#include <pcb_base_frame.h>
#include <pcbnew_settings.h>
#include <pgm_base.h>
#include <fp_lib_table.h>
#include <settings/settings_manager.h>
#include <widgets/lib_tree.h>
#include <widgets/footprint_preview_widget.h>
#include <widgets/wx_progress_reporters.h>
#include <footprint_info_impl.h>
#include <project_pcb.h>


PANEL_FOOTPRINT_CHOOSER::PANEL_FOOTPRINT_CHOOSER( PCB_BASE_FRAME* aFrame, wxTopLevelWindow* aParent,
                                                  const wxArrayString& aFootprintHistoryList,
                                                  std::function<void()> aCloseHandler ) :
        wxPanel( aParent, wxID_ANY, wxDefaultPosition, wxDefaultSize ),
        m_hsplitter( nullptr ),
        m_vsplitter( nullptr ),
        m_frame( aFrame ),
        m_closeHandler( std::move( aCloseHandler ) )
{
    FP_LIB_TABLE*   fpTable = PROJECT_PCB::PcbFootprintLibs( &aFrame->Prj() );

    // Load footprint files:
    WX_PROGRESS_REPORTER* progressReporter = new WX_PROGRESS_REPORTER( aParent,
                                                    _( "Loading Footprint Libraries" ), 3 );
    GFootprintList.ReadFootprintFiles( fpTable, nullptr, progressReporter );

    // Force immediate deletion of the WX_PROGRESS_REPORTER.  Do not use Destroy(), or use
    // Destroy() followed by wxSafeYield() because on Windows, APP_PROGRESS_DIALOG and
    // WX_PROGRESS_REPORTER have some side effects on the event loop manager.  For instance, a
    // subsequent call to ShowModal() or ShowQuasiModal() for a dialog following the use of a
    // WX_PROGRESS_REPORTER results in incorrect modal or quasi modal behavior.
    delete progressReporter;

    if( GFootprintList.GetErrorCount() )
        GFootprintList.DisplayErrors( aParent );

    m_adapter = FP_TREE_MODEL_ADAPTER::Create( aFrame, fpTable );
    FP_TREE_MODEL_ADAPTER* adapter = static_cast<FP_TREE_MODEL_ADAPTER*>( m_adapter.get() );

    std::vector<LIB_TREE_ITEM*> historyInfos;

    for( const wxString& item : aFootprintHistoryList )
    {
        LIB_TREE_ITEM* fp_info = GFootprintList.GetFootprintInfo( item );

        // this can be null, for example, if the footprint has been deleted from a library.
        if( fp_info != nullptr )
            historyInfos.push_back( fp_info );
    }

    adapter->DoAddLibrary( wxT( "-- " ) + _( "Recently Used" ) + wxT( " --" ), wxEmptyString,
                           historyInfos, false, true );

    if( historyInfos.size() )
        adapter->SetPreselectNode( historyInfos[0]->GetLibId(), 0 );

    adapter->AddLibraries( aFrame );

    wxBoxSizer*   sizer = new wxBoxSizer( wxVERTICAL );
    HTML_WINDOW*  details = nullptr;

    m_vsplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                        wxSP_LIVE_UPDATE | wxSP_3DSASH );

    m_hsplitter = new wxSplitterWindow( m_vsplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                        wxSP_LIVE_UPDATE | wxSP_3DSASH );

    //Avoid the splitter window being assigned as the Parent to additional windows
    m_hsplitter->SetExtraStyle( wxWS_EX_TRANSIENT );

    auto detailsPanel = new wxPanel( m_vsplitter );
    auto detailsSizer = new wxBoxSizer( wxVERTICAL );
    detailsPanel->SetSizer( detailsSizer );

    details = new HTML_WINDOW( detailsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                               wxHW_SCROLLBAR_AUTO );
    detailsSizer->Add( details, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5 );
    detailsPanel->Layout();
    detailsSizer->Fit( detailsPanel );

    m_vsplitter->SetSashGravity( 0.5 );
    m_vsplitter->SetMinimumPaneSize( 20 );
    m_vsplitter->SplitHorizontally( m_hsplitter, detailsPanel );

    sizer->Add( m_vsplitter, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5 );

    m_tree = new LIB_TREE( m_hsplitter, wxT( "footprints" ), fpTable, m_adapter,
                           LIB_TREE::FLAGS::ALL_WIDGETS, details );

    m_hsplitter->SetSashGravity( 0.8 );
    m_hsplitter->SetMinimumPaneSize( 20 );

    wxPanel*    rightPanel = new wxPanel( m_hsplitter );
    wxBoxSizer* rightPanelSizer = new wxBoxSizer( wxVERTICAL );

    m_preview_ctrl = new FOOTPRINT_PREVIEW_WIDGET( rightPanel, m_frame->Kiway() );
    m_preview_ctrl->SetUserUnits( m_frame->GetUserUnits() );
    rightPanelSizer->Add( m_preview_ctrl, 1, wxEXPAND | wxTOP | wxRIGHT, 5 );

    rightPanel->SetSizer( rightPanelSizer );
    rightPanel->Layout();
    rightPanelSizer->Fit( rightPanel );

    m_hsplitter->SplitVertically( m_tree, rightPanel );

    m_dbl_click_timer = new wxTimer( this );

    SetSizer( sizer );

    m_adapter->FinishTreeInitialization();

    Bind( wxEVT_TIMER, &PANEL_FOOTPRINT_CHOOSER::onCloseTimer, this, m_dbl_click_timer->GetId() );
    Bind( SYMBOL_PRESELECTED, &PANEL_FOOTPRINT_CHOOSER::onComponentPreselected, this );
    Bind( SYMBOL_SELECTED, &PANEL_FOOTPRINT_CHOOSER::onComponentSelected, this );

    Layout();
}


PANEL_FOOTPRINT_CHOOSER::~PANEL_FOOTPRINT_CHOOSER()
{
    Unbind( wxEVT_TIMER, &PANEL_FOOTPRINT_CHOOSER::onCloseTimer, this );
    Unbind( SYMBOL_PRESELECTED, &PANEL_FOOTPRINT_CHOOSER::onComponentPreselected, this );
    Unbind( SYMBOL_SELECTED, &PANEL_FOOTPRINT_CHOOSER::onComponentSelected, this );

    // I am not sure the following two lines are necessary, but they will not hurt anyone
    m_dbl_click_timer->Stop();
    delete m_dbl_click_timer;

    if( PCBNEW_SETTINGS* cfg = Pgm().GetSettingsManager().GetAppSettings<PCBNEW_SETTINGS>() )
    {
        // Save any changes to column widths, etc.
        m_adapter->SaveSettings();

        cfg->m_FootprintChooser.width = GetParent()->GetSize().x;
        cfg->m_FootprintChooser.height = GetParent()->GetSize().y;
        cfg->m_FootprintChooser.sash_h = m_hsplitter->GetSashPosition();

        if( m_vsplitter )
            cfg->m_FootprintChooser.sash_v = m_vsplitter->GetSashPosition();

        cfg->m_FootprintChooser.sort_mode = m_tree->GetSortMode();
    }
}


void PANEL_FOOTPRINT_CHOOSER::FinishSetup()
{
    if( PCBNEW_SETTINGS* settings = Pgm().GetSettingsManager().GetAppSettings<PCBNEW_SETTINGS>() )
    {
        auto horizPixelsFromDU =
                [&]( int x ) -> int
                {
                    wxSize sz( x, 0 );
                    return GetParent()->ConvertDialogToPixels( sz ).x;
                };

        PCBNEW_SETTINGS::FOOTPRINT_CHOOSER& cfg = settings->m_FootprintChooser;

        int w = cfg.width < 0 ? horizPixelsFromDU( 440 ) : cfg.width;
        int h = cfg.height < 0 ? horizPixelsFromDU( 340 ) : cfg.height;

        GetParent()->SetSize( wxSize( w, h ) );
        GetParent()->Layout();

        // We specify the width of the right window (m_symbol_view_panel), because specify
        // the width of the left window does not work as expected when SetSashGravity() is called
        if( cfg.sash_h < 0 )
            cfg.sash_h = horizPixelsFromDU( 220 );

        m_hsplitter->SetSashPosition( cfg.sash_h );

        if( cfg.sash_v < 0 )
            cfg.sash_v = horizPixelsFromDU( 230 );

        if( m_vsplitter )
            m_vsplitter->SetSashPosition( cfg.sash_v );

        m_adapter->SetSortMode( (LIB_TREE_MODEL_ADAPTER::SORT_MODE) cfg.sort_mode );
    }
}


void PANEL_FOOTPRINT_CHOOSER::SetPreselect( const LIB_ID& aPreselect )
{
    m_adapter->SetPreselectNode( aPreselect, 0 );
}


LIB_ID PANEL_FOOTPRINT_CHOOSER::GetSelectedLibId() const
{
    return m_tree->GetSelectedLibId();
}


void PANEL_FOOTPRINT_CHOOSER::onCloseTimer( wxTimerEvent& aEvent )
{
    // Hack because of eaten MouseUp event. See PANEL_FOOTPRINT_CHOOSER::onComponentSelected
    // for the beginning of this spaghetti noodle.

    auto state = wxGetMouseState();

    if( state.LeftIsDown() )
    {
        // Mouse hasn't been raised yet, so fire the timer again. Otherwise the
        // purpose of this timer is defeated.
        m_dbl_click_timer->StartOnce( PANEL_FOOTPRINT_CHOOSER::DblClickDelay );
    }
    else
    {
        m_closeHandler();
    }
}


void PANEL_FOOTPRINT_CHOOSER::onComponentPreselected( wxCommandEvent& aEvent )
{
    if( !m_preview_ctrl || !m_preview_ctrl->IsInitialized() )
        return;

    LIB_ID lib_id = m_tree->GetSelectedLibId();

    if( !lib_id.IsValid() )
    {
        m_preview_ctrl->SetStatusText( _( "No footprint selected" ) );
    }
    else
    {
        m_preview_ctrl->ClearStatus();
        m_preview_ctrl->DisplayFootprint( lib_id );
    }
}


void PANEL_FOOTPRINT_CHOOSER::onComponentSelected( wxCommandEvent& aEvent )
{
    if( m_tree->GetSelectedLibId().IsValid() )
    {
        // Got a selection. We can't just end the modal dialog here, because wx leaks some
        // events back to the parent window (in particular, the MouseUp following a double click).
        //
        // NOW, here's where it gets really fun. wxTreeListCtrl eats MouseUp.  This isn't really
        // feasible to bypass without a fully custom wxDataViewCtrl implementation, and even then
        // might not be fully possible (docs are vague). To get around this, we use a one-shot
        // timer to schedule the dialog close.
        //
        // See PANEL_FOOTPRINT_CHOOSER::onCloseTimer for the other end of this spaghetti noodle.
        m_dbl_click_timer->StartOnce( PANEL_FOOTPRINT_CHOOSER::DblClickDelay );
    }
}
