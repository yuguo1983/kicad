/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2013 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2013 Wayne Stambaugh <stambaughw@gmail.com>
 * Copyright (C) 1992-2018 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <bitmaps.h>
#include <board_commit.h>
#include <class_board.h>
#include <class_module.h>
#include <dialog_exchange_footprints.h>
#include <kicad_string.h>
#include <kiway.h>
#include <pcb_edit_frame.h>
#include <pcbnew_settings.h>
#include <project.h>
#include <wx_html_report_panel.h>


#define ID_MATCH_FP_ALL     4200
#define ID_MATCH_FP_SELECTED 4201
#define ID_MATCH_FP_REF     4202
#define ID_MATCH_FP_VAL     4203
#define ID_MATCH_FP_ID      4204


int g_matchModeForUpdate           = ID_MATCH_FP_ALL;
int g_matchModeForUpdateSelected   = ID_MATCH_FP_SELECTED;
int g_matchModeForExchange         = ID_MATCH_FP_REF;
int g_matchModeForExchangeSelected = ID_MATCH_FP_SELECTED;

bool g_removeExtraTextItems[2]  = { false, false };
bool g_resetTextItemLayers[2]   = { false, true };
bool g_resetTextItemEffects[2]  = { false, true };
bool g_resetFabricationAttrs[2] = { false, true };
bool g_reset3DModels[2]         = { false, true };


DIALOG_EXCHANGE_FOOTPRINTS::DIALOG_EXCHANGE_FOOTPRINTS( PCB_EDIT_FRAME* aParent, MODULE* aModule,
                                                        bool updateMode, bool selectedMode ) :
    DIALOG_EXCHANGE_FOOTPRINTS_BASE( aParent ),
    m_commit( aParent ),
    m_parent( aParent ),
    m_currentModule( aModule ),
    m_updateMode( updateMode )
{
    wxString title = updateMode ? _( "Update Footprints from Library" ) : _( "Change Footprints" );
    wxString verb  = updateMode ? _( "Update" )                         : _( "Change" );
    wxString reset = updateMode ? _( "Reset" )                          : _( "Update" );
    wxString label;

    SetTitle( title );

    if( m_updateMode )
    {
        label.Printf( m_matchAll->GetLabel(), verb );
        m_matchAll->SetLabel( label );
        m_changeSizer->Show( false );
    }
    else
    {
        m_upperSizer->FindItem( m_matchAll )->Show( false );
        m_newIDBrowseButton->SetBitmap( KiBitmap( small_library_xpm ) );
    }

    if( m_currentModule )
    {
        label.Printf( m_matchSelected->GetLabel(), verb );
        m_matchSelected->SetLabel( label );
        m_newID->AppendText( FROM_UTF8( m_currentModule->GetFPID().Format().c_str() ) );
    }
    else
        m_upperSizer->FindItem( m_matchSelected )->Show( false );

    label.Printf( m_matchSpecifiedRef->GetLabel(), verb );
    m_matchSpecifiedRef->SetLabel( label );

    // Use ChangeValue() instead of SetValue() so we don't generate events.
    if( m_currentModule )
        m_specifiedRef->ChangeValue( m_currentModule->GetReference() );

    label.Printf( m_matchSpecifiedValue->GetLabel(), verb );
    m_matchSpecifiedValue->SetLabel( label );

    if( m_currentModule )
        m_specifiedValue->ChangeValue( m_currentModule->GetValue() );

    label.Printf( m_matchSpecifiedID->GetLabel(), verb );
    m_matchSpecifiedID->SetLabel( label );

    if( m_currentModule )
        m_specifiedID->ChangeValue( FROM_UTF8( m_currentModule->GetFPID().Format().c_str() ) );

    m_specifiedIDBrowseButton->SetBitmap( KiBitmap( small_library_xpm ) );

    m_upperSizer->SetEmptyCellSize( wxSize( 0, 0 ) );
    // The upper sizer has its content modified: re-layout it:
    m_upperSizer->Layout();

    // initialize match-mode
    if( m_updateMode )
        m_matchMode = selectedMode ? &g_matchModeForUpdateSelected : &g_matchModeForUpdate;
    else
        m_matchMode = selectedMode ? &g_matchModeForExchangeSelected : &g_matchModeForExchange;

    wxCommandEvent event;
    event.SetEventObject( this );

    switch( *m_matchMode )
    {
    case ID_MATCH_FP_ALL:      OnMatchAllClicked( event );      break;
    case ID_MATCH_FP_SELECTED: OnMatchSelectedClicked( event ); break;
    case ID_MATCH_FP_REF:      OnMatchRefClicked( event );      break;
    case ID_MATCH_FP_VAL:      OnMatchValueClicked( event );    break;
    case ID_MATCH_FP_ID:       OnMatchIDClicked( event );       break;
    default:                                                    break;
    }

    label.Printf( m_resetTextItemLayers->GetLabel(), reset );
    m_resetTextItemLayers->SetLabel( label );

    label.Printf( m_resetTextItemEffects->GetLabel(), reset );
    m_resetTextItemEffects->SetLabel( label );

    label.Printf( m_resetFabricationAttrs->GetLabel(), reset );
    m_resetFabricationAttrs->SetLabel( label );

    label.Printf( m_reset3DModels->GetLabel(), reset );
    m_reset3DModels->SetLabel( label );

    m_removeExtraBox->SetValue( g_removeExtraTextItems[ m_updateMode ? 0 : 1 ] );
    m_resetTextItemLayers->SetValue( g_resetTextItemLayers[ m_updateMode ? 0 : 1 ] );
    m_resetTextItemEffects->SetValue( g_resetTextItemEffects[ m_updateMode ? 0 : 1 ] );
    m_resetFabricationAttrs->SetValue( g_resetFabricationAttrs[ m_updateMode ? 0 : 1 ] );
    m_reset3DModels->SetValue( g_reset3DModels[ m_updateMode ? 0 : 1 ] );

    m_MessageWindow->SetLazyUpdate( true );

    // DIALOG_SHIM needs a unique hash_key because classname is not sufficient
    // because the update and change versions of this dialog have different controls.
    m_hash_key = TO_UTF8( GetTitle() );

    // Ensure m_closeButton (with id = wxID_CANCEL) has the right label
    // (to fix automatic renaming of button label )
    m_sdbSizerCancel->SetLabel( _( "Close" ) );
    m_sdbSizerOK->SetLabel( verb );
    m_sdbSizerOK->SetDefault();

    // Now all widgets have the size fixed, call FinishDialogSettings
    FinishDialogSettings();
}


DIALOG_EXCHANGE_FOOTPRINTS::~DIALOG_EXCHANGE_FOOTPRINTS()
{
    g_removeExtraTextItems[ m_updateMode ? 0 : 1 ]  = m_removeExtraBox->GetValue();
    g_resetTextItemLayers[ m_updateMode ? 0 : 1 ]   = m_resetTextItemLayers->GetValue();
    g_resetTextItemEffects[ m_updateMode ? 0 : 1 ]  = m_resetTextItemEffects->GetValue();
    g_resetFabricationAttrs[ m_updateMode ? 0 : 1 ] = m_resetFabricationAttrs->GetValue();
    g_reset3DModels[ m_updateMode ? 0 : 1 ]         = m_reset3DModels->GetValue();
}


bool DIALOG_EXCHANGE_FOOTPRINTS::isMatch( MODULE* aModule )
{
    LIB_ID specifiedID;

    switch( *m_matchMode )
    {
    case ID_MATCH_FP_ALL:
        return true;
    case ID_MATCH_FP_SELECTED:
        return aModule == m_currentModule;
    case ID_MATCH_FP_REF:
        return WildCompareString( m_specifiedRef->GetValue(), aModule->GetReference(), false );
    case ID_MATCH_FP_VAL:
        return WildCompareString( m_specifiedValue->GetValue(), aModule->GetValue(), false );
    case ID_MATCH_FP_ID:
        specifiedID.Parse( m_specifiedID->GetValue(), LIB_ID::ID_PCB );
        return aModule->GetFPID() == specifiedID;
    default:
        return false;   // just to quiet compiler warnings....
    }
}


wxRadioButton* DIALOG_EXCHANGE_FOOTPRINTS::getRadioButtonForMode()
{
    switch( *m_matchMode )
    {
    case ID_MATCH_FP_ALL:      return m_matchAll;
    case ID_MATCH_FP_SELECTED: return m_matchSelected;
    case ID_MATCH_FP_REF:      return m_matchSpecifiedRef;
    case ID_MATCH_FP_VAL:      return m_matchSpecifiedValue;
    case ID_MATCH_FP_ID:       return m_matchSpecifiedID;
    default:                   return nullptr;
    }
}


void DIALOG_EXCHANGE_FOOTPRINTS::updateMatchModeRadioButtons( wxUpdateUIEvent& )
{
    wxRadioButton* rb_button = getRadioButtonForMode();

    wxRadioButton* rb_butt_list[] =
    {
        m_matchAll,
        m_matchSelected,
        m_matchSpecifiedRef,
        m_matchSpecifiedValue,
        m_matchSpecifiedID,
        nullptr     // end of list
    };

    // Ensure the button state is ok. Only one button can be checked
    // Change button state only if its state is incorrect, otherwise
    // we have issues depending on the platform.
    for( int ii = 0; rb_butt_list[ii]; ++ii )
    {
        bool state = rb_butt_list[ii] == rb_button;

        if( rb_butt_list[ii]->GetValue() != state )
            rb_butt_list[ii]->SetValue( state );
    }
}


void DIALOG_EXCHANGE_FOOTPRINTS::OnMatchAllClicked( wxCommandEvent& event )
{
    *m_matchMode = ID_MATCH_FP_ALL;

    if( event.GetEventObject() == this )
        SetInitialFocus( m_matchAll );
    else
        m_matchAll->SetFocus();
}


void DIALOG_EXCHANGE_FOOTPRINTS::OnMatchSelectedClicked( wxCommandEvent& event )
{
    *m_matchMode = ID_MATCH_FP_SELECTED;

    if( event.GetEventObject() == this )
        SetInitialFocus( m_matchSelected );
    else
        m_matchSelected->SetFocus();
}


void DIALOG_EXCHANGE_FOOTPRINTS::OnMatchRefClicked( wxCommandEvent& event )
{
    *m_matchMode = ID_MATCH_FP_REF;

    if( event.GetEventObject() == this )
        SetInitialFocus( m_specifiedRef );
    else if( event.GetEventObject() != m_specifiedRef )
        m_specifiedRef->SetFocus();
}


void DIALOG_EXCHANGE_FOOTPRINTS::OnMatchValueClicked( wxCommandEvent& event )
{
    *m_matchMode = ID_MATCH_FP_VAL;

    if( event.GetEventObject() == this )
        SetInitialFocus( m_specifiedValue );
    else if( event.GetEventObject() != m_specifiedValue )
        m_specifiedValue->SetFocus();
}


void DIALOG_EXCHANGE_FOOTPRINTS::OnMatchIDClicked( wxCommandEvent& event )
{
    *m_matchMode = ID_MATCH_FP_ID;

    if( event.GetEventObject() == this )
        SetInitialFocus( m_specifiedID );
    else if( event.GetEventObject() != m_specifiedID )
        m_specifiedID->SetFocus();
}


void DIALOG_EXCHANGE_FOOTPRINTS::OnOKClicked( wxCommandEvent& event )
{
    wxBusyCursor dummy;

    m_MessageWindow->Clear();
    m_MessageWindow->Flush( false );

    if( processMatchingModules() )
    {
        m_parent->Compile_Ratsnest( true );
        m_parent->GetCanvas()->Refresh();
    }

    m_MessageWindow->Flush( false );

    m_commit.Push( wxT( "Changed footprint" ) );
}


bool DIALOG_EXCHANGE_FOOTPRINTS::processMatchingModules()
{
    bool     change = false;
    LIB_ID   newFPID;
    wxString value;

    if( m_parent->GetBoard()->Modules().empty() )
        return false;

    if( !m_updateMode )
    {
        newFPID.Parse( m_newID->GetValue(), LIB_ID::ID_PCB );

        if( !newFPID.IsValid() )
            return false;
    }

    /* The change is done from the last module because processModule() modifies the last item
     * in the list.
     */
    for( auto it = m_parent->GetBoard()->Modules().rbegin();
            it != m_parent->GetBoard()->Modules().rend(); it++ )
    {
        auto mod = *it;

        if( !isMatch( mod ) )
            continue;

        if( m_updateMode )
        {
            if( processModule( mod, mod->GetFPID() ) )
                change = true;
        }
        else
        {
            if( processModule( mod, newFPID ) )
                change = true;
        }
    }

    return change;
}


bool DIALOG_EXCHANGE_FOOTPRINTS::processModule( MODULE* aModule, const LIB_ID& aNewFPID )
{
    LIB_ID    oldFPID = aModule->GetFPID();
    wxString  msg;

    // Load new module.
    msg.Printf( _( "%s footprint \"%s\" (from \"%s\") to \"%s\"" ),
                m_updateMode ? _( "Update" ) : _( "Change" ),
                aModule->GetReference(),
                oldFPID.Format().c_str(),
                aNewFPID.Format().c_str() );

    MODULE* newModule = m_parent->LoadFootprint( aNewFPID );

    if( !newModule )
    {
        msg << ": " << _( "*** footprint not found ***" );
        m_MessageWindow->Report( msg, RPT_SEVERITY_ERROR );
        return false;
    }

    m_parent->Exchange_Module( aModule, newModule, m_commit,
                               m_removeExtraBox->GetValue(),
                               m_resetTextItemLayers->GetValue(),
                               m_resetTextItemEffects->GetValue(),
                               m_resetFabricationAttrs->GetValue(),
                               m_reset3DModels->GetValue() );

    if( aModule == m_currentModule )
        m_currentModule = newModule;

    msg += ": OK";
    m_MessageWindow->Report( msg, RPT_SEVERITY_ACTION );

    return true;
}


void processTextItem( const TEXTE_MODULE& aSrc, TEXTE_MODULE& aDest,
                      bool resetText, bool resetTextLayers, bool resetTextEffects )
{
    if( !resetText )
        aDest.SetText( aSrc.GetText() );

    if( !resetTextLayers )
    {
        aDest.SetLayer( aSrc.GetLayer() );
        aDest.SetVisible( aSrc.IsVisible() );
    }

    if( !resetTextEffects )
    {
        // Careful: the visible bit is also in Effects
        bool visible = aDest.IsVisible();
        aDest.SetEffects( aSrc );
        aDest.SetVisible( visible );
    }
}


TEXTE_MODULE* getMatchingTextItem( TEXTE_MODULE* aRefItem, MODULE* aModule )
{
    std::vector<TEXTE_MODULE*> candidates;

    for( BOARD_ITEM* item : aModule->GraphicalItems() )
    {
        TEXTE_MODULE* candidate = dyn_cast<TEXTE_MODULE*>( item );

        if( candidate && candidate->GetText() == aRefItem->GetText() )
            candidates.push_back( candidate );
    }

    if( candidates.size() == 0 )
        return nullptr;

    if( candidates.size() == 1 )
        return candidates[0];

    // Try refining the match by layer
    std::vector<TEXTE_MODULE*> candidatesOnSameLayer;

    for( TEXTE_MODULE* candidate : candidates )
    {
        if( candidate->GetLayer() == aRefItem->GetLayer() )
            candidatesOnSameLayer.push_back( candidate );
    }

    if( candidatesOnSameLayer.size() == 1 )
        return candidatesOnSameLayer[0];

    // Last ditch effort: refine by position
    std::vector<TEXTE_MODULE*> candidatesAtSamePos;

    for( TEXTE_MODULE* candidate : candidatesOnSameLayer.size() ? candidatesOnSameLayer : candidates )
    {
        if( candidate->GetPos0() == aRefItem->GetPos0() )
            candidatesAtSamePos.push_back( candidate );
    }

    if( candidatesAtSamePos.size() > 0 )
        return candidatesAtSamePos[0];
    else if( candidatesOnSameLayer.size() > 0 )
        return candidatesOnSameLayer[0];
    else
        return candidates[0];
}


void PCB_EDIT_FRAME::Exchange_Module( MODULE* aExisting, MODULE* aNew, BOARD_COMMIT& aCommit,
                                      bool deleteExtraTexts, bool resetTextLayers,
                                      bool resetTextEffects, bool resetFabricationAttrs,
                                      bool reset3DModels )
{
    PCB_GROUP* parentGroup = aExisting->GetParentGroup();

    if( parentGroup )
    {
        parentGroup->RemoveItem( aExisting );
        parentGroup->AddItem( aNew );
    }

    aNew->SetParent( GetBoard() );

    PlaceModule( aNew, false );

    // PlaceModule will move the module to the cursor position, which we don't want.  Copy
    // the original position across.
    aNew->SetPosition( aExisting->GetPosition() );

    if( aNew->GetLayer() != aExisting->GetLayer() )
        aNew->Flip( aNew->GetPosition(), m_settings->m_FlipLeftRight );

    if( aNew->GetOrientation() != aExisting->GetOrientation() )
        aNew->SetOrientation( aExisting->GetOrientation() );

    aNew->SetLocked( aExisting->IsLocked() );

    for( D_PAD* pad : aNew->Pads() )
    {
        D_PAD* oldPad = aExisting->FindPadByName( pad->GetName() );

        if( oldPad )
        {
            pad->SetLocalRatsnestVisible( oldPad->GetLocalRatsnestVisible() );
            pad->SetNetCode( oldPad->GetNetCode() );
            pad->SetPinFunction( oldPad->GetPinFunction() );
        }
    }

    // Copy reference
    processTextItem( aExisting->Reference(), aNew->Reference(),
                     // never reset reference text
                     false,
                     resetTextLayers, resetTextEffects );

    // Copy value
    processTextItem( aExisting->Value(), aNew->Value(),
                     // reset value text only when it is a proxy for the footprint ID
                     // (cf replacing value "MountingHole-2.5mm" with "MountingHole-4.0mm")
                     aExisting->GetValue() == aExisting->GetFPID().GetLibItemName(),
                     resetTextLayers, resetTextEffects );

    // Copy fields in accordance with the reset* flags
    for( BOARD_ITEM* item : aExisting->GraphicalItems() )
    {
        TEXTE_MODULE* srcItem = dyn_cast<TEXTE_MODULE*>( item );

        if( srcItem )
        {
            TEXTE_MODULE* destItem = getMatchingTextItem( srcItem, aNew );

            if( destItem )
                processTextItem( *srcItem, *destItem, false, resetTextLayers, resetTextEffects );
            else if( !deleteExtraTexts )
                aNew->Add( new TEXTE_MODULE( *srcItem ) );
        }
    }

    if( !resetFabricationAttrs )
        aNew->SetAttributes( aExisting->GetAttributes() );

    // Copy 3D model settings in accordance with the reset* flag
    if( !reset3DModels )
        aNew->Models() = aExisting->Models();  // Linked list of 3D models.

    // Updating other parameters
    const_cast<KIID&>( aNew->m_Uuid ) = aExisting->m_Uuid;
    aNew->SetProperties( aExisting->GetProperties() );
    aNew->SetPath( aExisting->GetPath() );
    aNew->CalculateBoundingBox();

    aCommit.Remove( aExisting );
    aCommit.Add( aNew );

    aNew->ClearFlags();
}


void DIALOG_EXCHANGE_FOOTPRINTS::ViewAndSelectFootprint( wxCommandEvent& event )
{
    wxString newname = m_newID->GetValue();

    KIWAY_PLAYER* frame = Kiway().Player( FRAME_FOOTPRINT_VIEWER_MODAL, true );

    if( frame->ShowModal( &newname, this ) )
    {
        if( event.GetEventObject() == m_newIDBrowseButton )
            m_newID->SetValue( newname );
        else
            m_specifiedID->SetValue( newname );
    }

    frame->Destroy();
}


