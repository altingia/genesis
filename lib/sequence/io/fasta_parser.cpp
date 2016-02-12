/**
 * @brief
 *
 * @file
 * @ingroup sequence
 */

#include "sequence/io/fasta_parser.hpp"

#include "sequence/sequence.hpp"
#include "utils/io/counting_istream.hpp"

#include <assert.h>
#include <cctype>
#include <stdexcept>
#include <string>

namespace genesis {
namespace sequence {

/**
 * @brief Parse a fasta sequence.
 *
 * This function takes an input stream and interprets as as a fasta sequence. It extracts the data
 * and writes it into the given Sequence object. The expected format:
 *
 *   1. Has to start with a '>' character, followed by a label and possibly metadata, ended by a
 *      '\\n'. All text after the first space is considered to be metadata.
 *   2. An arbitrary number of comment lines, starting with ';', can follow, but are ignored.
 *   3. After that, a sequence has to follow, over one or more lines and ending in a '\\n' character.
 *   4. In total, only graphical chars (for which `isgraph(c) == true`), spaces and '\\n' are
 *      allowed in fasta files.
 *
 * The function stops after parsing one such sequence. It returns true if a sequence was extracted
 * and false if the stream is empty. If the input is not in the correct format, an
 * `std::runtime_error` exception is thrown indicating the malicious position in the input stream.
 *
 * See parse_fasta_sequence_fast() for a faster (~ double the speed), but non-error-checking
 * version of this function.
 */
bool parse_fasta_sequence( utils::CountingIstream& input_stream, Sequence& sequence )
{
    // Readability.
    auto& it = input_stream;

    // Check for data.
    if( it.eof() ) {
        return false;
    }

    // Check beginning of sequence.
    if( it.current() != '>' ) {
        throw std::runtime_error(
            "Malformed fasta file: Expecting '>' at beginning of sequence at " + it.at() + "."
        );
    }
    assert( it && *it == '>' );
    ++it;

    // Parse label.
    sequence.label().clear();
    while( it && isgraph(*it) ) {
        sequence.label() += *it;
        ++it;
    }
    if( sequence.label() == "" ) {
        throw std::runtime_error(
            "Malformed fasta file: Expecting label after '>' at " + it.at() + "."
        );
    }

    // Check for unexpected end of file.
    if( it.eof() || (*it != '\n' && *it != ' ')) {
        throw std::runtime_error(
            "Malformed fasta file: Expecting a sequence after the label line at " + it.at() + "."
        );
    }
    assert( it && (*it == '\n' || *it == ' ') );

    // Parse metadata.
    sequence.metadata().clear();
    if( *it == ' ' ) {
        ++it;
        while( it && isprint(*it) ) {
            sequence.metadata() += *it;
            ++it;
        }
    }

    // Check for unexpected end of file.
    if( it.eof() || *it != '\n' ) {
        throw std::runtime_error(
            "Malformed fasta file: Expecting a sequence after the label line at " + it.at() + "."
        );
    }
    assert( it && *it == '\n' );

    // Skip comments.
    while( it && *it == ';' ) {
        while( it && isprint(*it) ) {
            ++it;
        }
    }

    // Check for unexpected end of file.
    if( it.eof() || *it != '\n' ) {
        throw std::runtime_error(
            "Malformed fasta file: Expecting a sequence after the label line at " + it.at() + "."
        );
    }
    assert( it && *it == '\n' );
    ++it;

    // Parse sequence. At every beginning of the outer loop, we are at a line start.
    sequence.sites().clear();
    while( it && *it != '>' ) {
        assert( it.column() == 1 );

        size_t count = 0;
        while( it && isgraph(*it) ) {
            sequence.sites() += *it;
            ++it;
            ++count;
        }

        if( !it ) {
            throw std::runtime_error(
                "Malformed fasta file: Sequence does not end with '\\n' " + it.at() + "."
            );
        }
        assert( it );

        if( *it != '\n' ) {
            throw std::runtime_error(
                "Malformed fasta file: Invalid sequence symbols at " + it.at() + "."
            );
        }
        assert( it && *it == '\n' );
        ++it;

        if( count == 0 ) {
            throw std::runtime_error(
                "Malformed fasta file: Empty sequence line at " + it.at() + "."
            );
        }
    }
    assert( !it || *it == '>' );

    if( sequence.sites().length() == 0 ) {
        throw std::runtime_error(
            "Malformed fasta file: Empty sequence at " + it.at() + "."
        );
    }

    return true;
}

/**
 * @brief Parse a fasta sequence without checking for errors.
 *
 * This is a very fast implementation that neglects input error checking. Thus, the fasta sequence
 * has to be well-formed in order for this function to work properly. See parse_fasta_sequence()
 * for a description of the expected format.
 *
 * If the expected conditions are not met, instead of exceptions, undefined behaviour results.
 * Most probably, it will either write rubbish into the sequence or produce a segfault or an
 * infinite loop. So be warned and check your data first. If they are good, enjoy the speed!
 */
bool parse_fasta_sequence_fast( utils::CountingIstream& input_stream, Sequence& sequence )
{
    // Readability.
    auto& it = input_stream;

    // Check for data.
    if( it.eof() ) {
        return false;
    }

    // Check beginning of sequence.
    assert( *it == '>' );
    it.advance_non_counting();
    assert( it );

    // Parse label.
    sequence.label().clear();
    while( *it != '\n' && *it != ' ' ) {
        sequence.label() += *it;
        it.advance_non_counting();
        assert( it );
    }
    assert( *it == '\n' || *it == ' ' );

    // Parse metadata.
    sequence.metadata().clear();
    if( *it == ' ' ) {
        it.advance_non_counting();
        assert( it );
        while( *it != '\n' ) {
            sequence.metadata() += *it;
            it.advance_non_counting();
            assert( it );
        }
    }
    assert( *it == '\n' );
    it.advance_non_counting();
    assert( it );

    // Skip comments.
    while( *it == ';' ) {
        while( *it != '\n' ) {
            it.advance_non_counting();
        }
        assert( *it == '\n' );
        it.advance_non_counting();
        assert( it );
    }

    // Parse sequence. At every beginning of the outer loop, we are at a line start.
    sequence.sites().clear();
    while( it && *it != '>' ) {
        while( *it != '\n' ) {
            sequence.sites() += *it;
            it.advance_non_counting();
            assert( it );
        }
        assert( *it == '\n' );
        it.advance_non_counting();
    }

    return true;
}

} // namespace sequence
} // namespace genesis