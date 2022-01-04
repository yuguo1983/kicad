/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2021-2022 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <widgets/font_choice.h>
#include <wx/fontenum.h>
#include <font/fontconfig.h>


FONT_CHOICE::FONT_CHOICE( wxWindow* aParent, int aId, wxPoint aPosition, wxSize aSize,
                          int nChoices, wxString* aChoices, int aStyle ) :
        wxChoice( aParent, aId, aPosition, aSize, nChoices, aChoices, aStyle )
{
    m_systemFontCount = wxChoice::GetCount();

    std::vector<std::string> fontNames;
    Fontconfig().ListFonts( fontNames );

    wxArrayString menuList;

    for( const std::string& name : fontNames )
        menuList.Add( wxString( name ) );

    menuList.Sort();
    Append( menuList );

    m_notFound = wxS( " " ) + _( "<not found>" );
}


FONT_CHOICE::~FONT_CHOICE()
{
}


void FONT_CHOICE::SetFontSelection( KIFONT::FONT* aFont )
{
    if( !aFont )
    {
        SetSelection( 0 );
    }
    else
    {
        SetStringSelection( aFont->Name() );

        if( GetSelection() == wxNOT_FOUND )
        {
            Append( aFont->Name() + m_notFound );
            SetSelection( GetCount() );
        }
    }

    SendSelectionChangedEvent( wxEVT_CHOICE );
}


bool FONT_CHOICE::HaveFontSelection() const
{
    int sel = GetSelection();

    if( sel < 0 )
        return false;

    if( GetString( sel ).EndsWith( m_notFound ) )
        return false;

    return true;
}


KIFONT::FONT* FONT_CHOICE::GetFontSelection( bool aBold, bool aItalic ) const
{
    if( GetSelection() <= 0 )
        return nullptr;
    else if( GetSelection() == 1 && m_systemFontCount == 2 )
        return KIFONT::FONT::GetFont( "KiCad", aBold, aItalic );
    else
        return KIFONT::FONT::GetFont( GetStringSelection(), aBold, aItalic );
}

