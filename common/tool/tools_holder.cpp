/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2020 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <pgm_base.h>
#include <settings/common_settings.h>
#include <tool/action_manager.h>
#include <tool/action_menu.h>
#include <tool/actions.h>
#include <tool/tools_holder.h>
#include <tool/tool_manager.h>


TOOLS_HOLDER::TOOLS_HOLDER() :
        m_toolManager( nullptr ),
        m_actions( nullptr ),
        m_toolDispatcher( nullptr ),
        m_immediateActions( true ),
        m_dragAction( MOUSE_DRAG_ACTION::SELECT ),
        m_moveWarpsCursor( true )
{ }


// TODO: Implement an RAII mechanism for the stack PushTool/PopTool pairs
void TOOLS_HOLDER::PushTool( const std::string& actionName )
{
    m_toolStack.push_back( actionName );

    // Human cognitive stacking is very shallow; deeper tool stacks just get annoying
    if( m_toolStack.size() > 3 )
        m_toolStack.erase( m_toolStack.begin() );

    TOOL_ACTION* action = m_toolManager->GetActionManager()->FindAction( actionName );

    if( action )
        DisplayToolMsg( action->GetLabel() );
    else
        DisplayToolMsg( actionName );
}


void TOOLS_HOLDER::PopTool( const std::string& actionName )
{
    // Push/pop events can get out of order (such as when they're generated by the Simulator
    // frame but not processed until the mouse is back in the Schematic frame), so make sure
    // we're popping the right stack frame.

    for( int i = (int) m_toolStack.size() - 1; i >= 0; --i )
    {
        if( m_toolStack[ i ] == actionName )
        {
            m_toolStack.erase( m_toolStack.begin() + i );

            // If there's something underneath us, and it's now the top of the stack, then
            // re-activate it
            if( ( --i ) >= 0 && i == (int)m_toolStack.size() - 1 )
            {
                std::string  back = m_toolStack[ i ];
                TOOL_ACTION* action = m_toolManager->GetActionManager()->FindAction( back );

                if( action )
                {
                    // Pop the action as running it will push it back onto the stack
                    m_toolStack.pop_back();

                    TOOL_EVENT evt = action->MakeEvent();
                    evt.SetHasPosition( false );
                    GetToolManager()->PostEvent( evt );
                }
            }
            else
                DisplayToolMsg( ACTIONS::selectionTool.GetLabel() );

            return;
        }
    }
}


std::string TOOLS_HOLDER::CurrentToolName() const
{
    if( m_toolStack.empty() )
        return ACTIONS::selectionTool.GetName();
    else
        return m_toolStack.back();
}


bool TOOLS_HOLDER::IsCurrentTool( const TOOL_ACTION& aAction ) const
{
    if( m_toolStack.empty() )
        return &aAction == &ACTIONS::selectionTool;
    else
        return m_toolStack.back() == aAction.GetName();
}


void TOOLS_HOLDER::CommonSettingsChanged( bool aEnvVarsChanged, bool aTextVarsChanged )
{
    if( GetToolManager() )
        GetToolManager()->GetActionManager()->UpdateHotKeys( false );

    COMMON_SETTINGS* settings = Pgm().GetCommonSettings();

    m_moveWarpsCursor = settings->m_Input.warp_mouse_on_move;
    m_dragAction = settings->m_Input.drag_left;
    m_immediateActions = settings->m_Input.immediate_actions;
}

