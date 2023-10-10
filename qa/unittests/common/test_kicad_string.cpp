/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 KiCad Developers, see AUTHORS.TXT for contributors.
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

/**
 * @file
 * Test suite for general string functions
 */

#include <qa_utils/wx_utils/unit_test_utils.h>

// Code under test
#include <string_utils.h>
#include <richio.h>

/**
 * Declare the test suite
 */
BOOST_AUTO_TEST_SUITE( KicadString )


/**
 * Test the #GetTrailingInt method.
 */
BOOST_AUTO_TEST_CASE( TrailingInt )
{
    using CASE = std::pair<std::string, int>;

    const std::vector<CASE> cases = {
        { "", 0 }, { "foo", 0 },            // no int
        { "0", 0 },                         // only int
        { "42", 42 },                       // only int
        { "1001", 1001 },                   // only int
        { "Foo42", 42 }, { "12Foo42", 42 }, // only the trailing
        { "12Foo4.2", 2 },                  // no dots
    };

    for( const auto& c : cases )
    {
        BOOST_CHECK_EQUAL( GetTrailingInt( c.first ), c.second );
    }
}


/**
 * Test the #StrNumCmp method.
 */
BOOST_AUTO_TEST_CASE( NaturalNumberCompare )
{
    using CASE = std::pair<std::pair<std::string, std::string>, std::pair<int, int>>;

    const std::vector<CASE> cases = {
        { { "a", "b" }, { -1, -1 } },
        { { "b", "a" }, { 1, 1 } },
        { { "a", "a" }, { 0, 0 } },
        { { "a", "A" }, { 1, 0 } },
        { { "A", "a" }, { -1, 0 } },
        { { "a", "" }, { 1, 1 } },
        { { "", "a" }, { -1, -1 } },
        { { "1", "" }, { 1, 1 } },
        { { "", "1" }, { -1, -1 } },
        { { "10", "2" }, { 1, 1 } },
        { { "2", "10" }, { -1, -1 } },
        { { "2", "2" }, { 0, 0 } },
        { { "10", "10" }, { 0, 0 } },
        { { "01", "1" }, { 0, 0 } },
        { { "01a", "1a" }, { 0, 0 } },
        { { "01a", "1b" }, { -1, -1 } },
        { { "01b", "1a" }, { 1, 1 } },
        { { "10 ten", "2 two" }, { 1, 1 } },
        { { "SYM1", "sym2" }, { -1, -1 } },
        { { "sym2", "SYM1" }, { 1, 1 } },
        { { "a10b20c30", "a10b20c31" }, { -1, -1 } },
        { { "a10b20c31", "a10b20c30" }, { 1, 1 } },
        { { "10UF", "10UF" }, { 0, 0 } },
        { { "10uF", "10uF" }, { 0, 0 } },
        { { "u10", "u10" }, { 0, 0 } },
        { { "U10", "U10" }, { 0, 0 } },
        { { "u10", "U10" }, { 1, 0 } },
        { { "U10", "u10" }, { -1, 0 } },
        { { "U10.1", "U10.10" }, { -1, -1 } },
        { { "U10-1", "U10-10" }, { -1, -1 } },
        { { "U10,1", "U10,10" }, { -1, -1 } },
        { { "U10.A", "U10.a" }, { -1, 0 } },
        { { "U10-A", "U10-a" }, { -1, 0 } },
        { { "U10,A", "U10,a" }, { -1, 0 } },
    };

    for( const auto& c : cases )
    {
        BOOST_CHECK_MESSAGE( StrNumCmp( c.first.first, c.first.second ) == c.second.first,
                c.first.first + " AND " + c.first.second + " failed for case sensitive" );

        BOOST_CHECK_MESSAGE(
                StrNumCmp( c.first.first, c.first.second, true ) == c.second.second,
                c.first.first + " AND " + c.first.second + " failed for case insensitive" );
    }
}


/**
 * Test the #GetTrailingInt method.
 */
BOOST_AUTO_TEST_CASE( Double2Str )
{
    using CASE = std::pair<double, std::string>;

    // conceptually a little quirky because doubles do have all those pesky additional values
    const std::vector<CASE> cases = {
        { 0, "0" },
        { 1.000, "1" },
        { 1.050, "1.05" },                       // single trailing zero
        { 0.00001523, "0.00001523" },            // value less than the magic 0.0001 threshold
        { 0.00000000000000001523, "0" },         // really small decimal that gets cut off
        { 623523, "623523" },                    // large whole number
    };

    for( const auto& c : cases )
    {
        // Test both of these functions that work the same but the innards are different
        BOOST_CHECK_EQUAL( FormatDouble2Str( c.first ), c.second );
        BOOST_CHECK_EQUAL( UIDouble2Str( c.first ), c.second );
    }
}

/**
 * Test the #vprint method.
 */
BOOST_AUTO_TEST_CASE( VPrint )
{
    std::string output;

    // Test 1: Basic strings and numbers
    StrPrintf( &output, "Hello %s! ", "World" );
    StrPrintf( &output, "Number: %d, ", 42 );
    StrPrintf( &output, "Float: %.2f, ", 3.14 );
    StrPrintf( &output, "Char: %c. ", 'A' );
    BOOST_CHECK_EQUAL( output, std::string( "Hello World! Number: 42, Float: 3.14, Char: A. " ) );
    output.clear();

    // Test 2: Large string
    std::string longString( 500, 'A' );
    StrPrintf( &output, "%s", longString.c_str() );
    BOOST_CHECK_EQUAL( output, longString );
    output.clear();

    // Test 3: Edge case with zero characters
    StrPrintf( &output, "" );
    BOOST_ASSERT( output.empty() );

    // Test 4: Mixing small and large strings
    StrPrintf( &output, "Small, " );
    StrPrintf( &output, "%s, ", longString.c_str() );
    StrPrintf( &output, "End." );
    BOOST_CHECK_EQUAL( output, std::string( "Small, " + longString + ", End." ) );
    output.clear();

    // Test 5: Formatting with various data types
    StrPrintf( &output, "%d %s %c %.2f", 123, "Hello", 'X', 9.876 );
    BOOST_CHECK_EQUAL( output, std::string( "123 Hello X 9.88" ) );
    output.clear();
}

BOOST_AUTO_TEST_SUITE_END()