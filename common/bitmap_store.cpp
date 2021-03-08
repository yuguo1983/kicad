/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2021 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <wx/bitmap.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/mstream.h>
#include <wx/stdpaths.h>

#include <asset_archive.h>
#include <bitmaps.h>
#include <bitmap_store.h>
#include <bitmaps/bitmap_info.h>
#include <kiplatform/ui.h>
#include <paths.h>


/// A question-mark icon shown when we can't find a given bitmap in the archive
static const unsigned char s_imageNotFound[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x08, 0x04, 0x00, 0x00, 0x00, 0x4a, 0x7e, 0xf5,
    0x73, 0x00, 0x00, 0x01, 0xab, 0x49, 0x44, 0x41, 0x54, 0x38, 0xcb, 0x8d, 0xd4, 0x3f, 0x2c, 0x03,
    0x51, 0x18, 0x00, 0xf0, 0x4b, 0x0c, 0x22, 0xb1, 0x19, 0x8c, 0x46, 0x09, 0x62, 0x60, 0x68, 0x62,
    0x20, 0xb1, 0x98, 0x24, 0x24, 0x77, 0x08, 0xd2, 0x10, 0x03, 0x06, 0x8b, 0x90, 0x98, 0x48, 0x25,
    0x3a, 0x34, 0x12, 0xb1, 0x19, 0x0c, 0xa2, 0x34, 0xfe, 0xc4, 0x24, 0x29, 0x93, 0xa1, 0x0c, 0x52,
    0x7f, 0xe2, 0xcf, 0x20, 0x5a, 0x83, 0xa1, 0xb4, 0xf7, 0xa7, 0x77, 0xef, 0xde, 0xdd, 0xd3, 0xde,
    0xd1, 0x7e, 0x7a, 0x17, 0xda, 0xe2, 0xb5, 0xee, 0x7d, 0xdb, 0x7d, 0xdf, 0xef, 0xde, 0xbb, 0xfb,
    0xde, 0x7b, 0x0c, 0x0b, 0x6c, 0xfa, 0x77, 0x70, 0x46, 0x9f, 0x69, 0x05, 0x67, 0xfe, 0xc9, 0x01,
    0xc3, 0xa6, 0x99, 0xa2, 0xa1, 0xb5, 0x8b, 0x81, 0xa4, 0x9a, 0x20, 0xf7, 0xca, 0x39, 0x0a, 0x2b,
    0x11, 0x84, 0x0c, 0xe1, 0x20, 0x55, 0x57, 0xc8, 0xe7, 0xaa, 0x0b, 0x80, 0xb4, 0xf0, 0x77, 0xcf,
    0xda, 0x7a, 0x66, 0x02, 0xd8, 0x7c, 0xb8, 0x61, 0xf3, 0x3d, 0x89, 0x71, 0x1b, 0x15, 0x68, 0x4d,
    0xb2, 0x31, 0x54, 0x54, 0xfc, 0x1d, 0x0b, 0x20, 0x21, 0x5c, 0x43, 0x01, 0x0c, 0x23, 0x04, 0xd7,
    0x3f, 0x58, 0x98, 0x85, 0x7d, 0xf3, 0x46, 0x89, 0xa2, 0x4b, 0xe4, 0xfb, 0x22, 0xc1, 0x94, 0x32,
    0x4f, 0x05, 0x7a, 0xb3, 0xf0, 0xf6, 0xa8, 0x4a, 0xbc, 0xb2, 0xa8, 0x77, 0x91, 0x56, 0xdc, 0x2b,
    0x3e, 0x2f, 0x67, 0x2d, 0xe0, 0x81, 0x78, 0x98, 0x0a, 0x18, 0x46, 0xf1, 0xe0, 0x1e, 0xa8, 0xc8,
    0xbf, 0x60, 0xf8, 0x04, 0x5b, 0x60, 0x06, 0x12, 0xd1, 0x12, 0xe0, 0xe7, 0x10, 0xd7, 0x02, 0xa6,
    0x05, 0xbc, 0x10, 0x0f, 0x39, 0x00, 0xc9, 0xf9, 0x17, 0x32, 0x62, 0x7f, 0xc3, 0x99, 0xae, 0x4d,
    0xfd, 0x03, 0xa0, 0x52, 0xdc, 0x79, 0xd0, 0xc7, 0xec, 0xf2, 0xd5, 0x8c, 0x14, 0x83, 0xaa, 0xb2,
    0x40, 0xa8, 0x16, 0xaf, 0x4f, 0x89, 0xf5, 0x8b, 0x39, 0xf0, 0x9b, 0xa2, 0x80, 0xeb, 0xa9, 0x7d,
    0x28, 0x02, 0x87, 0xc7, 0x29, 0x2e, 0x57, 0x3e, 0x08, 0x21, 0x22, 0xde, 0xea, 0xb5, 0x25, 0x3a,
    0x9d, 0x6f, 0x60, 0x43, 0x9c, 0x0c, 0xd8, 0xe5, 0x8f, 0xba, 0xb4, 0x0d, 0x95, 0xc5, 0x39, 0x3a,
    0x18, 0x3f, 0x22, 0xd6, 0xda, 0x77, 0x0d, 0x69, 0xeb, 0x77, 0x8e, 0x0a, 0xd4, 0x85, 0xbd, 0xac,
    0x3b, 0xb7, 0x8b, 0x2e, 0x54, 0xdc, 0xed, 0x08, 0xe0, 0x39, 0xf9, 0xcd, 0x0a, 0x45, 0xd7, 0x3a,
    0x1d, 0x81, 0x72, 0xa3, 0x04, 0x50, 0x5d, 0xc8, 0x2b, 0xfb, 0xb4, 0x0e, 0x87, 0x40, 0x19, 0x7d,
    0xd5, 0xfd, 0x99, 0x8d, 0x6c, 0x4c, 0x4b, 0x4e, 0x3b, 0x02, 0x7c, 0x6c, 0xc6, 0xee, 0xf0, 0x64,
    0xee, 0x1c, 0x38, 0x02, 0x32, 0x71, 0xdb, 0xa0, 0x1f, 0xd4, 0x8f, 0xc2, 0xce, 0x2d, 0x37, 0xc3,
    0xd5, 0x8a, 0x7d, 0x0a, 0x96, 0x80, 0x7f, 0x72, 0x34, 0x83, 0xd6, 0x28, 0xf1, 0x4f, 0x28, 0x82,
    0x24, 0x59, 0x75, 0xd1, 0x00, 0xe5, 0x9a, 0xb1, 0x2e, 0x1a, 0xce, 0xa0, 0x3d, 0x67, 0xe1, 0x13,
    0xb5, 0xc5, 0x7e, 0xb7, 0x69, 0x08, 0x53, 0x5c, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
    0xae, 0x42, 0x60, 0x82,
};


/// Icon used for EDA_ITEMs that don't have a custom icon configured
/// @todo Replace this with an external file?
static const unsigned char s_dummyItem[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff,
    0x61, 0x00, 0x00, 0x00, 0x5f, 0x49, 0x44, 0x41, 0x54, 0x38, 0xcb, 0x63, 0xf8, 0xff, 0xff, 0x3f,
    0x03, 0x25, 0x98, 0x61, 0x68, 0x1a, 0x00, 0x04, 0x46, 0x40, 0xfc, 0x02, 0x88, 0x45, 0x41, 0x1c,
    0x76, 0x20, 0xfe, 0x01, 0xc4, 0xbe, 0x24, 0x18, 0x60, 0x01, 0xc4, 0x20, 0x86, 0x04, 0x88, 0xc3,
    0x01, 0xe5, 0x04, 0x0c, 0xb8, 0x01, 0x37, 0x81, 0xf8, 0x04, 0x91, 0xf8, 0x0a, 0x54, 0x8f, 0x06,
    0xb2, 0x01, 0x9b, 0x81, 0x78, 0x02, 0x91, 0x78, 0x05, 0x54, 0x8f, 0xca, 0xe0, 0x08, 0x03, 0x36,
    0xa8, 0xbf, 0xec, 0xc8, 0x32, 0x80, 0xcc, 0x84, 0x04, 0x0a, 0xbc, 0x1d, 0x40, 0x2c, 0xc8, 0x30,
    0xf4, 0x33, 0x13, 0x00, 0x6b, 0x1a, 0x46, 0x7b, 0x68, 0xe7, 0x0f, 0x0b, 0x00, 0x00, 0x00, 0x00,
    0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};


static const wxString traceBitmaps = wxT( "KICAD_BITMAPS" );

static const wxString IMAGE_ARCHIVE = wxT( "images.tar.gz" );


size_t std::hash<std::pair<BITMAPS, int>>::operator()( const std::pair<BITMAPS, int>& aPair ) const
{
    return std::hash<int>()( static_cast<int>( aPair.first ) ^ std::hash<int>()( aPair.second ) );
}


BITMAP_STORE::BITMAP_STORE()
{
    wxFileName path( PATHS::GetStockDataPath() + wxT( "/resources" ), IMAGE_ARCHIVE );

    wxLogTrace( traceBitmaps, "Loading bitmaps from " + path.GetFullPath() );

    m_archive = std::make_unique<ASSET_ARCHIVE>( path.GetFullPath() );

    m_theme = KIPLATFORM::UI::IsDarkTheme() ? wxT( "dark" ) : wxT( "light" );

    buildBitmapInfoCache();
}


wxBitmap BITMAP_STORE::GetBitmap( BITMAPS aBitmapId, int aHeight )
{
    return wxBitmap( getImage( aBitmapId, aHeight ) );
}


wxBitmap BITMAP_STORE::GetBitmapScaled( BITMAPS aBitmapId, int aScaleFactor, int aHeight )
{
    wxImage image = getImage( aBitmapId, aHeight );

    // Bilinear seems to genuinely look better for these line-drawing icons
    // than bicubic, despite claims in the wx documentation that bicubic is
    // "highest quality". I don't recommend changing this. Bicubic looks
    // blurry and makes me want an eye exam.
    image.Rescale( aScaleFactor * image.GetWidth() / 4, aScaleFactor * image.GetHeight() / 4,
                   wxIMAGE_QUALITY_BILINEAR );

    return wxBitmap( image );
}


wxImage BITMAP_STORE::getImage( BITMAPS aBitmapId, int aHeight )
{
    const unsigned char* data = nullptr;
    long count;

    if( aBitmapId == BITMAPS::dummy_item )
    {
        data  = s_dummyItem;
        count = sizeof( s_dummyItem );
    }
    else
    {
        count = m_archive->GetFilePointer( bitmapName( aBitmapId, aHeight ), &data );

        if( count < 0 )
        {
            data  = s_imageNotFound;
            count = sizeof( s_imageNotFound );
        }
    }

    wxMemoryInputStream is( data, count );
    wxImage image( is, wxBITMAP_TYPE_PNG );
    wxBitmap bitmap( image );

    return image;
}


void BITMAP_STORE::ThemeChanged()
{
    m_bitmapNameCache.clear();
}


const wxString& BITMAP_STORE::bitmapName( BITMAPS aBitmapId, int aHeight )
{
    std::pair<BITMAPS, int> key = std::make_pair( aBitmapId, aHeight );

    if( !m_bitmapNameCache.count( key ) )
        m_bitmapNameCache[key] = computeBitmapName( aBitmapId, aHeight );

    return m_bitmapNameCache.at( key );
}


wxString BITMAP_STORE::computeBitmapName( BITMAPS aBitmapId, int aHeight )
{
    if( !m_bitmapInfoCache.count( aBitmapId ) )
    {
        wxLogTrace( traceBitmaps, "No bitmap info available for %d", aBitmapId );
        return wxEmptyString;
    }

    wxString fn;

    for( const BITMAP_INFO& info : m_bitmapInfoCache.at( aBitmapId ) )
    {
        if( info.theme != m_theme )
            continue;

        if( aHeight < 0 || info.height == aHeight )
        {
            fn = info.filename;
            break;
        }
    }

    if( fn.IsEmpty() )
    {
        wxLogTrace( traceBitmaps, "No bitmap found matching ID %d, height %d, theme %s",
                    aBitmapId, aHeight, m_theme );
        return m_bitmapInfoCache.at( aBitmapId ).begin()->filename;
    }

    return fn;
}


void BITMAP_STORE::buildBitmapInfoCache()
{
    for( const BITMAP_INFO& entry : g_BitmapInfo )
        m_bitmapInfoCache[entry.id].emplace_back( entry );
}
