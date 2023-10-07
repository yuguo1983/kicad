/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2023 Alex Shvartzkop <dudesuchamazing@gmail.com>
 * Copyright (C) 2023 KiCad Developers, see AUTHORS.txt for contributors.
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

#include "generator_tool.h"

#include <collectors.h>
#include <tool/tool_manager.h>
#include <tools/pcb_selection_tool.h>
#include <tools/pcb_actions.h>

#include <dialog_generators.h>


GENERATOR_TOOL::GENERATOR_TOOL() :
        GENERATOR_TOOL_PNS_PROXY( "pcbnew.Generators" ),
        m_mgrDialog( nullptr )
{
}


GENERATOR_TOOL::~GENERATOR_TOOL()
{
}


void GENERATOR_TOOL::Reset( RESET_REASON aReason )
{
    GENERATOR_TOOL_PNS_PROXY::Reset( aReason );
}


void GENERATOR_TOOL::DestroyManagerDialog()
{
    if( m_mgrDialog )
    {
        m_mgrDialog->Destroy();
        m_mgrDialog = nullptr;
    }
}


int GENERATOR_TOOL::ShowGeneratorsManager( const TOOL_EVENT& aEvent )
{
    PCB_EDIT_FRAME* pcbFrame = static_cast<PCB_EDIT_FRAME*>( frame() );

    if( !pcbFrame )
        return 0;

    if( !m_mgrDialog )
    {
        m_mgrDialog = new DIALOG_GENERATORS( pcbFrame, pcbFrame );
    }
    else
    {
        m_mgrDialog->RebuildModels();
    }

    m_mgrDialog->Show( true );

    return 0;
}


int GENERATOR_TOOL::ShowGeneratorProperties( const TOOL_EVENT& aEvent )
{
    PCB_BASE_EDIT_FRAME* editFrame = getEditFrame<PCB_BASE_EDIT_FRAME>();
    PCB_GENERATOR*       gen = aEvent.Parameter<PCB_GENERATOR*>();

    gen->ShowPropertiesDialog( editFrame );

    return 0;
}


int GENERATOR_TOOL::RegenerateAll( const TOOL_EVENT& aEvent )
{
    BOARD_COMMIT  localCommit( this );
    BOARD_COMMIT* commit = dynamic_cast<BOARD_COMMIT*>( aEvent.Commit() );

    if( !commit )
        commit = &localCommit;

    GENERATORS generators = board()->Generators();

    std::sort( generators.begin(), generators.end(),
               []( const PCB_GENERATOR* a, const PCB_GENERATOR* b ) -> bool
               {
                   return a->GetUpdateOrder() < b->GetUpdateOrder();
               } );

    for( PCB_GENERATOR* gen : generators )
    {
        gen->EditStart( this, board(), frame(), commit );
        gen->Update( this, board(), frame(), commit );
        gen->EditPush( this, board(), frame(), commit );
    }

    return 0;
}


int GENERATOR_TOOL::RegenerateOutdated( const TOOL_EVENT& aEvent )
{
    BOARD_COMMIT  localCommit( this );
    BOARD_COMMIT* commit = dynamic_cast<BOARD_COMMIT*>( aEvent.Commit() );

    if( !commit )
        commit = &localCommit;

    GENERATORS generators = board()->Generators();

    std::sort( generators.begin(), generators.end(),
               []( const PCB_GENERATOR* a, const PCB_GENERATOR* b ) -> bool
               {
                   return a->GetUpdateOrder() < b->GetUpdateOrder();
               } );

    for( PCB_GENERATOR* gen : generators )
    {
        gen->EditStart( this, board(), frame(), commit );
        gen->Update( this, board(), frame(), commit );
        gen->EditPush( this, board(), frame(), commit );
    }

    return 0;
}


int GENERATOR_TOOL::RegenerateSelected( const TOOL_EVENT& aEvent )
{
    BOARD_COMMIT  localCommit( this );
    BOARD_COMMIT* commit = dynamic_cast<BOARD_COMMIT*>( aEvent.Commit() );

    if( !commit )
        commit = &localCommit;

    PCB_SELECTION_TOOL* selTool = m_toolMgr->GetTool<PCB_SELECTION_TOOL>();

    PCB_SELECTION sel = selTool->RequestSelection(
            []( const VECTOR2I& aPt, GENERAL_COLLECTOR& aCollector, PCB_SELECTION_TOOL* sTool )
            {
                // Iterate from the back so we don't have to worry about removals.
                for( int i = aCollector.GetCount() - 1; i >= 0; --i )
                {
                    BOARD_ITEM* item = aCollector[i];

                    if( item->Type() != PCB_GENERATOR_T )
                        aCollector.Remove( item );
                }
            } );

    GENERATORS generators;

    for( EDA_ITEM* item : sel )
    {
        if( PCB_GENERATOR* gen = dynamic_cast<PCB_GENERATOR*>( item ) )
            generators.push_back( gen );
    }

    std::sort( generators.begin(), generators.end(),
               []( const PCB_GENERATOR* a, const PCB_GENERATOR* b ) -> bool
               {
                   return a->GetUpdateOrder() < b->GetUpdateOrder();
               } );

    for( PCB_GENERATOR* gen : generators )
    {
        gen->EditStart( this, board(), frame(), commit );
        gen->Update( this, board(), frame(), commit );
        gen->EditPush( this, board(), frame(), commit );
    }

    return 0;
}


int GENERATOR_TOOL::RegenerateItem( const TOOL_EVENT& aEvent )
{
    BOARD_COMMIT  localCommit( this );
    BOARD_COMMIT* commit = dynamic_cast<BOARD_COMMIT*>( aEvent.Commit() );

    if( !commit )
        commit = &localCommit;

    PCB_GENERATOR* gen = aEvent.Parameter<PCB_GENERATOR*>();

    gen->EditStart( this, board(), frame(), commit );
    gen->Update( this, board(), frame(), commit );
    gen->EditPush( this, board(), frame(), commit );

    return 0;
}


int GENERATOR_TOOL::GenEditAction( const TOOL_EVENT& aEvent )
{
    BOARD_COMMIT* commit = dynamic_cast<BOARD_COMMIT*>( aEvent.Commit() );

    wxCHECK( commit, 0 );

    PCB_GENERATOR* gen = aEvent.Parameter<PCB_GENERATOR*>();

    if( aEvent.IsAction( &PCB_ACTIONS::genStartEdit ) )
    {
        gen->EditStart( this, board(), frame(), commit );
    }
    else if( aEvent.IsAction( &PCB_ACTIONS::genUpdateEdit ) )
    {
        gen->Update( this, board(), frame(), commit );
    }
    else if( aEvent.IsAction( &PCB_ACTIONS::genPushEdit ) )
    {
        gen->EditPush( this, board(), frame(), commit );

        wxASSERT( commit->Empty() );
    }
    else if( aEvent.IsAction( &PCB_ACTIONS::genRevertEdit ) )
    {
        gen->EditRevert( this, board(), frame(), commit );

        wxASSERT( commit->Empty() );
    }
    else if( aEvent.IsAction( &PCB_ACTIONS::genRemove ) )
    {
        gen->Remove( this, board(), frame(), commit );
    }

    return 0;
}


void GENERATOR_TOOL::setTransitions()
{
    // Generator actions
    Go( &GENERATOR_TOOL::ShowGeneratorsManager, PCB_ACTIONS::generatorsShowManager.MakeEvent() );
    Go( &GENERATOR_TOOL::ShowGeneratorProperties, PCB_ACTIONS::generatorProperties.MakeEvent() );

    Go( &GENERATOR_TOOL::RegenerateAll, PCB_ACTIONS::regenerateAll.MakeEvent() );
    Go( &GENERATOR_TOOL::RegenerateOutdated, PCB_ACTIONS::regenerateOutdated.MakeEvent() );
    Go( &GENERATOR_TOOL::RegenerateSelected, PCB_ACTIONS::regenerateSelected.MakeEvent() );
    Go( &GENERATOR_TOOL::RegenerateItem, PCB_ACTIONS::regenerateItem.MakeEvent() );

    Go( &GENERATOR_TOOL::GenEditAction, PCB_ACTIONS::genStartEdit.MakeEvent() );
    Go( &GENERATOR_TOOL::GenEditAction, PCB_ACTIONS::genUpdateEdit.MakeEvent() );
    Go( &GENERATOR_TOOL::GenEditAction, PCB_ACTIONS::genPushEdit.MakeEvent() );
    Go( &GENERATOR_TOOL::GenEditAction, PCB_ACTIONS::genRevertEdit.MakeEvent() );
    Go( &GENERATOR_TOOL::GenEditAction, PCB_ACTIONS::genRemove.MakeEvent() );
}