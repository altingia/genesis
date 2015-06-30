/**
 * @brief Implementation of functions for parsing and printing FASTA documents.
 *
 * @file
 * @ingroup alignment
 */

#include "alignment/fasta_processor.hpp"

#include <sstream>
#include <string>

#include "alignment/fasta_lexer.hpp"
#include "alignment/sequence_set.hpp"
#include "utils/logging.hpp"
#include "utils/utils.hpp"

namespace genesis {

// =============================================================================
//     Parsing
// =============================================================================

/**
 * @brief
 */
bool FastaProcessor::from_file (const std::string fn, SequenceSet& aln)
{
    if (!file_exists(fn)) {
        LOG_WARN << "FASTA file '" << fn << "' does not exist.";
        return false;
    }
    return from_string(file_read(fn), aln);
}

/**
 * @brief
 */
bool FastaProcessor::from_string (const std::string& fs, SequenceSet& aln)
{
    // do stepwise lexing
    FastaLexer lexer;
    lexer.from_string(fs);

    // basic checks
    if (lexer.empty()) {
        LOG_INFO << "FASTA document is empty.";
        return false;
    }
    if (lexer.has_error()) {
        LOG_WARN << "Lexing error at " << lexer.back().at()
                 << " with message: " << lexer.back().value();
        return false;
    }
    aln.clear();

    // delete tailing tokens immediately, produce tokens intime.
    FastaLexer::iterator it = lexer.begin();

    std::string        label;
    std::ostringstream seq;

    // process all sequences
    while (it != lexer.end()) {
        // parse label
        if (!it->is_tag()) {
            LOG_WARN << "FASTA sequence does not start with '>' at " << it->at();
            return false;
        }
        label = it->value();
        ++it;

        // parse sequence
        seq.str("");
        seq.clear();
        while (it != lexer.end() && it->is_symbol()) {
            seq << it->value();
            ++it;
        }

        // add to alignment
        Sequence* nseq = new Sequence(label, seq.str());
        aln.sequences.push_back(nseq);

        // there are no other lexer tokens than tag and symbol for fasta files!
        // not even an error token can be produced by the lexer in its current implementation.
        assert(it == lexer.end() || it->is_tag());
    }

    return true;
}

// =============================================================================
//     Printing
// =============================================================================

/**
 * @brief
 */
bool FastaProcessor::to_file (const SequenceSet& sset, const std::string fn)
{
    if (file_exists(fn)) {
        LOG_WARN << "FASTA file '" << fn << "' already exist. Will not overwrite it.";
        return false;
    }
    std::string fs;
    to_string(sset, fs);
    return file_write(fn, fs);
}

/**
 * @brief
 */
void FastaProcessor::to_string (const SequenceSet& sset, std::string& fs)
{
    fs = to_string(sset);
}

/**
 * @brief
 */
std::string FastaProcessor::to_string (const SequenceSet& sset)
{
    std::ostringstream seq("");
    for (Sequence* s : sset.sequences) {
        // print label
        seq << ">" << s->label() << "\n";

        // print sequence. if needed, add new line at every line_length position.
        if (line_length > 0) {
            for (size_t i = 0; i < s->length(); i += line_length) {
                // write line_length many characters.
                // (if the string is shorter, as many characters as possible are used)
                seq << s->sites().substr(i, line_length) << "\n";
            }
        } else {
            seq << s->sites() << "\n";
        }
    }
    return seq.str();
}

} // namespace genesis
