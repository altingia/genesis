#ifndef GENESIS_SEQUENCE_IO_FASTA_PARSER_H_
#define GENESIS_SEQUENCE_IO_FASTA_PARSER_H_

/**
 * @brief
 *
 * @file
 * @ingroup sequence
 */

namespace genesis {

// =================================================================================================
//     Forward Declarations
// =================================================================================================

namespace utils {

class CountingIstream;

}

namespace sequence {
class Sequence;

// =================================================================================================
//     Fasta Parser
// =================================================================================================

bool parse_fasta_sequence(      utils::CountingIstream& input_stream, Sequence& sequence );
bool parse_fasta_sequence_fast( utils::CountingIstream& input_stream, Sequence& sequence );

} // namespace sequence
} // namespace genesis

#endif // include guard