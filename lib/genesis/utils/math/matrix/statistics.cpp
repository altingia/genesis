/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2017 Lucas Czech

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation,  either version 3 of the License,  or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not,  see <http://www.gnu.org/licenses/>.

    Contact:
    Lucas Czech <lucas.czech@h-its.org>
    Exelixis Lab,  Heidelberg Institute for Theoretical Studies
    Schloss-Wolfsbrunnenweg 35,  D-69118 Heidelberg,  Germany
*/

/**
 * @brief
 *
 * @file
 * @ingroup utils
 */

#include "genesis/utils/math/matrix/statistics.hpp"

#include "genesis/utils/core/logging.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdexcept>

#ifdef GENESIS_OPENMP
#   include <omp.h>
#endif

namespace genesis {
namespace utils {

// ================================================================================================
//     Normalize
// ================================================================================================

std::vector<MinMaxPair<double>> normalize_cols( Matrix<double>& data )
{
    auto col_minmax = matrix_col_minmax( data );
    assert( col_minmax.size() == data.cols() );

    // Iterate the matrix.
    #pragma omp parallel for
    for( size_t r = 0; r < data.rows(); ++r ) {
        for( size_t c = 0; c < data.cols(); ++c ) {
            // Adjust column values.
            assert( col_minmax[c].max >= col_minmax[c].min );
            double diff = col_minmax[c].max - col_minmax[c].min;
            data( r, c ) = ( data( r, c ) - col_minmax[c].min ) / diff;
        }
    }

    return col_minmax;
}

std::vector<MinMaxPair<double>> normalize_rows( Matrix<double>& data )
{
    auto row_minmax = matrix_row_minmax( data );
    assert( row_minmax.size() == data.rows() );

    // Iterate the matrix.
    #pragma omp parallel for
    for( size_t r = 0; r < data.rows(); ++r ) {
        for( size_t c = 0; c < data.cols(); ++c ) {
            // Adjust row values.
            assert( row_minmax[r].max >= row_minmax[r].min );
            double diff = row_minmax[r].max - row_minmax[r].min;
            data( r, c ) = ( data( r, c ) - row_minmax[r].min ) / diff;
        }
    }

    return row_minmax;
}

// ================================================================================================
//     Standardize
// ================================================================================================

std::vector<MeanStddevPair> standardize_cols(
    Matrix<double>& data,
    bool            scale_means,
    bool            scale_std
) {
    auto col_mean_stddev = matrix_col_mean_stddev( data, 0.0000001 );
    assert( col_mean_stddev.size() == data.cols() );

    // Iterate the matrix.
    #pragma omp parallel for
    for( size_t r = 0; r < data.rows(); ++r ) {
        for( size_t c = 0; c < data.cols(); ++c ) {

            // Subtract mean (i.e., center data).
            if( scale_means ) {
                data( r, c ) -= col_mean_stddev[c].mean;
            }

            // Scale to unit variance, if needed.
            if( scale_std ) {
                assert( col_mean_stddev[c].stddev > 0.0 );
                data( r, c ) /= col_mean_stddev[c].stddev;
            }
        }
    }

    return col_mean_stddev;
}

std::vector<MeanStddevPair> standardize_rows(
    Matrix<double>& data,
    bool            scale_means,
    bool            scale_std
) {
    auto row_mean_stddev = matrix_row_mean_stddev( data, 0.0000001 );
    assert( row_mean_stddev.size() == data.rows() );

    // Iterate the matrix.
    #pragma omp parallel for
    for( size_t r = 0; r < data.rows(); ++r ) {
        for( size_t c = 0; c < data.cols(); ++c ) {

            // Subtract mean (i.e., center data).
            if( scale_means ) {
                data( r, c ) -= row_mean_stddev[r].mean;
            }

            // Scale to unit variance, if needed.
            if( scale_std ) {
                assert( row_mean_stddev[r].stddev > 0.0 );
                data( r, c ) /= row_mean_stddev[r].stddev;
            }
        }
    }

    return row_mean_stddev;
}

// ================================================================================================
//     Mean and Stddev
// ================================================================================================

std::vector<MeanStddevPair> matrix_col_mean_stddev( Matrix<double> const& data, double epsilon )
{
    auto ret = std::vector<MeanStddevPair>( data.cols(), { 0.0, 0.0 } );

    // Nothing to do. Better stop here or we risk dividing by zero.
    if( data.rows() == 0 ) {
        return ret;
    }

    // Iterate columns.
    #pragma omp parallel for
    for( size_t c = 0; c < data.cols(); ++c ) {
        double mean   = 0.0;
        double stddev = 0.0;

        // Calculate column mean.
        for( size_t r = 0; r < data.rows(); ++r ) {
            mean += data( r, c );
        }
        mean /= static_cast<double>( data.rows() );

        // Calculate column std dev.
        for( size_t r = 0; r < data.rows(); ++r ) {
            stddev += (( data( r, c ) - mean ) * ( data( r, c ) - mean ));
        }
        stddev /= static_cast<double>( data.rows() );
        stddev = std::sqrt(stddev);

        // The following in an inelegant (but usual) way to handle near-zero values,
        // which later would cause a division by zero.
        assert( stddev >= 0.0 );
        if( stddev <= epsilon ){
            stddev = 1.0;
        }

        // Set result entries.
        ret[ c ].mean   = mean;
        ret[ c ].stddev = stddev;
    }

    return ret;
}

std::vector<MeanStddevPair> matrix_row_mean_stddev( Matrix<double> const& data, double epsilon )
{
    auto ret = std::vector<MeanStddevPair>( data.rows(), { 0.0, 0.0 } );

    // Nothing to do. Better stop here or we risk dividing by zero.
    if( data.cols() == 0 ) {
        return ret;
    }

    // Iterate columns.
    #pragma omp parallel for
    for( size_t r = 0; r < data.rows(); ++r ) {
        double mean   = 0.0;
        double stddev = 0.0;

        // Calculate row mean.
        for( size_t c = 0; c < data.cols(); ++c ) {
            mean += data( r, c );
        }
        mean /= static_cast<double>( data.cols() );

        // Calculate column std dev.
        for( size_t c = 0; c < data.cols(); ++c ) {
            stddev += (( data( r, c ) - mean ) * ( data( r, c ) - mean ));
        }
        stddev /= static_cast<double>( data.cols() );
        stddev = sqrt(stddev);

        // The following in an inelegant (but usual) way to handle near-zero values,
        // which later would cause a division by zero.
        assert( stddev >= 0.0 );
        if( stddev <= epsilon ){
            stddev = 1.0;
        }

        // Set result entries.
        ret[ r ].mean   = mean;
        ret[ r ].stddev = stddev;
    }

    return ret;
}

// ================================================================================================
//     Correlation Matrix
// ================================================================================================

Matrix<double> correlation_matrix( Matrix<double> const& data )
{
    // Standardize mean and variance.
    auto stddata = data;
    standardize_cols( stddata, true, true );

    // Calculate matrix. First build the sum of squares, then normalize.
    auto sscp = sums_of_squares_and_cross_products_matrix( stddata );
    for( auto& elem : sscp ) {
        elem /= static_cast<double>( data.rows() );
    }
    return sscp;
}

// ================================================================================================
//     Covariance Matrix
// ================================================================================================

Matrix<double> covariance_matrix( Matrix<double> const& data )
{
    // Standardize mean, but not the variance.
    auto stddata = data;
    standardize_cols( stddata, true, false );

    // Calculate matrix. First build the sum of squares, then normalize.
    auto sscp = sums_of_squares_and_cross_products_matrix( stddata );
    for( auto& elem : sscp ) {
        elem /= static_cast<double>( data.rows() );
    }
    return sscp;
}

// ================================================================================================
//     Sums of Squares Cross Products Matrix
// ================================================================================================

Matrix<double> sums_of_squares_and_cross_products_matrix( Matrix<double> const& data )
{
    auto mat = Matrix<double>( data.cols(), data.cols() );

    // Calculate the covariance matrix.
    for( size_t c1 = 0; c1 < data.cols(); ++c1 ) {
        for( size_t c2 = c1; c2 < data.cols(); ++c2 ) {
            mat( c1, c2 ) = 0.0;
            for( size_t r = 0; r < data.rows(); ++r ) {
                mat( c1, c2 ) += data( r, c1 ) * data( r, c2 );
            }
            mat( c2, c1 ) = mat( c1, c2 );
        }
    }

    return mat;
}

// =================================================================================================
//     Pearson Correlation Coefficient
// =================================================================================================

double matrix_col_pearson_correlation_coefficient(
    Matrix<double> const& mat1, size_t col1,
    Matrix<double> const& mat2, size_t col2
) {
    if( mat1.rows() != mat2.rows() ) {
        throw std::runtime_error( "Matrices need to have same number of rows." );
    }
    if( col1 >= mat1.cols() || col2 >= mat2.cols() ) {
        throw std::runtime_error( "Column indices cannot be bigger then number of columns." );
    }

    // Calculate column means.
    double mean1 = 0.0;
    double mean2 = 0.0;
    for( size_t r = 0; r < mat1.rows(); ++r ) {
        mean1 += mat1( r, col1 );
        mean2 += mat2( r, col2 );
    }
    mean1 /= static_cast<double>( mat1.rows() );
    mean2 /= static_cast<double>( mat2.rows() );

    // Calculate PCC parts.
    double numerator = 0.0;
    double stddev1   = 0.0;
    double stddev2   = 0.0;
    for( size_t r = 0; r < mat1.rows(); ++r ) {
        double const d1 = mat1( r, col1 ) - mean1;
        double const d2 = mat2( r, col2 ) - mean2;
        numerator += d1 * d2;
        stddev1   += d1 * d1;
        stddev2   += d2 * d2;
    }

    // Calcualte PCC, and assert that it is in the correct range
    // (or not a number, which can happen if the std dev is 0.0, e.g. in all-zero columns).
    auto const pcc = numerator / ( std::sqrt( stddev1 ) * std::sqrt( stddev2 ) );
    assert(( -1.0 <= pcc && pcc <= 1.0 ) || ( ! std::isfinite( pcc ) ));
    return pcc;
}

} // namespace utils
} // namespace genesis
