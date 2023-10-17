/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 1992-2022 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef __DIALOG_IMPORT_GFX_PCB_H__
#define __DIALOG_IMPORT_GFX_PCB_H__

#include <widgets/unit_binder.h>
#include <pcb_edit_frame.h>
#include <pcbnew_settings.h>
#include "dialog_import_gfx_pcb_base.h"
#include <import_gfx/graphics_importer_pcbnew.h>

class GRAPHICS_IMPORT_MGR;


class DIALOG_IMPORT_GFX_PCB : public DIALOG_IMPORT_GFX_PCB_BASE
{
public:
    DIALOG_IMPORT_GFX_PCB( PCB_BASE_FRAME* aParent, bool aUseModuleItems = false );
    ~DIALOG_IMPORT_GFX_PCB();

    /**
     * @return a list of items imported from a vector graphics file.
     */
    std::list<std::unique_ptr<EDA_ITEM>>& GetImportedItems()
    {
        return m_importer->GetItems();
    }

    /**
     * @return true if the placement is interactive, i.e. all imported
     * items must be moved by the mouse cursor to the final position
     * false means the imported items are placed to the final position after import.
     */
    bool IsPlacementInteractive() { return s_placementInteractive; }

    /**
     * @return true if the items should be added into a group when being placed.
     */
    bool ShouldGroupItems() { return s_shouldGroupItems; }

    /**
     * @return true if discontinuities in the shapes should be fixed.
     */
    bool ShouldFixDiscontinuities() { return s_fixDiscontinuities; }

    /**
     * @return tolerance to connect the shapes.
     */
    int GetTolerance() { return s_toleranceValue; }

    bool TransferDataFromWindow() override;

private:
    // Virtual event handlers
    void onBrowseFiles( wxCommandEvent& event ) override;
    void onFilename( wxCommandEvent& event );
    void originOptionOnUpdateUI( wxUpdateUIEvent& event ) override;

	void onInteractivePlacement( wxCommandEvent& event ) override
    {
        s_placementInteractive = true;
    }

	void onAbsolutePlacement( wxCommandEvent& event ) override
    {
        s_placementInteractive = false;
    }

    void onGroupItems( wxCommandEvent& event ) override
    {
        s_shouldGroupItems = m_groupItems->GetValue();
    }

    void onFixDiscontinuities( wxCommandEvent& event ) override
    {
        s_fixDiscontinuities = m_rbFixDiscontinuities->GetValue();
        m_tolerance.Enable( s_fixDiscontinuities );
    }

private:
    PCB_BASE_FRAME*                           m_parent;
    std::unique_ptr<GRAPHICS_IMPORTER_PCBNEW> m_importer;
    std::unique_ptr<GRAPHICS_IMPORT_MGR>      m_gfxImportMgr;

    UNIT_BINDER          m_xOrigin;
    UNIT_BINDER          m_yOrigin;
    UNIT_BINDER          m_defaultLineWidth;
    UNIT_BINDER          m_tolerance;

    static bool          s_shouldGroupItems;
    static bool          s_placementInteractive;
    static bool          s_fixDiscontinuities;
    static int           s_toleranceValue;
    static double        s_importScale;         // a scale factor to change the size of imported
                                                // items m_importScale =1.0 means keep original size
};

#endif    //  __DIALOG_IMPORT_GFX_PCB_H__
