/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 1992-2021 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef FP_TEXT_H
#define FP_TEXT_H

#include <eda_text.h>
#include <board_item.h>

class LINE_READER;
class EDA_RECT;
class FOOTPRINT;
class MSG_PANEL_ITEM;
class PCB_BASE_FRAME;
class SHAPE;


class FP_TEXT : public BOARD_ITEM, public EDA_TEXT
{
public:
    /**
     * Footprint text type: there must be only one (and only one) for each of the reference
     * value texts in one footprint; others could be added for the user (DIVERS is French for
     * 'others'). Reference and value always live on silkscreen (on the footprint side); other
     * texts are planned to go on whatever layer the user wants.
     */
    enum TEXT_TYPE
    {
        TEXT_is_REFERENCE = 0,
        TEXT_is_VALUE     = 1,
        TEXT_is_DIVERS    = 2
    };

    FP_TEXT( FOOTPRINT* aParentFootprint, TEXT_TYPE text_type = TEXT_is_DIVERS );

    // Do not create a copy constructor & operator=.
    // The ones generated by the compiler are adequate.

    ~FP_TEXT();

    static inline bool ClassOf( const EDA_ITEM* aItem )
    {
        return aItem && aItem->Type() == PCB_FP_TEXT_T;
    }

    bool IsType( const KICAD_T aScanTypes[] ) const override
    {
        if( BOARD_ITEM::IsType( aScanTypes ) )
            return true;

        for( const KICAD_T* p = aScanTypes; *p != EOT; ++p )
        {
            if( *p == PCB_LOCATE_TEXT_T )
                return true;
        }

        return false;
    }

    wxString GetParentAsString() const { return m_parent->m_Uuid.AsString(); }

    bool Matches( const wxFindReplaceData& aSearchData, void* aAuxData ) const override
    {
        return BOARD_ITEM::Matches( GetShownText(), aSearchData );
    }

    virtual wxPoint GetPosition() const override
    {
        return EDA_TEXT::GetTextPos();
    }

    virtual void SetPosition( const wxPoint& aPos ) override
    {
        EDA_TEXT::SetTextPos( aPos );
        SetLocalCoord();
    }

    void SetTextAngle( double aAngle ) override;

    /**
     * Called when rotating the parent footprint.
     */
    void KeepUpright( double aOldOrientation, double aNewOrientation );

    /**
     * @return force the text rotation to be always between -90 .. 90 deg. Otherwise the text
     *         is not easy to read if false, the text rotation is free.
     */
    bool IsKeepUpright() const
    {
        return m_keepUpright;
    }

    void SetKeepUpright( bool aKeepUpright )
    {
        m_keepUpright = aKeepUpright;
    }

    /// Rotate text, in footprint editor
    /// (for instance in footprint rotation transform)
    void Rotate( const wxPoint& aOffset, double aAngle ) override;

    /// Flip entity during footprint flip
    void Flip( const wxPoint& aCentre, bool aFlipLeftRight ) override;

    bool IsParentFlipped() const;

    /// Mirror text position in footprint editing
    /// the text itself is not mirrored, and the layer not modified,
    /// only position is mirrored.
    /// (use Flip to change layer to its paired and mirror the text in fp editor).
    void Mirror( const wxPoint& aCentre, bool aMirrorAroundXAxis );

    /// move text in move transform, in footprint editor
    void Move( const wxPoint& aMoveVector ) override;

    /// @deprecated it seems (but the type is used to 'protect'
    //  reference and value from deletion, and for identification)
    void SetType( TEXT_TYPE aType )     { m_Type = aType; }
    TEXT_TYPE GetType() const           { return m_Type; }

    /**
     * Set the text effects from another instance.
     */
    void SetEffects( const FP_TEXT& aSrc )
    {
        EDA_TEXT::SetEffects( aSrc );
        SetLocalCoord();
        // SetType( aSrc.GetType() );
    }

    /**
     * Swap the text effects of the two involved instances.
     */
    void SwapEffects( FP_TEXT& aTradingPartner )
    {
        EDA_TEXT::SwapEffects( aTradingPartner );
        SetLocalCoord();
        aTradingPartner.SetLocalCoord();
        // std::swap( m_Type, aTradingPartner.m_Type );
    }

    // The Pos0 accessors are for footprint-relative coordinates.
    void SetPos0( const wxPoint& aPos ) { m_Pos0 = aPos; SetDrawCoord(); }
    const wxPoint& GetPos0() const      { return m_Pos0; }

    int GetLength() const;        // text length

    /**
     * @return the text rotation for drawings and plotting the footprint rotation is taken
     *         in account.
     */
    virtual double GetDrawRotation() const override;
    double GetDrawRotationRadians() const { return GetDrawRotation() * M_PI/1800; }

    // Virtual function
    const EDA_RECT GetBoundingBox() const override;

    ///< Set absolute coordinates.
    void SetDrawCoord();

    ///< Set relative coordinates.
    void SetLocalCoord();

    void GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList ) override;

    bool TextHitTest( const wxPoint& aPoint, int aAccuracy = 0 ) const override;
    bool TextHitTest( const EDA_RECT& aRect, bool aContains, int aAccuracy = 0 ) const override;

    bool HitTest( const wxPoint& aPosition, int aAccuracy ) const override
    {
        return TextHitTest( aPosition, aAccuracy );
    }

    bool HitTest( const EDA_RECT& aRect, bool aContained, int aAccuracy = 0 ) const override
    {
        return TextHitTest( aRect, aContained, aAccuracy );
    }

    void TransformShapeWithClearanceToPolygon( SHAPE_POLY_SET& aCornerBuffer, PCB_LAYER_ID aLayer,
                                               int aClearance, int aError, ERROR_LOC aErrorLoc,
                                               bool aIgnoreLineWidth ) const override;

    void TransformTextShapeWithClearanceToPolygon( SHAPE_POLY_SET& aCornerBuffer,
                                                   PCB_LAYER_ID aLayer, int aClearanceValue,
                                                   int aError, ERROR_LOC aErrorLoc ) const;

    // @copydoc BOARD_ITEM::GetEffectiveShape
    virtual std::shared_ptr<SHAPE> GetEffectiveShape( PCB_LAYER_ID aLayer = UNDEFINED_LAYER,
            FLASHING aFlash = FLASHING::DEFAULT ) const override;

    wxString GetClass() const override
    {
        return wxT( "MTEXT" );
    }

    wxString GetSelectMenuText( EDA_UNITS aUnits ) const override;

    BITMAPS GetMenuImage() const override;

    EDA_ITEM* Clone() const override;

    virtual wxString GetShownText( int aDepth = 0 ) const override;

    virtual const BOX2I ViewBBox() const override;

    virtual void ViewGetLayers( int aLayers[], int& aCount ) const override;

    double ViewGetLOD( int aLayer, KIGFX::VIEW* aView ) const override;

#if defined(DEBUG)
    virtual void Show( int nestLevel, std::ostream& os ) const override { ShowDummy( os ); }
#endif

private:
    /* Note: orientation in 1/10 deg relative to the footprint
     * Physical orient is m_Orient + m_Parent->m_Orient
     */

    TEXT_TYPE m_Type;       ///< 0=ref, 1=val, etc.

    wxPoint   m_Pos0;       ///< text coordinates relative to the footprint anchor, orient 0.
                            ///< text coordinate ref point is the text center

    bool      m_keepUpright;    ///< if true, keep rotation angle between -90 .. 90 deg.
                                ///< to keep the text more easy to read

};

#endif // FP_TEXT_H
