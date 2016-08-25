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
 * @ingroup sequence
 */

#include "sequence/functions/consensus.hpp"

#include "sequence/counts.hpp"
#include "sequence/sequence_set.hpp"
#include "sequence/sequence.hpp"
#include "sequence/functions/codes.hpp"
#include "sequence/functions/functions.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace genesis {
namespace sequence {

// =================================================================================================
//     Majority
// =================================================================================================

/**
 * @brief Calculate the majority rule consensus sequence by using the most frequent character at
 * each site.
 *
 * The function creates a consensus sequence by using the character at each position that has the
 * highest count (or frequency). It does not assume any character codes. Thus, it works for all
 * kinds of sequence codes, e.g., nucleic acid or amino acid codes.
 *
 * The optional parameter `allow_gaps` (default is `true`) determines whether gaps in the consensus
 * sequence are allowed. By default, if a site consists mostly of gaps, the consensus sequence also
 * contains a gap at that site. If however this option is set to `false`, the consensus sequence
 * will contain the most frequent non-gap character, even if there are more gaps at this site than
 * the character itself. In other words, if the parameter is set to `false`, gaps are treated as
 * missing characters instead of another type of character for computing the consensus.
 * The only exception are gaps-only sites; in this case, the resulting sites contain a gap
 * characters even if the parameter is set to `false`.
 *
 * The optional parameter `gap_char` (default value '-') is used for sites where no counts are
 * available (i.e., are all zero), or, if `allow_gaps` is set to `true`, for sites that contain
 * mostly gaps.
 *
 * Furthermore, if two or more characters have the same frequency, the first one is used. That is,
 * the one that appears first in SequenceCounts::characters().
 *
 * For an alternative version of this function that takes those ambiguities into account, see
 * consensus_sequence_with_ambiguities(). Also, for a version of this function that takes a
 * threshold for the character frequencies into account, see consensus_sequence_with_threshold().
 * However, both of them currently only work for nucleic acid codes (`ACGT`).
 */
std::string consensus_sequence_with_majorities(
    SequenceCounts const& counts,
    bool                  allow_gaps,
    char                  gap_char
) {
    std::string result;
    result.reserve( counts.length() );

    // Prepare some constants for simplicity.
    auto const chars     = counts.characters();
    auto const seq_count = counts.added_sequences_count();
    auto const num_chars = counts.characters().size();

    for( size_t site_idx = 0; site_idx < counts.length(); ++site_idx ) {

        size_t                        max_pos    = 0;
        SequenceCounts::CountsIntType max_val    = 0;
        SequenceCounts::CountsIntType counts_sum = 0;

        for( size_t char_idx = 0; char_idx < num_chars; ++char_idx ) {
            auto const char_count = counts.count_at( site_idx, char_idx );
            counts_sum += char_count;

            // We use a strict greater here, as this ensures to use the first character in cases
            // where many have the same count.
            if( char_count > max_val ) {
                max_pos = char_idx;
                max_val = char_count;
            }
        }

        // We can never have a max higher than the total sum of counts, and this again cannot be
        // higher then the number of sequences that were added to the counts object.
        assert( max_val    <= counts_sum );
        assert( counts_sum <= seq_count  );

        // We write a code char if it is the majority, that is, > 0 and > all other code counts.
        // In other cases, write a gap. That is, either no code has a count > 0, or, if we allow
        // gaps and gaps are more frquent than actual codes.
        auto const gap_count = seq_count - counts_sum;
        if(( max_val > 0 ) && (( ! allow_gaps ) || ( max_val > gap_count ))) {
            result += chars[ max_pos ];
        } else {
            result += gap_char;
        }
    }

    return result;
}

/**
 * @brief Calculate the majority rule consensus sequence by using the most frequent character at
 * each site.
 *
 * See @link consensus_sequence_with_majorities( SequenceCounts const&, bool, char )
 * consensus_sequence_with_majorities(...)@endlink for details.
 *
 * This function is merely a wrapper that instead of a SequenceCount objects, takes a SequenceSet
 * object and the set of characters to be used for counting character frequencies in the Sequence%s.
 * That means, only the provided characters are counted and used for the consensus sequence.
 * This is useful in order to get rid of errors and noise in the Sequence%s. For example, if you
 * want to build a consensus sequence for a set of sequences with amino acid codes, use
 * amino_acid_codes_plain() for set `characters` parameter.
 */
std::string consensus_sequence_with_majorities(
    SequenceSet const&    sequences,
    std::string const&    characters,
    bool                  allow_gaps,
    char                  gap_char
) {
    // Basic checks.
    if( sequences.size() == 0 ) {
        throw std::runtime_error( "Cannot caluclate consensus sequence of empty SequenceSet." );
    }
    if( ! is_alignment( sequences ) ) {
        throw std::runtime_error(
            "Cannot caluclate consensus sequence for SequenceSet that is not an alignment. "
            "That is, all Sequences need to have the same length."
        );
    }

    // Build counts object.
    auto counts = SequenceCounts( characters, sequences[0].size() );
    counts.add_sequences( sequences );

    // Return consensus sequence.
    return consensus_sequence_with_majorities( counts, allow_gaps, gap_char );
}

/**
 * @brief Calculate the majority rule consensus sequence by using the most frequent character at
 * each site for nucleic acid codes `ACGT`.
 *
 * See @link consensus_sequence_with_majorities( SequenceCounts const&, bool, char )
 * consensus_sequence_with_majorities(...)@endlink for details.
 *
 * This function is merely a wrapper that instead of a SequenceCount objects, takes a SequenceSet
 * object consisting of Sequence%s with nucleic acid codes `ACGT` and uses '-' for gaps.
 */
std::string consensus_sequence_with_majorities(
    SequenceSet const&    sequences,
    bool                  allow_gaps
) {
    return consensus_sequence_with_majorities(
        sequences,
        nucleic_acid_codes_plain(),
        allow_gaps,
        '-'
    );
}

// =================================================================================================
//     Ambiguity
// =================================================================================================

/**
 * @brief Calculate a consensus sequence by using the most frequent characters at each site,
 * for nucleic acid codes `ACGT` and their ambiguities.
 *
 * The function calculates a consensus sequence for nucleic acid codes (`ACGT`), using their
 * ambiguity codes (e.g., `W` for "weak" == `AT`) if the counts (i.e., character frequencies) are
 * similar at a site. It uses `similarity_factor` to decide which counts are close enough to each
 * other in order to be considered ambiguous.
 *
 * For example, with `similarity_factor == 1.0`, only exact matches are used, that is, if two
 * counts are exactly the same. Let <code>count('A') == 42</code> and <code>count('T') == 42</code>,
 * and both other counts be `0`, this results in the code `W` at that site. If however
 * <code>count('T') == 41</code>, only `A` is used for the site.
 * Thus, with `similarity_factor == 1.0`, this function behaves very similar to
 * consensus_sequence_with_majorities(), except in cases were two counts are exaclty the same.
 *
 * On the other hand, with `similarity_factor == 0.0`, all codes that are present at a site are
 * considered to be ambiguous. That is, if a site contains `counts > 0` for `A`, `G` and `T`, the
 * resulting site gets the code `D` ("not C").
 *
 * For intermediate values, e.g., the default `0.9`, the value is used as a threshold to decide
 * the ambiguities. For example, let <code>count('A') == 42</code> and <code>count('T') == 38</code>,
 * and both other counts be `0`. Then, the allowed deviation from the maximum `42` is
 * `0.9 * 42 = 37.8`. Thus, as the count for `T` is above this value, those two codes are
 * considered ambiguous, resulting in a `W` at that site.
 *
 * The optional parameter `allow_gaps` (default is `true`) behaves similar to its counterpart in
 * consensus_sequence_with_majorities(). If set to `true`, the count of the gap character is also
 * considered. If then the count of no character is within the similarity range of the gap count,
 * the result contains a gap at that site. If however there are codes within the range (i.e., above
 * `similarity_factor * max_count`), those are used instead, even if gaps are more frequent.
 *
 * If `allow_gaps` is set to `false` instead, gaps are not considered. That means, the ambiguities
 * are calculated as if there were no gaps. So even if a site contains mostly gaps, but only a few
 * other characters, those will be used. Solely all-gap sites result in a gap at that site.
 *
 * Remark: As this function expects nucleic acid codes, the gap character is fixed to '-' here.
 * The ambiguity codes are converted using nucleic_acid_ambiguity_code(). See there for more
 * information on the used codes.
 *
 * If the provided SequenceCounts object does not use nucleic acid codes, or if the
 * `similarity_factor` is not within the range `[ 0.0, 1.0 ]`, an exception is thrown.
 */
std::string consensus_sequence_with_ambiguities(
    SequenceCounts const& counts,
    double                similarity_factor,
    bool                  allow_gaps
) {
    // Check whether the counts object uses the needed character codes for this function.
    // The characters in the counts object are sorted, so we can directly compare then like this.
    if( counts.characters() != "ACGT" ) {
        throw std::runtime_error(
            "Computation of consensus_sequence_with_ambiguities() only possible "
            "for nucleic acid codes (ACGT)."
        );
    }

    // Check the deviation range.
    if( similarity_factor < 0.0 || similarity_factor > 1.0 ) {
        throw std::invalid_argument(
            "Value of similarity_factor has to be in range [ 0.0, 1.0 ]."
        );
    }

    std::string result;
    result.reserve( counts.length() );

    // Prepare some constants for simplicity.
    auto const chars     = counts.characters();
    auto const seq_count = counts.added_sequences_count();
    auto const num_chars = counts.characters().size();

    // Use a hard coded gap char here, as we have fixed character codes anyway.
    auto const gap_char  = '-';

    // We expect ACGT here.
    assert( num_chars == 4 );

    // Helper struct to store the codes sorted by count.
    using CountPair = std::pair< size_t, char >;
    auto count_pair_compare = []( CountPair const& lhs, CountPair const& rhs ) {
        return lhs.first > rhs.first;
    };

    // Process all sites of the sequence.
    for( size_t site_idx = 0; site_idx < counts.length(); ++site_idx ) {

        // Total sum of counts. Used for getting the number of gaps.
        SequenceCounts::CountsIntType counts_sum = 0;

        // Map from counts to positions. We use this for sorting by count. It's a vector, because
        // it will only have 4 or 5 elements, thus this should be faster than complex containers.
        std::vector< CountPair > counts_map;
        counts_map.reserve( 4 );

        // Add all chars with their counts to the map.
        for( size_t char_idx = 0; char_idx < num_chars; ++char_idx ) {
            auto const char_count = counts.count_at( site_idx, char_idx );
            counts_sum += char_count;
            counts_map.push_back({ char_count, chars[ char_idx ] });
        }

        // We can never have a sum of counts higher then the number of sequences that were added
        // to the counts object.
        assert( counts_sum <= seq_count );

        // We already checked that the counts object is for nucleic acids.
        // Thus, we expect four values here.
        assert( counts_map.size() == 4 );

        // If we want to use gaps as a normal character, add their count to the map.
        // This special case is a bit tricky to handle. We want to compare the gap count with all
        // other frequencies, so it makes sense to just treat it as a normal character here.
        // However, we want to avoid ending up with an invalid ambiguity code like "-AT", so we
        // still need to treat gaps separately (see below).
        auto const gap_count = seq_count - counts_sum;
        if( allow_gaps ) {
            counts_map.push_back({ gap_count, gap_char });
            counts_sum += gap_count;

            // Now that we added gaps, the total sum matches the number of added sequences.
            assert( counts_sum == seq_count );
        }

        // Sort the counts so that the highest one is first.
        std::sort( counts_map.begin(), counts_map.end(), count_pair_compare );

        // Prepare a string of characters codes for the ambiguities.
        std::string ambiguity_codes;

        // Check if this is an all gaps site. If not, we can do the following computation.
        if( counts_map[0].first > 0 ) {

            // Initialize the ambiguity string to the most frequent character code, unless
            // the most frequent one is the gap.
            // This is because if there are other characters that are within the deviation range,
            // we want to prefer them, and not end up with an ill-formed list like "-AT", but just
            // "AT". Only if no other character is as frequent as the gap (and not in the deviation
            // range), we want to use the gap char. If this is the case, the following loop will
            // not add any char to the ambiguity codes, thus it ends up being empty.
            // This is later checked and the gap char is added if needed.
            if( counts_map[0].second != gap_char ) {
                ambiguity_codes = std::string( 1, counts_map[0].second );
            }

            // Every character that has at least this count is added to the ambiguity.
            auto const deviation_threshold = similarity_factor
                                           * static_cast<double>( counts_map[0].first );

            // Compare the less frequent codes to the most frequent one and
            // decide whether to add them to the ambiguities.
            for( size_t i = 1; i < counts_map.size(); ++i ) {
                auto const cur_count = static_cast<double>( counts_map[i].first );

                // If the count is within the deviation range (and not a gap), add it.
                // We also avoid zero counts, as this leads to wrong results with an
                // similarity_factor of 0.0. It would then just add all, ending up with all "N"s,
                // instead of just all codes that appear in the sequence.
                if( cur_count > 0.0 && cur_count >= deviation_threshold ) {
                    if( counts_map[i].second != gap_char ) {
                        ambiguity_codes += counts_map[i].second;
                    }

                // If not, we can stop here. The map is sorted,
                // so no other count will be high enough.
                } else {
                    break;
                }
            }

        } else {

            // If the first entry in the map has zero counts, that means that this is an all-gaps
            // site: The range is sorted, so if the first one is zero, all are. That also implies
            // that allow_gaps is false, because otherwise an all-gaps site would have the highest
            // count in the map for gaps (unless the counts object did not have any sequences added
            // to it...).
            assert( ! allow_gaps || seq_count == 0 );
        }

        // Special case: If the most frequent char is the gap, and no other char is withing the
        // deviation range, we want to output a gap char. We did not add it before, so do it now.
        if( ambiguity_codes.size() == 0 ) {
            ambiguity_codes += gap_char;
        }

        // Get the ambiguity code that represents the selected characters and add it to the seq.
        result += nucleic_acid_ambiguity_code( ambiguity_codes );
    }

    return result;
}

/**
 * @brief Calculate a consensus sequence by using the most frequent characters at each site,
 * for nucleic acid codes `ACGT` and their ambiguities.
 *
 * See @link consensus_sequence_with_ambiguities( SequenceCounts const&, double, bool )
 * consensus_sequence_with_ambiguities(...)@endlink for details.
 * This is merely a wrapper function that takes a SequenceSet instead of a SequenceCounts object.
 */
std::string consensus_sequence_with_ambiguities(
    SequenceSet const&    sequences,
    double                similarity_factor,
    bool                  allow_gaps
) {
    // Basic checks.
    if( sequences.size() == 0 ) {
        throw std::runtime_error( "Cannot caluclate consensus sequence of empty SequenceSet." );
    }
    if( ! is_alignment( sequences ) ) {
        throw std::runtime_error(
            "Cannot caluclate consensus sequence for SequenceSet that is not an alignment. "
            "That is, all Sequences need to have the same length."
        );
    }

    // Build counts object.
    auto counts = SequenceCounts( nucleic_acid_codes_plain(), sequences[0].size() );
    counts.add_sequences( sequences );

    // Return consensus sequence.
    return consensus_sequence_with_ambiguities( counts, similarity_factor, allow_gaps );
}

// =================================================================================================
//     Threshold
// =================================================================================================

/**
 * @brief Calculate a consensus sequence where the character frequency needs to be above a given
 * threshold, for nucleic acid codes `ACGT`.
 *
 * The function calculates a consensus sequence for nucleic acid codes (`ACGT`). It uses the
 * frequency of characters at each site to determine the consensus. The frequency is relative to the
 * total number of counts at that site, thus, it is a value in the range `[ 0.0, 1.0 ]`.
 *
 * If the frequency of a character at a site is above the given `frequency_threshold`, it is used
 * for the consensus. If not, the resulting character depends on `use_ambiguities`.
 * If `use_ambiguities` is set to `true` (default), the sorted frequencies of the characters are
 * added until the threshold is reached, and the ambiguity code for those characters is used.
 * For example, let `frequency_threshold == 0.9`, <code>count('A') == 42</code> and
 * <code>count('T') == 42</code>, and both other counts be 0. Then, neither `A` nor `T` have counts
 * above the threshold, but combined they do, so the result is code `W == AT` at that site.
 * If however `use_ambiguities` is `false`, the mask character `X` is used for sites that are below
 * the threshold.
 *
 * Furthermore, if `allow_gaps` is set to `true` (default), gaps are counted when determining the
 * threshold and checking whether the frequency is above it. That is, gaps are then treated as just
 * another character at the site. For sites that mostly contain gaps and the gap frequency reaches
 * the threshold, a gap is used as consensus.
 * If `allow_gaps` is `false`, however, gaps are not counted for determining the frequency of the
 * other characters. This is similar to the counterpart in consensus_sequence_with_majorities().
 * Solely sites that are gaps-only result in a gap char for the consensus then.
 *
 * For `frequency_threshold < 0.5`, it may happen that more than one character has a frequency
 * above the threshold. In such cases, the most frequent character is used (or, if they have exaclty
 * the same counts, they are used in the order `ACGT`).
 * This is in line with the behaviour of consensus_sequence_with_majorities(). Usually, however,
 * the threshold is above 0.5 anyway, as this gives more meaningful results. If you want to use
 * ambiguity characters for low frequency characters, you can use
 * consensus_sequence_with_ambiguities() instead.
 *
 * An extreme case is a `frequency_threshold` of 1.0. In this case, for sites which only have one
 * character, this one is directly used in the consensus. Sites with multiple different characters
 * result in the ambiguity code of all occuring characters at that site. Thus, the function then
 * behaves similar to consensus_sequence_with_ambiguities() with a `similarity_factor` of 0.0.
 *
 * The other extrem case is a `frequency_threshold` of 0.0. In this case, the function simply uses
 * the most frequent character per site, as it always fulfills the threshold. As said above, if then
 * more than one character has exactly the same frequency, they are used in the order `ACGT`, thus
 * the function then behaves similar to consensus_sequence_with_majorities().
 *
 * Remark: As this function expects nucleic acid codes, the gap character is fixed to '-' here,
 * and the mask character to 'X'.
 * The ambiguity codes are converted using nucleic_acid_ambiguity_code(). See there for more
 * information on the used codes.
 *
 * If the provided SequenceCounts object does not use nucleic acid codes, or if the
 * `frequency_threshold` is not within the range `[ 0.0, 1.0 ]`, an exception is thrown.
 */
std::string consensus_sequence_with_threshold(
    SequenceCounts const& counts,
    double                frequency_threshold,
    bool                  allow_gaps,
    bool                  use_ambiguities
) {
    // Check whether the counts object uses the needed character codes for this function.
    // The characters in the counts object are sorted, so we can directly compare then like this.
    if( counts.characters() != "ACGT" ) {
        throw std::runtime_error(
            "Computation of consensus_sequence_with_ambiguities() only possible "
            "for nucleic acid codes (ACGT)."
        );
    }

    // Check the frequency threshold.
    if( frequency_threshold < 0.0 || frequency_threshold > 1.0 ) {
        throw std::invalid_argument(
            "Value of frequency_threshold has to be in range [ 0.0, 1.0 ]."
        );
    }

    std::string result;
    result.reserve( counts.length() );

    // Use hard coded chars here, as we have fixed character codes anyway.
    auto const gap_char  = '-';
    auto const mask_char = 'X';

    // Prepare some constants for simplicity.
    auto const chars     = counts.characters();
    auto const seq_count = counts.added_sequences_count();
    auto const num_chars = counts.characters().size();

    // Special case: empty counts object. In this case, return an all-gap sequence.
    // We check this here as it prevents some special case treatment later.
    if( seq_count == 0 ) {
        // We do not immediately return the string here, in order to facilitate copy elision.
        result = std::string( counts.length(), gap_char );
        return result;
    }

    // We expect ACGT here.
    assert( num_chars == 4 );

    // Helper struct to store the codes sorted by count.
    using CountPair = std::pair< size_t, char >;
    auto count_pair_compare = []( CountPair const& lhs, CountPair const& rhs ) {
        return lhs.first > rhs.first;
    };

    // Process all sites of the sequence.
    for( size_t site_idx = 0; site_idx < counts.length(); ++site_idx ) {

        // Total sum of counts. Used for getting the number of gaps.
        SequenceCounts::CountsIntType counts_sum = 0;

        // Map from counts to positions. We use this for sorting by count. It's a vector, because
        // it will only have 4 or 5 elements, thus this should be faster than complex containers.
        std::vector< CountPair > counts_map;
        counts_map.reserve( 4 );

        // Add all chars with their counts to the map.
        for( size_t char_idx = 0; char_idx < num_chars; ++char_idx ) {
            auto const char_count = counts.count_at( site_idx, char_idx );
            counts_sum += char_count;
            counts_map.push_back({ char_count, chars[ char_idx ] });
        }

        // Sort the counts so that the highest one is first.
        std::sort( counts_map.begin(), counts_map.end(), count_pair_compare );

        // We can never have a sum of counts higher then the number of sequences that were added
        // to the counts object.
        assert( counts_sum <= seq_count );

        // We already checked that the counts object is for nucleic acids.
        // Thus, we expect four values here.
        assert( counts_map.size() == 4 );

        // If we want to use gaps as a normal character, add their count.
        auto const gap_count = seq_count - counts_sum;
        if( allow_gaps ) {
            counts_sum += gap_count;

            // Now that we added gaps, the total sum matches the number of added sequences.
            assert( counts_sum == seq_count );
        }

        // counts_sum can never be zero here, unless it is an all-gaps site and we didn't use gaps.
        // The other possibilty is if seq_count was zero, which we already checked in the beginning.
        // Thus, either the following division is legal (counts_sum>0), or its result is never
        // used (! allow_gaps).
        assert( counts_sum > 0 || ( ! allow_gaps && gap_count == seq_count ) );

        // If allow gaps and the frequency of gaps reaches the threshold, we use a gap at that site.
        // We only do this if gaps are more frequent than the most frequent other code;
        // otherwise, a threshold of 0.0 would always give a gap, which we do not want.
        // Also, we use a gap if it is a gap-only site.
        auto gap_fraction = static_cast<double>( gap_count ) / static_cast<double>( counts_sum );
        if(
            ( gap_count == seq_count ) ||
            ( allow_gaps && gap_fraction >= frequency_threshold && gap_count > counts_map[0].first )
        ) {

            result += gap_char;

        // Otherwise, we compute the ambiguity code from the other characters.
        } else {

            // Prepare a string of characters codes for the ambiguities.
            std::string ambiguity_codes;

            // Add up the counts and combine ambiguities until we reach the threshold.
            // If we still do not reach the threshold with all codes, we end up with an N.
            size_t accumulated_sum = 0;
            double fraction        = 0.0;
            for( size_t i = 0; i < counts_map.size(); ++i ) {

                // If there are no counts, we do not use it (and stop here, because in a sorted
                // counts order, all following counts will be zero anyway). This way, we only use
                // those codes for the ambiguity that actually appear at the site.
                if( counts_map[i].first == 0 ) {
                    break;
                }

                accumulated_sum += counts_map[i].first;
                ambiguity_codes += counts_map[i].second;

                fraction = static_cast<double>( accumulated_sum ) / static_cast<double>( counts_sum );
                if( fraction >= frequency_threshold ) {
                    break;
                }
            }

            // Finally, add the needed code to the result.
            if( ambiguity_codes.size() > 1 && ! use_ambiguities ) {
                result += mask_char;
            } else {
                result += nucleic_acid_ambiguity_code( ambiguity_codes );
            }
        }
    }

    return result;
}

/**
 * @brief Calculate a consensus sequence where the character frequency needs to be above a given
 * threshold, for nucleic acid codes `ACGT`.
 *
 * See @link consensus_sequence_with_threshold( SequenceCounts const&, double, bool, bool )
 * consensus_sequence_with_ambiguities(...)@endlink for details.
 * This is merely a wrapper function that takes a SequenceSet instead of a SequenceCounts object.
 */
std::string consensus_sequence_with_threshold(
    SequenceSet const&    sequences,
    double                frequency_threshold,
    bool                  allow_gaps,
    bool                  use_ambiguities
) {
    // Basic checks.
    if( sequences.size() == 0 ) {
        throw std::runtime_error( "Cannot caluclate consensus sequence of empty SequenceSet." );
    }
    if( ! is_alignment( sequences ) ) {
        throw std::runtime_error(
            "Cannot caluclate consensus sequence for SequenceSet that is not an alignment. "
            "That is, all Sequences need to have the same length."
        );
    }

    // Build counts object.
    auto counts = SequenceCounts( nucleic_acid_codes_plain(), sequences[0].size() );
    counts.add_sequences( sequences );

    // Return consensus sequence.
    return consensus_sequence_with_threshold(
        counts,
        frequency_threshold,
        allow_gaps,
        use_ambiguities
     );
}

} // namespace sequence
} // namespace genesis
