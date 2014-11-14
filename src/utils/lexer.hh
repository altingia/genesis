#ifndef GNS_UTILS_LEXER_H_
#define GNS_UTILS_LEXER_H_

/**
 * @brief Provides a basic lexer to process a string and split it into tokens.
 *
 * @file
 * @ingroup utils
 */

#include <string>
#include <vector>

namespace genesis {

// =============================================================================
//     General Definitions
// =============================================================================

/** @brief Returns whether a char is a digit (0-9). */
inline bool CharIsDigit (const char c)
{
    return ('0' <= c) && (c <= '9');
}

/** @brief Returns whether a char is a sign (+-). */
inline bool CharIsSign (const char c)
{
    return ('+' == c) || ('-' == c);
}

/** @brief Enum for the different types of LexerToken. */
enum LexerTokenType {
    kError,
    kUnknown,
    kWhite,
    kComment,
    kSymbol,
    kNumber,
    kString,
    kBracket,
    kOperator,
    kTag,
    kEOF
};

/** @brief Converts a LexerTokenType into its string representation. */
std::string LexerTokenTypeToStr (const LexerTokenType t);

// =============================================================================
//     LexerToken
// =============================================================================

/**
 * @brief POD struct that represents a token that is outputted by the Lexer.
 *
 * The main types of tokens are:
 *
 * 1. **Symbol**: A named symbol, usually starts with a letter or underscore,
 *    followed by any number of letters, digits or underscores.
 *
 * 2. **Number**: A number in the format
 *
 *        [+-]123[.456][eE[+-]789].
 *
 * 3. **String**: A literal string, usually enclosed in either 'abc' or "def".
 *
 *    It can contain escape charaters using a backslash, where \\n, \\t and \\r
 *    are translated into their whitespace representation using
 *    StringDeescape() when the option Lexer::use_string_escape is set.
 *
 *    Also, see Lexer::trim_quotation_marks and Lexer::use_string_doubled_quotes
 *
 * 4. **Bracket**: Typically one of these brackets
 *
 *        ( ) [ ] { } < >
 *
 *    This is always a single bracket per token.
 *
 * 5. **Operator**: An operator or sequence of them, typically out of the set
 *
 *        + - * / < > ? ! ^ = % & | , : ;
 *
 * 6. **Tag**: A character denoting a tag, e.g. `<>` for xml. Always a single
 *    tag character per token.
 *
 * Furthermore, there are token types marking whitespaces and comments, which
 * are included when the corresponding options are set in the Lexer class.
 *
 * The distinction between brackets, operators and tags is arbitrary (although
 * operators can appear in sequences like `+=`, while brackets and tags are
 * always contained in a separate token per character). The distinction is meant
 * to help writing better parsers with higher semantic readability.
 *
 * In case of an error while processing the input, an error token is produced
 * which contains the location of the error.
 */
struct LexerToken
{
public:
    /**
     * @brief Constructor that sets the values for this token.
     */
    inline LexerToken
    (
        const LexerTokenType t, const int         l,
        const int            c, const std::string v
    ) :
        type_(t), line_(l), column_(c), value_(v)
    {};

    /**
     * @brief Getter for the LexerTokenType of this token.
     */
    inline LexerTokenType type() const {return type_;};

    /** @brief Getter for the line where this token occured. */
    inline size_t line() const {return line_;};

    /** @brief Getter for the column where this token occured. */
    inline size_t column() const {return column_;};

    /** @brief Getter for the string value of this token. */
    inline std::string value() const {return value_;};

    /** @brief Shortcut that returns "line:column" (e.g., for logging). */
    inline std::string at() const {return std::to_string(line_)+":"+std::to_string(column_);};

    /**
     * @brief Returns whether this token is a given type of bracket.
     *
     * Usage: `token.IsBracket(')')` will return true if this token is
     * of LexerTokenType kBracket and if it is the closing parenthesis.
     */
    inline bool IsBracket(const char c) const
    {
        return type_ == kBracket && value_[0] == c;
    }

    /**
     * @brief Returns whether this token is a given type of operator.
     *
     * Usage: `token.IsOperator('%')` will return true if this token is
     * of LexerTokenType kOperator and if it is the modulo operator.
     */
    inline bool IsOperator(const char c) const
    {
        return type_ == kOperator && value_[0] == c;
    }

    /**
     * @brief Returns whether this token is a given type of tag.
     *
     * Usage: `token.IsTag('<')` will return true if this token is
     * of LexerTokenType kTag and if it is the opening xml tag.
     */
    inline bool IsTag(const char c) const
    {
        return type_ == kTag && value_[0] == c;
    }

    /**
     * @brief Returns the string representation for the LexerTokenType of
     * this token.
     */
    inline std::string TypeToStr() const
    {
        return LexerTokenTypeToStr(type_);
    }

private:
    const LexerTokenType   type_;
    const int              line_;
    const int              column_;
    const std::string      value_;
};

// =============================================================================
//     Lexer
// =============================================================================

/**
 * @brief Basic lexer class that provides an easy way of tokenizing a string.
 *
 * For typical usage of this class, see Process() function.
 *
 * The tokens produced with the Process() method are of type LexerToken (see
 * there for a list of the types of tokens) and can be accessed in various ways:
 *
 *   * Using an iterator, see Lexer::iterator
 *   * Using range based loops, see begin()
 *   * Using index based array access, see operator[]()
 *
 * This class is intended to be a base class that concrete lexers can inherit
 * from in order to get the basic functioning. An instance of this base class is
 * possible, but will only be able to find numbers as well as symbols consisting
 * of consecutive letters. In order to make use of other semantics like
 * comments, strings, operators etc, it has to be derived.
 *
 * When doing so, have a look at Process() to learn about how this class works.
 * Also, see SetCharType() for more information on how to change which characters
 * are interpreted as which type of token.
 */
class Lexer
{
public:
    virtual bool Process (const std::string& text);
    bool ValidateBrackets();
    std::string Dump();

    // =========================================================================
    //     Accessors
    // =========================================================================

    /**
     * @brief Iterator type to access the tokens produces by the lexer.
     *
     * This iterator allows to use a loop like this:
     *
     *     Lexer l;
     *     for (Lexer::iterator t = l.begin(); t != l.end(); ++t) {
     *         std::cout << t->value() << std::endl;
     *     }
     * %
     */
    typedef std::vector<LexerToken>::iterator       iterator;

    /** @brief Const version of the iterator. */
    typedef std::vector<LexerToken>::const_iterator const_iterator;

    /**
     * @brief Returns an iterator to the beginning of the token list.
     *
     * This is used for the iterator and also allows to use range based
     * looping over the tokens:
     *
     *     Lexer l;
     *     for (LexerToken& t : l) {
     *         std::cout << t.value() << std::endl;
     *     }
     * %
     */
    inline iterator begin()
    {
        return tokens_.begin();
    }

    /** @brief Const version of begin(). */
    inline const_iterator begin() const
    {
        return tokens_.begin();
    }

    /** @brief Returns an iterator to the end of the token list. */
    inline iterator end()
    {
        return tokens_.end();
    }

    /** @brief Const version of end(). */
    inline const_iterator end() const
    {
        return tokens_.end();
    }

    /**
     * @brief Provides index based array acces to the tokens.
     *
     * This also allows to iterate over them using:
     *
     *     Lexer l;
     *     for (size_t i = 0; i < l.size(); ++i) {
     *        LexerToken t = l[i];
     *        std::cout << t.value() << std::endl;
     *     }
     *
     * Caveat: this operator does no boundary check. If you need this check,
     * use at() instead.
     */
    inline LexerToken operator[](const std::size_t index) const
    {
        return tokens_[index];
    }

    /**
     * @brief Provides index based array acces to the tokens, doing a
     * boundary check first.
     *
     * In out of bounds cases, a special EOF token is returned.
     */
    inline LexerToken at(const std::size_t index) const
    {
        if (index < tokens_.size()) {
            return tokens_[index];
        } else {
            return LexerToken(LexerTokenType::kEOF, 0, 0, "");
        }
    }

    /**
     * @brief Returns a reference to the first token.
     *
     * Calling this function on an empty() lexer causes undefined behavior.
     */
    inline LexerToken front() const
    {
        return tokens_.front();
    }

    /**
     * @brief Returns a reference to the last token.
     *
     * Calling this function on an empty() lexer causes undefined behavior.
     */
    inline LexerToken back() const
    {
        return tokens_.back();
    }

    /**
     * @brief Returns whether the list of tokens is empty.
     *
     * This is usually the case before Process() was run.
     */
    inline bool empty() const
    {
        return tokens_.empty();
    }

    /**
     * @brief Returns the number of tokens produced during the analysis process.
     */
    inline std::size_t size() const
    {
        return tokens_.size();
    }

    /**
     * @brief Clears all tokens, as if the object was newly created.
     *
     * The options of the lexer are however not changed.
     */
    inline void clear()
    {
        // use swap to make sure vector is of size 0
        std::vector<LexerToken>().swap(tokens_);
    }

    /** @brief Returns whether there appeared an error while lexing. */
    inline bool HasError() const
    {
        return !tokens_.empty() && tokens_.back().type() == kError;
    }

    // =========================================================================
    //     Settings
    // =========================================================================

    /** @brief Determines whether whitespaces are included as tokens. */
    bool include_whitespace = false;

    /** @brief Determines whether comments are included as tokens. */
    bool include_comments = false;

    /*
     * @brief If set, comments are stripped from the text before starting
     * the analysis process.
     *
     * This is useful when the specification of the text allows comments to
     * appear anywhere (e.g. Newick trees). If it is not set, comments are
     * only found at the borders between tokens, but not within them
     * (for example, within a number).
     */
    // TODO make the option strip_comments available
    // one way to do so is to scan including comments, and after the scanning,
    // loop over the result, and merge each comment (and adjacent comments also)
    // with its surrounding tokens (one to the left and one to the right) into
    // one new token (probably mostly with the same type as the left token).
    // e.g.:     some_token comment_a comment_b other_token
    // become:   some_tokenother_token
    //~ bool strip_comments = false;

    /**
     * @brief Determines whether to glue a sign to a number following it.
     *
     * If disabled, a term like `1+2=3` will be parsed into single tokes
     * for each character:
     *
     *     "1" "+" "2" "=" "3"
     *
     * If enabled, signs that preceed a number will be glued to that number,
     * so that a term like `items [1.0, -3.14]` will result in
     *
     *     "items" "[" "1.0" "," "-3.14" "]"
     *
     * This is useful when the input is a list or similar data. As this case
     * is more common in bioinformatics, this is the default.
     */
    bool glue_sign_to_number = true;

    /**
     * @brief Determines whether the quotation marks shall be included
     * when a literal string is found.
     *
     * Strings are usually enclosed in 'abc' or "def", see ScanString for more
     * details on that. The value of trim_quotation_marks determines
     * whether those marks are included in the final token or not.
     * Default is to not include them, which makes preprocessing of the
     * string easier.
     */
    bool trim_quotation_marks = true;

    /**
     * @brief Determines whether to use escape sequences for literal strings.
     *
     * If set to true, and escape sequence starting with a backslash will be
     * treated specially within a literal string: The character after the
     * backslash will be de-escaped using StringDeescape, meaning that for
     * example an escaped quotation mark will not end the string but be included
     * literally in the result. Also, sequences like \\n will turn into a new
     * line and so on.
     *
     * This only affects literal strings, typically enclosed in 'abc' or
     * "def". See ScanString for more.
     */
    bool use_string_escape = false;

    /**
     * @brief If set, doubled quotation marks in a string are considered
     * as normal quotation marks without ending the string.
     *
     * For example, the character sequence (including all quotation marks)
     *
     *     "For learning C++, ""Hello World"" is a good start."
     *
     * will be interpreted as a string containing normal quotation marks
     * around `"Hello World"`.
     *
     * The type of quotation marks used here depends on which chars are set
     * to LexerTokenType kString using SetCharType(). See ScanString() for more.
     */
    bool use_string_doubled_quotes = false;

protected:
    bool ScanFromTo (const char* from, const char* to);
    virtual bool ScanUnknown();
    virtual bool ScanWhitespace();
    virtual bool ScanComment();
    virtual bool ScanSymbol();
    virtual bool ScanNumber();
    virtual bool ScanString();
    virtual bool ScanOperator();
    virtual bool ScanBracket();
    virtual bool ScanTag();

    // =========================================================================
    //     Internal functions
    // =========================================================================

    /** @brief Init the lexer by resetting state and assigning the text. */
    inline void Init (const std::string& text)
    {
        text_ = text.c_str();
        itr_  = 0;
        len_  = text.size();
        line_ = 1;
        tokens_.clear();
    }

    /** @brief Returns the current iterator position while lexing. */
    inline size_t GetPosition() const
    {
        return itr_;
    }

    /**
     * @brief Returns the char at the iterator position plus an offset.
     *
     * If the position is not inside the text, a null char is returned.
     *
     * It can be used for a save version to get the current char by using
     * `GetChar(0);` instead of `GetChar();`.
     */
    inline char GetChar(const int offset) const
    {
        int pos = itr_ + offset;
        if (pos < 0 || static_cast<size_t>(pos) >= len_) {
            return '\0';
        } else {
            return text_[pos];
        }
    }

    /**
     * @brief Returns the char at the current iterator position.
     *
     * Caveat: The function does no boundary check for speed reasons. Thus,
     * it should be used in combination with IsEnd only.
     */
    inline char GetChar() const
    {
        return text_[itr_];
    }

    /**
     * @brief Returns the LexerTokenType of a char.
     *
     * This does not mean that any char of a given type can only appear in
     * tokens of that type. For example, typically a symbol can start with
     * letters, but then contain numbers later, too. Thus, although a digit
     * is of type kNumber, it can end up in a symbol token, depending on the
     * context.
     *
     * For more information on how this char type is used, see Process().
     */
    inline LexerTokenType GetCharType(const char c) const
    {
        // we use char [-128,127] here.
        if (c < 0) {
            return kError;
        } else {
            return start_char_table_[static_cast<unsigned char>(c)];
        }
    }

    /**
     * @brief Returns the token type of the current char.
     *
     * Caveat: The function does not do a boundary check to see if the
     * current char in inside the text. Thus, the function should only be
     * used in combination with IsEnd.
     */
    inline LexerTokenType GetCharType() const
    {
        return GetCharType(GetChar());
    }

    /** @brief Sets the token type for a set of characters.
     *
     * This function takes a token type and a list of characters in form of a
     * string and sets the char type for each of them to the given type.
     * This type will be used by the standard implementation of Process() to
     * determine the correct scanner for a token (see Process() for more on that).
     *
     * If this class is derived, the derived constructor will typically this
     * function in order to set the particular chars needed for the concrete
     * lexer to process its text. For example, it might set "[]" as comment
     * chars and so on.
     *
     * Following chars are particularly interesting to consider:
     *
     *     ! " # $ % & ' ( ) * + , - . / : ; < = > ? @ [ \ ] ^ _ ` { | } ~
     *
     * This is a superset of the C graphical characters and contains all ASCII
     * chars that are on a standard keyboard layout. See start_char_table_
     * for their ASCII representation.
     */
    inline void SetCharType (const LexerTokenType type, std::string chars)
    {
        for (char c : chars) {
            start_char_table_[static_cast<unsigned char>(c)] = type;
        }
    }

    /**
     * @brief Moves the internal iterator to the next char.
     *
     * While doing so, it checks whether also to increase the line counter.
     * This induces some overhead for most parts of the
     * scanning process, because usually it is sufficient to check for
     * the new line conditions only during whitespace scanning, comments
     * or quotes. However, doing it here has two advantages:
     *   * it takes away the responsibility from the scanners,
     *     making them simpler,
     *   * and thus errors in line counting are completely avoided,
     *     making it more reliable
     * %
     */
    inline void NextChar()
    {
        ++itr_;

        // we use the save version of GetChar here, which is equivalant to
        // check for IsEnd. first, CR, then LF. the second condition ensures
        // not to count a CR+LF as two line increases.
        if (GetChar(0) == '\r') {
            ++line_;
        }
        if (GetChar(0) == '\n' && GetChar(-1) != '\r') {
            ++line_;
        }
    }

    /**
     * @brief True if the internal iterator is at the end of the text.
     */
    inline bool IsEnd() const
    {
        return itr_ >= len_;
    }

    /**
     * @brief True if the internal iterator plus some offset is at the
     * end of the text.
     */
    inline bool IsEnd(int offset) const
    {
        return itr_ + offset >= len_;
    }

    /**
     * @brief Extracts a substring of the text betweeen two positions, end
     * excluded.
     */
    inline std::string GetSubstr (size_t start, size_t end) const
    {
        if (start<end) {
            return std::string(text_+start, end-start);
        } else {
            return std::string("");
        }
    }

    void PushToken (const LexerTokenType t, const size_t start, const std::string value);

    /** @brief Create a token and push it to the list. */
    inline void PushToken (const LexerTokenType t, const size_t start, const size_t end)
    {
        PushToken(t, start, GetSubstr(start, end));
    }

private:
    // =========================================================================
    //     Members (make up the state of the object)
    // =========================================================================

    /**
     * @brief This array contains the token types for all chars, in order to
     * determine the correct scanner for the char.
     *
     * See Process() for more on this.
     */
    LexerTokenType start_char_table_[128] = {
        /*      */  kError,     kError,     kError,     kError,
        /*      */  kError,     kError,     kError,     kError,
        /*      */  kError,     kWhite,     kWhite,     kWhite,
        /*      */  kWhite,     kWhite,     kError,     kError,
        /*      */  kError,     kError,     kError,     kError,
        /*      */  kError,     kError,     kError,     kError,
        /*      */  kError,     kError,     kError,     kError,
        /*      */  kError,     kError,     kError,     kError,
        /*  !"# */  kWhite,     kUnknown,   kUnknown,   kUnknown,
        /* $%&' */  kUnknown,   kUnknown,   kUnknown,   kUnknown,
        /* ()*+ */  kUnknown,   kUnknown,   kUnknown,   kUnknown,
        /* ,-./ */  kUnknown,   kUnknown,   kUnknown,   kUnknown,
        /* 0123 */  kNumber,    kNumber,    kNumber,    kNumber,
        /* 4567 */  kNumber,    kNumber,    kNumber,    kNumber,
        /* 89:; */  kNumber,    kNumber,    kUnknown,   kUnknown,
        /* <=>? */  kUnknown,   kUnknown,   kUnknown,   kUnknown,
        /* @ABC */  kUnknown,   kSymbol,    kSymbol,    kSymbol,
        /* DEFG */  kSymbol,    kSymbol,    kSymbol,    kSymbol,
        /* HIJK */  kSymbol,    kSymbol,    kSymbol,    kSymbol,
        /* LMNO */  kSymbol,    kSymbol,    kSymbol,    kSymbol,
        /* PQRS */  kSymbol,    kSymbol,    kSymbol,    kSymbol,
        /* TUVW */  kSymbol,    kSymbol,    kSymbol,    kSymbol,
        /* XYZ[ */  kSymbol,    kSymbol,    kSymbol,    kUnknown,
        /* \]^_ */  kUnknown,   kUnknown,   kUnknown,   kUnknown,
        /* `abc */  kUnknown,   kSymbol,    kSymbol,    kSymbol,
        /* defg */  kSymbol,    kSymbol,    kSymbol,    kSymbol,
        /* hijk */  kSymbol,    kSymbol,    kSymbol,    kSymbol,
        /* lmno */  kSymbol,    kSymbol,    kSymbol,    kSymbol,
        /* pqrs */  kSymbol,    kSymbol,    kSymbol,    kSymbol,
        /* tuvw */  kSymbol,    kSymbol,    kSymbol,    kSymbol,
        /* xyz{ */  kSymbol,    kSymbol,    kSymbol,    kUnknown,
        /* |}~  */  kUnknown,   kUnknown,   kUnknown,   kError
    };

    // Caveat: the following variables are heavily interweaved during a run
    // of Process()! They have to stay consistent, otherwise the resulting
    // tokens will contain wrong information.

    /** @brief The text that is being processed. */
    const char* text_ = "\0";

    /** @brief The current position in the text while processing. */
    size_t      itr_  = 0;

    /** @brief The length of the text being processed. */
    size_t      len_  = 0;

    /** @brief The current line in the text while processing. */
    int         line_ = 1;

    /** @brief The list of tokens resulting from the analysis process. */
    std::vector<LexerToken> tokens_;
};

} // namespace genesis

#endif // include guard
