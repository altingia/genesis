#ifndef GENESIS_UTILS_IO_NEXUS_BLOCK_H_
#define GENESIS_UTILS_IO_NEXUS_BLOCK_H_

/**
 * @brief
 *
 * @file
 * @ingroup utils
 */

#include <string>
#include <iosfwd>

namespace genesis {
namespace nexus {

// =================================================================================================
//     Nexus Block
// =================================================================================================

/**
 * @brief
 */
class Block
{

    // -----------------------------------------------------
    //     Virtual Functions
    // -----------------------------------------------------

public:

    virtual std::string block_name() const = 0;
    virtual void to_stream( std::ostream& os ) const = 0;

};

// =================================================================================================
//     Operators
// =================================================================================================

std::ostream& operator<< (std::ostream& os, Block const& block);

} // namespace nexus
} // namespace genesis

#endif // include guard
