/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
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

#ifndef SCREEN_H
#define SCREEN_H

#include <memory>
#include <stddef.h>
#include <unordered_set>
#include <vector>
#include <wx/arrstr.h>
#include <wx/chartype.h>
#include <wx/gdicmn.h>
#include <wx/string.h>

#include <base_screen.h>
#include <core/typeinfo.h>
#include <kiid.h>
#include <kiway_holder.h>
#include <layer_ids.h>
#include <marker_base.h>
#include <page_info.h>
#include <template_fieldnames.h>
#include <title_block.h>

#include <lib_id.h>
#include <sch_symbol.h>         // SYMBOL_INSTANCE_REFERENCE
#include <sch_reference_list.h>
#include <sch_rtree.h>
#include <sch_sheet.h>
#include <sch_sheet_path.h>

class BUS_ALIAS;
class EDA_ITEM;
class LIB_SYMBOL;
class LIB_PIN;
class SCH_SYMBOL;
class SCH_LINE;
class SCH_TEXT;
class PLOTTER;
class REPORTER;
class SCH_EDIT_FRAME;
class SCH_SHEET;
class SCH_SHEET_LIST;
class SCH_SEXPR_PARSER;
class SCH_SEXPR_PLUGIN;

enum SCH_LINE_TEST_T
{
    ENTIRE_LENGTH_T,
    END_POINTS_ONLY_T,
    EXCLUDE_END_POINTS_T
};


/// Max number of sheets in a hierarchy project
#define NB_MAX_SHEET    500

struct PICKED_SYMBOL
{
    LIB_ID LibId;
    int    Unit;
    int    Convert;

    std::vector<std::pair<int, wxString>> Fields;

    PICKED_SYMBOL() : Unit( 1 ), Convert( 1 )
    {
    }
};

class SCH_SCREEN : public BASE_SCREEN
{
public:
    SCH_SCREEN( EDA_ITEM* aParent = nullptr );

    ~SCH_SCREEN();

    SCHEMATIC* Schematic() const;

    EE_RTREE& Items() { return m_rtree; }
    const EE_RTREE& Items() const { return m_rtree; }

    bool IsEmpty() const
    {
        return m_rtree.empty();
    }

    bool HasItems( KICAD_T aItemType ) const;

    bool HasSheets() const { return HasItems( SCH_SHEET_T ); }

    static bool ClassOf( const EDA_ITEM* aItem );

    virtual wxString GetClass() const override
    {
        return wxT( "SCH_SCREEN" );
    }

    void SetFileFormatVersionAtLoad( int aVersion ) { m_fileFormatVersionAtLoad = aVersion; }
    int GetFileFormatVersionAtLoad()  const { return m_fileFormatVersionAtLoad; }

    const PAGE_INFO& GetPageSettings() const                { return m_paper; }
    void SetPageSettings( const PAGE_INFO& aPageSettings )  { m_paper = aPageSettings; }

    /**
     * Set the file name for this screen to \a aFileName.
     *
     * @note Screen file names must be absolute or empty.  Absolute file names do not have to
     *       exist yet in the case of a new schematic file but file names cannot be relative.
     *
     * @param aFileName is the absolute file name and path of the screen.
     */
    void SetFileName( const wxString& aFileName );

    const wxString& GetFileName() const                     { return m_fileName; }

    const wxPoint& GetAuxOrigin() const                     { return m_aux_origin; }
    void SetAuxOrigin( const wxPoint& aPosition )           { m_aux_origin = aPosition; }

    const TITLE_BLOCK& GetTitleBlock() const                { return m_titles; }

    void SetTitleBlock( const TITLE_BLOCK& aTitleBlock )    { m_titles = aTitleBlock; }

    void DecRefCount();
    void IncRefCount();
    int GetRefCount() const                                 { return m_refCount; }

    /**
     * Return the number of times this screen is used.
     *
     * In the legacy file formats: if this screen is used only once (not a complex hierarchy)
     * the reference field can be used to store the symbol reference.  If this screen is used
     * more than once (a complex hierarchy), then symbols must have a full alternate reference
     * management via sheet paths.
     *
     * In the new schematic file format, all instance data is stored in the root sheet even
     * for simple hierarchies.
     *
     * @return the sheet paths sharing this screen.
     */
    std::vector<SCH_SHEET_PATH>& GetClientSheetPaths()
    {
        return m_clientSheetPathList;
    }

    void Append( SCH_ITEM* aItem );

    /**
     * Copy the contents of \a aScreen into this #SCH_SCREEN object.
     *
     * @warning The contents of \a Screen cannot contain any duplicate sheet names or any
     *          hierarchy recursion issues or bad things will happen.
     *
     * @param aScreen is the screen to append to this one.
     * @return false if there are any duplicate sheet names or any hierarchy recursion issues the
     *         calling this method or KiCad will crash.
     */
    void Append( SCH_SCREEN* aScreen );

    /**
     * Delete all draw items and clears the project settings.
     */
    void Clear( bool aFree = true );

    /**
     * Free all the items from the schematic associated with the screen.
     *
     * This does not delete any sub hierarchies.
     */
    void FreeDrawList();

    /**
     * Check \a aPosition within a distance of \a aAccuracy for items of type \a aFilter.
     *
     * @param[in] aPosition Position in drawing units.
     * @param aAccuracy The maximum distance within \a Position to check for an item.
     * @param aType The type of item to find.
     * @return The item found that meets the search criteria or NULL if none found.
     */
    SCH_ITEM* GetItem( const wxPoint& aPosition, int aAccuracy = 0,
                       KICAD_T aType = SCH_LOCATE_ANY_T ) const;

    /**
     * Initialize the #LIB_SYMBOL reference for each #SCH_SYMBOL found in this schematic
     * from the project #SYMBOL_LIB_TABLE.
     *
     * Symbol library links are set using the symbol library table and will fall back to
     * the cache only if the cache is loaded.  The cache should only be loaded when opening
     * legacy schematic files.
     *
     * @note This should only be called when the user specifically requests all library symbol
     *       links to be updated or when the legacy schematic is opened for the last time.  All
     *       subsequent schematic loads with the new s-expression will contain the library
     *       symbols and should call #UpdateLocalLibSymbolLinks.
     *
     * @param[in] aReporter Optional #REPORTER object to write status and error messages into.
     */
    void UpdateSymbolLinks( REPORTER* aReporter = nullptr );

    /**
     * Initialize the #LIB_SYMBOL reference for each #SCH_SYMBOL found in this schematic
     * with the local project library symbols.
     */
    void UpdateLocalLibSymbolLinks();

    /**
     * Print all the items in the screen to \a aDC.
     *
     * @note This function is useful only for schematic.  The library editor and library viewer
     *       do not use a draw list and therefore draws nothing.
     */
    void Print( const RENDER_SETTINGS* aSettings );

    /**
     * Plot all the schematic objects to \a aPlotter.
     *
     * @note This function is useful only for schematic. The library editor and library viewer
     *       do not use a draw list and therefore plots nothing.
     *
     * @param[in] aPlotter The plotter object to plot to.
     */
    void Plot( PLOTTER* aPlotter ) const;

    /**
     * Remove \a aItem from the schematic associated with this screen.
     *
     * @note The removed item is not deleted.  It is only unlinked from the item list.
     * @param[in] aItem Item to be removed from schematic.
     * @return True if we successfully removed the item
     */
    bool Remove( SCH_ITEM* aItem );

    /**
     * Update \a aItem's bounding box in the tree
     *
     * @param[in] aItem Item that needs to be updated.
     */
    void Update( SCH_ITEM* aItem );

    /**
     * Removes \a aItem from the linked list and deletes the object.
     *
     * If \a aItem is a schematic sheet label, it is removed from the screen associated with
     * the sheet that contains the label to be deleted.
     *
     * @param[in] aItem The schematic object to be deleted from the screen.
     */
    void DeleteItem( SCH_ITEM* aItem );

    bool CheckIfOnDrawList( const SCH_ITEM* aItem ) const;

    /**
     * Test all of the connectable objects in the schematic for unused connection points.
     *
     * @param aPath is a sheet path to pass to UpdateDanglingState if desired.
     * @param aChangedHandler is an optional callback to make on each changed item.
     */
    void TestDanglingEnds( const SCH_SHEET_PATH* aPath = nullptr,
                           std::function<void( SCH_ITEM* )>* aChangedHandler = nullptr ) const;

    /**
     * Return all wires and junctions connected to \a aSegment which are not connected any
     * symbol pin.
     *
     * @param aSegment The segment to test for connections.
     */
    std::set<SCH_ITEM*> MarkConnections( SCH_LINE* aSegment );

    /**
     * Clear the state flags of all the items in the screen.
     */
    void ClearDrawingState();

    size_t CountConnectedItems( const wxPoint& aPos, bool aTestJunctions ) const;

    /**
     * Test if a junction is required for the items at \a aPosition on the screen.
     *
     * A junction is required at \a aPosition if one of the following criteria is satisfied:
     *  - One wire midpoint and one or more wire endpoints.
     *  - Three or more wire endpoints.
     *  - One wire midpoint and a symbol pin.
     *  - Two or more wire endpoints and a symbol pin.
     *
     * @param[in] aPosition The position to test.
     * @param aNew Checks if a _new_ junction is needed, i.e. there isn't one already
     * @return True if a junction is required at \a aPosition.
     */
    bool IsJunctionNeeded( const wxPoint& aPosition, bool aNew = false ) const;

    /**
     * Test if \a aPosition is a connection point on \a aLayer.
     *
     * @param[in] aPosition Position to test.
     * @param aLayer The layer type to test against.  Valid layer types are #LAYER_NOTES,
     *               #LAYER_BUS, and #LAYER_WIRE.
     * @return True if \a Position is a connection point on \a aLayer.
     */
    bool IsTerminalPoint( const wxPoint& aPosition, int aLayer ) const;

    /**
     * Test the screen for a symbol pin item at \a aPosition.
     *
     * @param[in] aPosition Position to test.
     * @param[out] aSymbol The symbol if a pin was found, otherwise NULL.
     * @param aEndPointOnly Set to true to test if \a aPosition is the connection
     *                      point of the pin.
     * @return The pin item if found, otherwise NULL.
     */
    LIB_PIN* GetPin( const wxPoint& aPosition, SCH_SYMBOL** aSymbol = nullptr,
                     bool aEndPointOnly = false ) const;

    /**
     * Test the screen if \a aPosition is a sheet label object.
     *
     * @param[in] aPosition The position to test.
     * @return The sheet label object if found otherwise NULL.
     */
    SCH_SHEET_PIN* GetSheetPin( const wxPoint& aPosition ) const;

    /**
     * Clear the annotation for the symbols in \a aSheetPath on the screen.
     *
     * @param[in] aSheetPath The sheet path of the symbol annotation to clear.  If NULL then
     *                       the entire hierarchy is cleared.
     */
    void ClearAnnotation( SCH_SHEET_PATH* aSheetPath );

    /**
     * For screens shared by many sheetpaths (complex hierarchies):
     * to be able to clear or modify any reference related  sharing this screen
     * (i.e. the list of symbols), an entry for each screen path must exist.
     * This function creates missing entries, using as default reference the current
     * reference field and unit number
     * Note: m_clientSheetPathList must be up to date
     * ( built by SCH_SCREENS::BuildClientSheetPathList() )
     */
    void EnsureAlternateReferencesExist();

    /**
     * Add all schematic sheet and symbol objects in the screen to \a aItems.
     *
     * @param[out] aItems Hierarchical item list to fill.
     */
    void GetHierarchicalItems( std::vector<SCH_ITEM*>* aItems ) const;

    /**
     * Similar to Items().OfType( SCH_SHEET_T ), but return the sheets in a
     * deterministic order (L-R, T-B) for sheet numbering.
     */
    void GetSheets( std::vector<SCH_ITEM*>* aItems ) const;

    /**
     * Return a line item located at \a aPosition.
     *
     * @param[in] aPosition The wxPoint to test for a line item.
     * @param aAccuracy Amount to inflate the item hit test bounding box.
     * @param aLayer The layer the line is drawn upon.
     * @param aSearchType Additional line test criteria.
     * @return The SCH_LINE* of the wire item found at \a aPosition or NULL if item not
     *         found.
     */
    SCH_LINE* GetLine( const wxPoint& aPosition, int aAccuracy = 0, int aLayer = LAYER_NOTES,
                       SCH_LINE_TEST_T aSearchType = ENTIRE_LENGTH_T ) const;

    SCH_LINE* GetWire( const wxPoint& aPosition, int aAccuracy = 0,
                       SCH_LINE_TEST_T aSearchType = ENTIRE_LENGTH_T ) const
    {
        return GetLine( aPosition, aAccuracy, LAYER_WIRE, aSearchType );
    }

    SCH_LINE* GetBus( const wxPoint& aPosition, int aAccuracy = 0,
                      SCH_LINE_TEST_T aSearchType = ENTIRE_LENGTH_T ) const
    {
        return GetLine( aPosition, aAccuracy, LAYER_BUS, aSearchType );
    }

    /**
     * Return a label item located at \a aPosition.
     *
     * @param[in] aPosition The wxPoint to test for label items.
     * @param aAccuracy Amount to inflate the item hit test bounding box.
     * @return The SCH_TEXT* of the label item found at \a aPosition or NULL if item not
     *         found.
     */
    SCH_TEXT* GetLabel( const wxPoint& aPosition, int aAccuracy = 0 ) const;

    /**
     * Fetch a list of unique #LIB_SYMBOL object pointers required to properly render each
     * #SCH_SYMBOL in this schematic.
     *
     * @return The list of unique #LIB_SYMBOL object pointers.
     */
    std::map<wxString, LIB_SYMBOL*>& GetLibSymbols() { return m_libSymbols; }
    const std::map<wxString, LIB_SYMBOL*>& GetLibSymbols() const { return m_libSymbols; }

    /**
     * Add \a aLibSymbol to the library symbol map.
     *
     * The symbol is mapped to the result of #LIB_ID::Format().  If a symbol is already
     * mapped, the existing symbol is replaced with \a aLibSymbol.  The screen object takes
     * ownership of the pointer.
     *
     * @param aLibSymbol A pointer the #LIB_SYMBOL to be added to the symbol map.
     */
    void AddLibSymbol( LIB_SYMBOL* aLibSymbol );

    /**
     * Add a bus alias definition (and transfers ownership of the pointer).
     */
    void AddBusAlias( std::shared_ptr<BUS_ALIAS> aAlias );

    /**
     * Remove all bus alias definitions.
     */
    void ClearBusAliases()
    {
        m_aliases.clear();
    }

    /**
     * Return a list of bus aliases defined in this screen
     */
    std::unordered_set< std::shared_ptr<BUS_ALIAS> > GetBusAliases() const
    {
        return m_aliases;
    }

    const std::vector<SYMBOL_INSTANCE_REFERENCE>& GetSymbolInstances() const
    {
        return m_symbolInstances;
    }

    const std::vector<SCH_SHEET_INSTANCE>& GetSheetInstances() const
    {
        return m_sheetInstances;
    }

    const KIID& GetUuid() const { return m_uuid; }

    void AssignNewUuid() { m_uuid = KIID(); }

#if defined(DEBUG)
    void Show( int nestLevel, std::ostream& os ) const override;
#endif

    /**
     * last value for the zoom level, useful in Eeschema when changing the current displayed
     * sheet to reuse the same zoom level when back to the sheet using this screen
     */
    double m_LastZoomLevel;

private:
    friend SCH_EDIT_FRAME;     // Only to populate m_symbolInstances.
    friend SCH_SEXPR_PARSER;   // Only to load instance information from schematic file.
    friend SCH_SEXPR_PLUGIN;   // Only to save the loaded instance information to schematic file.

    void clearLibSymbols();

    wxString    m_fileName;                 // File used to load the screen.
    int         m_fileFormatVersionAtLoad;
    int         m_refCount;                 // Number of sheets referencing this screen.
                                            // Delete when it goes to zero.
    /**
     * The list of sheet paths sharing this screen.  Used in some annotation calculations to
     * update alternate references.
     *
     * Note: a screen having a m_refCount = 1 (only one sheet path using it) can have many
     * sheet paths sharing this screen if it is inside another sheet having many instances.
     */
    std::vector<SCH_SHEET_PATH> m_clientSheetPathList;


    PAGE_INFO   m_paper;                    // The size of the paper to print or plot on.
    TITLE_BLOCK m_titles;
    wxPoint     m_aux_origin;               // Origin used for drill & place files by Pcbnew.
    EE_RTREE    m_rtree;

    int         m_modification_sync;        // Inequality with SYMBOL_LIBS::GetModificationHash()
                                            // will trigger ResolveAll().

    bool        m_zoomInitialized;          // Set to true once the zoom value is initialized with
                                            // `InitZoom()`.

    /// List of bus aliases stored in this screen.
    std::unordered_set< std::shared_ptr< BUS_ALIAS > > m_aliases;

    /// Library symbols required for this schematic.
    std::map<wxString, LIB_SYMBOL*> m_libSymbols;

    /**
     * The list of symbol instances loaded from the schematic file.
     *
     * This list is only used to as temporary storage when the schematic file is loaded.
     * If the screen is the root sheet, then this information is used to update the
     *  #SCH_SYMBOL instance reference and unit information after the entire schematic
     * is loaded and is never used again.  If this screen is not the root sheet, then the
     * schematic file is the root sheet of another project and this information is saved
     * unchanged back to the schematic file.
     *
     * @warning Under no circumstances is this information to be modified or used after the
     *          schematic file is loaded.  It is read only and it is only written to non-root
     *          schematic files.
     */
    std::vector<SYMBOL_INSTANCE_REFERENCE> m_symbolInstances;
    std::vector<SCH_SHEET_INSTANCE> m_sheetInstances;

    /**
     * A unique identifier for each schematic file.
     *
     * As of right now, this only has meaning for the root schematic.  In the future, it may
     * be useful to detect unexpected hierarchy changes.
     */
    KIID m_uuid;
};


/**
 * Container class that holds multiple #SCH_SCREEN objects in a hierarchy.
 *
 * Individual #SCH_SCREEN objects are unique and correspond to .sch files.
 *
 * NOTE: It may be desirable to fold the functionality of SCH_SCREENS into
 * the new SCHEMATIC class at some point, since SCHEMATIC can also be thought
 * of as owning the collection of all the SCH_SCREEN objects.
 */
class SCH_SCREENS
{
public:
    SCH_SCREENS( SCH_SHEET* aSheet );
    SCH_SCREENS( SCH_SHEET& aSheet ) : SCH_SCREENS( &aSheet ) {}
    ~SCH_SCREENS();
    size_t GetCount() const { return m_screens.size(); }
    SCH_SCREEN* GetFirst();
    SCH_SCREEN* GetNext();
    SCH_SCREEN* GetScreen( unsigned int aIndex ) const;
    SCH_SHEET* GetSheet( unsigned int aIndex ) const;

    /**
     * Clear the annotation for the symbols inside new sheetpaths
     * when a complex hierarchy is modified and new sheetpaths added
     * when a screen shares more than one sheet path, missing alternate references are added
     * and alternate references of new sheet paths are cleared
     *
     * @param aInitialSheetPathList is the initial sheet paths list of hierarchy before changes.
     */
    void ClearAnnotationOfNewSheetPaths( SCH_SHEET_LIST& aInitialSheetPathList );

    /**
     * Test all sheet and symbol objects in the schematic for duplicate time stamps
     * and replaces them as necessary.
     *
     * Time stamps must be unique in order for complex hierarchies know which symbols go
     * to which sheets.
     *
     * @return The number of duplicate time stamps replaced.
     */
    int ReplaceDuplicateTimeStamps();

    /**
     * Delete all electronic rules check markers of \a aMarkerType from all the screens in
     * the list.
     *
     * @param aMarkerType Type of markers to be deleted.
     */
    void DeleteAllMarkers( enum MARKER_BASE::TYPEMARKER aMarkerType, bool aIncludeExclusions );

    /**
     * Delete all markers of a particular type and error code.
     */
    void DeleteMarkers( enum MARKER_BASE::TYPEMARKER aMarkerTyp, int aErrorCode,
                        bool aIncludeExclusions = true );

    /**
     * Delete a specific marker.
     */
    void DeleteMarker( SCH_MARKER* aMarker );

    /**
     * Initialize the #LIB_SYMBOL reference for each #SCH_SYMBOL found in the full schematic.
     *
     * @note This should only be called when the user specifically requests all library symbol
     *       links to be update or when the legacy schematic is opened for the last time.  All
     *       subsequent schematic loads with the new s-expression will contain the library
     *       symbols.
     *
     * @param[in] aReporter An optional #REPORTER object pointer to write warning and error
     *                      messages into.
     */
    void UpdateSymbolLinks( REPORTER* aReporter = nullptr );

    void ClearEditFlags();

    /**
     * Test all of the schematic symbols to see if all #LIB_ID objects library nickname is not
     * set.
     *
     * If none of the #LIB_ID object library nicknames are not set, this indicates that the
     * project was created before the symbol library implementation.
     *
     * @return true in there are any symbols and if all of the #LIB_ID object library nicknames
     *         are empty, otherwise false.
     */
    bool HasNoFullyDefinedLibIds();

    /**
     * Fetch all of the symbol library nicknames into \a aLibNicknames.
     *
     * @param[out] aLibNicknames is the array to populate with all of the unique library nicknames.
     * @return the number of symbol library nicknames found.
     */
    size_t GetLibNicknames( wxArrayString& aLibNicknames );

    /**
     * Change all of the symbol library nicknames.
     *
     * @param[in] aFrom the current symbol library name to change.
     * @param[in] aTo the new symbol library name.
     * @return the number of symbol library nicknames that were changed.
     */
    int ChangeSymbolLibNickname( const wxString& aFrom, const wxString& aTo );

    /**
     * Check if one of the schematics in the list of screens is \a aSchematicFileName.
     *
     * Schematic file names in SCH_SCREEN object are stored with the absolute path to
     * the schematic file.
     *
     * @param[in] aSchematicFileName is the schematic file name to search.
     * @return true if the a schematic matching the file name has been found.
     */
    bool HasSchematic( const wxString& aSchematicFileName );

    /**
     * built the list of sheet paths sharing a screen for each screen in use
     */
    void BuildClientSheetPathList();

    /**
     * Check \a aSchematicFileName for a potential file name case sensitivity issue.
     *
     * On platforms where file names are case sensitive, it is possible to schematic sheet
     * file names that would cause issues on platforms where file name are case insensitive.
     * File names foo.sch and Foo.sch are unique files on Linux and MacOS but on Windows
     * this would result in a broken schematic.
     *
     * @param[in] aSchematicFileName is the absolute path and file name of the file to test.
     * @return true if \a aSchematicFileName would cause an issue.
     */
    bool CanCauseCaseSensitivityIssue( const wxString& aSchematicFileName ) const;

private:
    void addScreenToList( SCH_SCREEN* aScreen, SCH_SHEET* aSheet );
    void buildScreenList( SCH_SHEET* aSheet);

    std::vector< SCH_SCREEN* > m_screens;
    std::vector< SCH_SHEET* >  m_sheets;
    unsigned int               m_index;
};

#endif /* SCREEN_H */
