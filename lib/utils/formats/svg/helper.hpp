#ifndef GENESIS_UTILS_FORMATS_SVG_HELPER_H_
#define GENESIS_UTILS_FORMATS_SVG_HELPER_H_

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
 * @ingroup utils
 */

#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace genesis {
namespace utils {

// =================================================================================================
//     Svg Point
// =================================================================================================

struct SvgPoint
{
    explicit SvgPoint( double x = 0.0, double y = 0.0 )
        : x(x)
        , y(y)
    {}

    double x;
    double y;
};

// =================================================================================================
//     Svg Size
// =================================================================================================

struct SvgSize
{
    explicit SvgSize( double width = 0.0, double height = 0.0 )
        : width(width)
        , height(height)
    {}

    double width;
    double height;
};

// =================================================================================================
//     Svg Helper Functions
// =================================================================================================

inline std::string svg_comment( std::string const& content )
{
    return "<!-- " + content + " -->\n";
}

template< typename T >
std::string svg_attribute(
    std::string const& name,
    T const&           value,
    std::string const& unit = ""
) {
    std::stringstream ss;
    ss << " " << name << "=\"" << value << unit << "\"";
    return ss.str();
}

// template< typename T >
// std::string svg_style(
//     std::string const& name,
//     T const&           value,
//     std::string const& unit = ""
// ) {
//     std::stringstream ss;
//     ss << name << ":\"" << value << unit << "\" ";
//     return ss.str();
// }

} // namespace utils
} // namespace genesis

#endif // include guard
