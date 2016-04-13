/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2016 Lucas Czech

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contact:
    Lucas Czech <lucas.czech@h-its.org>
    Exelixis Lab, Heidelberg Institute for Theoretical Studies
    Schloss-Wolfsbrunnenweg 35, D-69118 Heidelberg, Germany
*/

/**
 * @brief
 *
 * @file
 * @ingroup test
 */

#include "common.hpp"

#include "lib/utils/core/std.hpp"

using namespace genesis;

TEST(Std, RoundTo)
{
    EXPECT_FLOAT_EQ( 3.0     , round_to( 3.1415926535, 0 ) );
    EXPECT_FLOAT_EQ( 3.1     , round_to( 3.1415926535, 1 ) );
    EXPECT_FLOAT_EQ( 3.14    , round_to( 3.1415926535, 2 ) );
    EXPECT_FLOAT_EQ( 3.142   , round_to( 3.1415926535, 3 ) );
    EXPECT_FLOAT_EQ( 3.1416  , round_to( 3.1415926535, 4 ) );
    EXPECT_FLOAT_EQ( 3.14159 , round_to( 3.1415926535, 5 ) );
}
