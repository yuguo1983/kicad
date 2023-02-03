/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2022 Mark Roszko <mark.roszko@gmail.com>
 * Copyright (C) 1992-2022 KiCad Developers, see AUTHORS.txt for contributors.
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

#include "command_fp_export_svg.h"
#include <cli/exit_codes.h>
#include "jobs/job_fp_export_svg.h"
#include <kiface_base.h>
#include <layer_ids.h>
#include <wx/crt.h>
#include <wx/dir.h>

#include <macros.h>
#include <wx/tokenzr.h>

#define ARG_FOOTPRINT "--footprint"

CLI::FP_EXPORT_SVG_COMMAND::FP_EXPORT_SVG_COMMAND() : EXPORT_PCB_BASE_COMMAND( "svg" )
{
    addLayerArg( false );

    m_argParser.add_argument( "-t", ARG_THEME )
            .default_value( std::string() )
            .help( UTF8STDSTR( _( "Color theme to use (will default to pcbnew settings)" ) ) );

    m_argParser.add_argument( "-fp", ARG_FOOTPRINT )
            .default_value( std::string() )
            .help( UTF8STDSTR( _( "Specific symbol to export within the library" ) ) );

    m_argParser.add_argument( ARG_BLACKANDWHITE )
            .help( UTF8STDSTR( _( ARG_BLACKANDWHITE_DESC ) ) )
            .implicit_value( true )
            .default_value( false );
}


int CLI::FP_EXPORT_SVG_COMMAND::doPerform( KIWAY& aKiway )
{
    int baseExit = EXPORT_PCB_BASE_COMMAND::doPerform( aKiway );
    if( baseExit != EXIT_CODES::OK )
        return baseExit;

    std::unique_ptr<JOB_FP_EXPORT_SVG> svgJob = std::make_unique<JOB_FP_EXPORT_SVG>( true );

    svgJob->m_libraryPath = FROM_UTF8( m_argParser.get<std::string>( ARG_INPUT ).c_str() );
    svgJob->m_outputDirectory = FROM_UTF8( m_argParser.get<std::string>( ARG_OUTPUT ).c_str() );
    svgJob->m_blackAndWhite = m_argParser.get<bool>( ARG_BLACKANDWHITE );
    svgJob->m_footprint = FROM_UTF8( m_argParser.get<std::string>( ARG_FOOTPRINT ).c_str() );

    if( !wxDir::Exists( svgJob->m_libraryPath ) )
    {
        wxFprintf( stderr, _( "Footprint library does not exist or is not accessible\n" ) );
        return EXIT_CODES::ERR_INVALID_INPUT_FILE;
    }

    svgJob->m_colorTheme = FROM_UTF8( m_argParser.get<std::string>( ARG_THEME ).c_str() );

    if( m_selectedLayers.count() > 0 )
        svgJob->m_printMaskLayer = m_selectedLayers;
    else
        svgJob->m_printMaskLayer = LSET::AllLayersMask();

    int exitCode = aKiway.ProcessJob( KIWAY::FACE_PCB, svgJob.get() );

    return exitCode;
}