#pragma once
#include "ict/ict.h"

#include <iostream>
#include <sstream>
#include <string>

namespace xenon {
struct xml_error {
    std::string description;
    int xml_line;
    int xml_column;
};
} // namespace xenon
// macro to throw error with "expected X before Y token" description
#define THROW_EXPECTED(X, Y)                                                   \
    do {                                                                       \
        std::ostringstream out_stream_msg_;                                    \
        out_stream_msg_ << "expected '" << (X) << "' before '" << (Y)          \
                        << "' token";                                          \
        throw xenon::xml_error{out_stream_msg_.str(), line(), column()};       \
    } while (0)

// macro to throw error with "unexpected X token" description
#define THROW_UNEXPECTED(X)                                                    \
    do {                                                                       \
        std::ostringstream out_stream_msg_;                                    \
        out_stream_msg_ << "unexpected '" << X << "' token";                   \
        throw xenon::xml_error{out_stream_msg_.str(), line(), column()};       \
    } while (0)

// macro to throw an xml_exception with description
#define IT_THROW_XML(desc)                                                     \
    do {                                                                       \
        std::ostringstream out_stream_msg_;                                    \
        out_stream_msg_ << desc;                                               \
        throw xenon::xml_error{out_stream_msg_.str(), line(), column()};                    \
    } while (0)

namespace xenon {

/* http://www.w3.org/TR/2006/REC-xml-20060816/ */
class xml_parser_base {
  public:
    xml_parser_base();

    // TODO: write copy constructor and copy assignment
    xml_parser_base(const xml_parser_base &b);

    xml_parser_base &operator=(const xml_parser_base &b);

    virtual ~xml_parser_base();

    /* Prepare a parser object to be re-used.  This is particularly
       valuable when memory allocation overhead is disproportionatly high,
       such as when a large number of small documents need to be parsed.
       The state of the parser is simply reset to the initial state.
    */
    void reset();

    /* Parses some input. Throws std::runtime_error if a fatal error is
       detected.  The last call to parse() must have final true; len may be
       zero for this call.
    */
    void parse(const char *s, size_t len, bool final);

    /* These functions return information about the current parse
       location.  This location refers to the last character parsed.  For
       example, in a startElement() or endElement() handler, the current
       byte location will be referencing the '>' character.
    */
    int line() const { return line_; }
    int column() const { return column_; }
    int byte() const { return byte_; }

    /* Return true if parse() is called after it was called with the final
     * flag set */
    bool finished() const { return finished_; }

    /* Stops parsing, causing parse() to return.
       Must be called from within a handler.  Some handlers may still follow
       because they would otherwise get lost. For example, for empty elements,
       endElement() will still be invoked if stop() is called from
       startElement().

       If a stopped parser is stopped again, an InvalidState Exception.h will be
       thrown.

       When suspended, parsing can be resumed by calling resume(). resume() can
       possibly throw an InvalidStated exception.
    */
    void stop();
    void resume();

  protected:
    // handlers
    virtual void startElement(const char *s, const char **atts) = 0;
    virtual void endElement(const char *s) = 0;
    virtual void characterData(const char *s) = 0;
    virtual void comment(const char *s) = 0;
    virtual void proc_inst(const char *){}

    enum { MaxAtts = 50 };
    enum { MaxCDataSize = 1024 * 10 };
    enum { MaxCommentSize = 1024 * 10 };
    enum { MaxTagDepth = 100 };
    enum { MaxTagSize = 1024 };
    enum { MaxAttSize = 1024 * 10 };

    enum State {
        First,      // looking for S or first '<'
        StateStart, // parsing cdata, looking for '<' or '>'
        Start1,     // found '<', looking for [!?/] or tag start char

        CommentStart1, // "<!"
        CommentStart2, // "<!-"

        CommentStart3, // "<!--"

        CommentBody, // (parsing comment string)
        CommentEnd1, // "-"
        CommentEnd2, // "--"

        PI, // looking for '?'

        PIEnd1, // looking for '>'

        // STag ::= '<' Name (S Attribute)* S? '>'

        StartTag1, // (parsing tag Name)

        FirstAtt1, // looking for first Attribute start char or '>' or S
        FirstAtt2, // found S, looking for first Attribute start char or S

        AttName1,  // looking for subsequent Attribute start char or '>' or S
        AttName2,  // found S, looking for subsequent Attribute start char
                   // or S or '>'
        AttName2a, // found S, looking for subsequent Attribute start char or S

        AttName3, // reading attribute name, looking for '=' or S
        AttName4, // looking for '=' or S

        AttValue1, // looking for starting '"'
        AttValue2, // looking for att value start char
        AttValue3, // reading att value, looking for ending '"'

        Empty, // / (just read '/' and looking for '>')

        // ETag ::=  '</' Name S? '>'
        EndTag1, // looking for end tag Name start char
        EndTag2, // parsing end tag Name
        EndTag3, // looking for '>'

        Epilog, // finished reading end root element tag
        Epilog1
    };

  protected:
    void xparse(const char *s, size_t len, bool final);

  private:
    inline bool isNameStartChar(int c) {
        return (isalpha(c) || (c == '_') || (c == ':'));
    }

    inline bool isNameChar(int c) {
        return (isalnum(c) || (c == '.') || (c == '-') || (c == '_') ||
                (c == ':'));
    }

    inline bool isAttValueChar(int c) { return (!((c == '^') || (c == '"'))); }

    inline void nextAtt() {
        ac_++;
        ai_++;
        a_ = *ai_;
    }
    inline void resetAtts() {
        ac_ = 0;
        ai_ = atts_;
        a_ = *ai_;
        *a_ = '\0';
    }
    inline void lastAtt() {
        ac_++;
        ai_++;
        *(*ai_) = '\0';
    }

    inline void resetTags() {
        tc_ = 0;
        ti_ = tags_;
        t_ = *ti_;
        *t_ = '\0';
    }
    inline void pushTag() {
        tc_++;
        ti_++;
        t_ = *ti_;
    }
    inline void popTag() {
        tc_--;
        ti_--;
        t_ = *ti_;
    }

    // handle &lt; &gt; &amp; &quot; &apos;
    enum CharState {
        Idle,
        Entity, // found '&', looking for ';' reading entity reference

        LT1,  // "l"
        GT1,  // "g"
        AMP1, // "a"

        LT2,  // "lt"
        GT2,  // "gt"
        AMP2, // "am"
        AMP3, // "amp"

        QUOT1, // q
        QUOT2, // qu
        QUOT3, // quo
        QUOT4, // quot

        APOS2, // ap
        APOS3, // apo
        APOS4, // apos
    };

    /* This will work with a char * or an iterator */
    template <typename T> T addChar(CharState *s, T d) {
        switch (*s) {
        case Idle:
            switch (*ch_) {
            case '&':
                *s = Entity;
                break;
            default: { *d++ = *ch_; }
            }
            break;
        case Entity:
            switch (*ch_) {
            case 'l':
                *s = LT1;
                break;
            case 'g':
                *s = GT1;
                break;
            case 'a':
                *s = AMP1;
                break;
            case 'q':
                *s = QUOT1;
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;

        // lt;
        case LT1:
            switch (*ch_) {
            case 't':
                *s = LT2;
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;
        case LT2:
            switch (*ch_) {
            case ';':
                *s = Idle;
                *d++ = '<';
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;

        // gt;
        case GT1:
            switch (*ch_) {
            case 't':
                *s = GT2;
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;
        case GT2:
            switch (*ch_) {
            case ';':
                *s = Idle;
                *d++ = '>';
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;

        // amp;
        case AMP1:
            switch (*ch_) {
            case 'm':
                *s = AMP2;
                break;
            case 'p':
                *s = APOS2;
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;
        case AMP2:
            switch (*ch_) {
            case 'p':
                *s = AMP3;
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;
        case AMP3:
            switch (*ch_) {
            case ';':
                *s = Idle;
                *d++ = '&';
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;

        // quot
        case QUOT1:
            switch (*ch_) {
            case 'u':
                *s = QUOT2;
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;
        case QUOT2:
            switch (*ch_) {
            case 'o':
                *s = QUOT3;
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;
        case QUOT3:
            switch (*ch_) {
            case 't':
                *s = QUOT4;
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;
        case QUOT4:
            switch (*ch_) {
            case ';':
                *s = Idle;
                *d++ = '"';
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;

        // apos
        case APOS2:
            switch (*ch_) {
            case 'o':
                *s = APOS3;
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;
        case APOS3:
            switch (*ch_) {
            case 's':
                *s = APOS4;
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;
        case APOS4:
            switch (*ch_) {
            case ';':
                *s = Idle;
                *d++ = '\'';
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;
        }
        return d;
    }

    inline void set(State s) { state_ = s; }
    inline State state() { return state_; }

    const char *ch_; // current char

    std::vector<char> cdata_;
    std::vector<char>::iterator d_;

    std::vector<char> comment_;
    std::vector<char>::iterator c_;

    std::vector<char> proc__inst;
    std::vector<char>::iterator pi_;

    char **tags_; // tag stack
    char **ti_;   // tag iterator
    char *t_;
    int tc_; // tag depth count (for debugging)

    char *e_; // end tag char pointer

    char **atts_; // attribute list and values
    char **ai_;   // atribute iterator
    char *a_;
    int ac_; // att count (for debugging)

    // int error_;
    int line_;
    int column_;
    int byte_;
    bool done_;

    State state_;
    CharState cd__state;
    CharState att__state;

    bool finished_;
    bool suspended_;
};

inline void xml_parser_base::xparse(const char *s, size_t len, bool final) {
    if (finished_)
        IT_PANIC("finished parsing");
    if (suspended_)
        IT_PANIC("parsing suspended");
    if (!len)
        return;
    if (final)
        finished_ = true;

    for (ch_ = s; ch_ != (s + len); ++ch_) {
        ++byte_;
        if (*ch_ == '\n') {
            ++line_;
            column_ = 0;
        } else {
            ++column_;
        }

        switch (state()) {
        case First: // looking for first tag
            switch (*ch_) {
            case ' ':
            case '\n':
            case '\r':
            case '\t':
                break;
            case '<': // starting a tag
                set(Start1);
                break;
            default:
                THROW_EXPECTED("whitespace or '<'", *ch_);
            }
            break;
        case StateStart: // expect a start tag
            switch (*ch_) {
            case '<': // starting a tag
                // if there is cdata, then call handler
                if (d_ != cdata_.begin()) {
                    *d_ = '\0';
                    characterData(&cdata_[0]);
                    d_ = cdata_.begin();
                }
                set(Start1);
                break;
            case '>':
                THROW_UNEXPECTED(*ch_);
            default: // just cdata
                d_ = addChar(&cd__state, d_);
            }
            break;
        case Start1:
            switch (*ch_) {
            case '!':
                set(CommentStart1);
                break;
            case '?':
                set(PI);
                break;
            case '/':
                set(EndTag1);
                break;
            default: // must be a start of an element name
                if (isNameStartChar(*ch_)) {
                    pushTag();
                    *t_++ = *ch_;
                    set(StartTag1);
                } else
                    THROW_EXPECTED("start char", *ch_);
            }
            break;
        case StartTag1:
            if (isNameChar(*ch_))
                *t_++ = *ch_;
            else
                switch (*ch_) {
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    *t_ = '\0';
                    t_ = *ti_; // t_ now holds the tag name
                    set(FirstAtt1);
                    break;
                case '/': // we have an empty element with no attributes
                    *t_ = '\0';
                    t_ = *ti_; // t_ now holds the tag name
                    resetAtts();
                    set(Empty);
                    break;
                case '>': // done, call handler with no attributes
                    *t_ = '\0';
                    t_ = *ti_; // t_ now holds the tag name
                    resetAtts();
                    startElement(t_, const_cast<const char **>(atts_));
                    set(StateStart);
                    break;
                default:
                    THROW_UNEXPECTED(*ch_);
                }
            break;
        case FirstAtt1:
            if (isNameStartChar(*ch_)) {
                resetAtts();
                a_ = addChar(&att__state, a_);
                set(AttName3);
            } else
                switch (*ch_) {
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    set(FirstAtt2);
                    break;
                case '>': // (same as above case)
                    resetAtts();
                    startElement(t_, const_cast<const char **>(atts_));
                    set(StateStart);
                    break;
                default:
                    THROW_UNEXPECTED(*ch_);
                }
            break;
        case FirstAtt2:
            if (isNameStartChar(*ch_)) {
                resetAtts();
                a_ = addChar(&att__state, a_);
                set(AttName3);
            } else
                switch (*ch_) {
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    break;
                default:
                    THROW_UNEXPECTED(*ch_);
                }
            break;
        case AttName1:
            if (isNameStartChar(*ch_)) {
                nextAtt();
                a_ = addChar(&att__state, a_);
                set(AttName3);
            } else
                switch (*ch_) {
                case '/':
                    set(Empty);
                    break;
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    set(AttName2);
                    break;
                case '>': // done, call handler with attributes
                    lastAtt();
                    startElement(t_, const_cast<const char **>(atts_));
                    resetAtts();
                    set(StateStart);
                    break;
                default:
                    THROW_UNEXPECTED(*ch_);
                }
            break;
        case AttName2:
            if (isNameStartChar(*ch_)) {
                nextAtt();
                a_ = addChar(&att__state, a_);
                set(AttName3);
            } else
                switch (*ch_) {
                case '/':
                    set(Empty);
                    break;
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    set(AttName2a);
                    break;
                case '>': // done, call handler with attributes
                    lastAtt();
                    startElement(t_, const_cast<const char **>(atts_));
                    resetAtts();
                    set(StateStart);
                    break;
                default:
                    THROW_UNEXPECTED(*ch_);
                }
            break;
        case AttName2a:
            if (isNameStartChar(*ch_)) {
                nextAtt();
                a_ = addChar(&att__state, a_);
                set(AttName3);
            } else
                switch (*ch_) {
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    break;
                default:
                    THROW_UNEXPECTED(*ch_);
                }
            break;
        case AttName3:
            if (isNameChar(*ch_)) {
                a_ = addChar(&att__state, a_);
            } else
                switch (*ch_) {
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    *a_ = '\0'; // a_ now holds the tag name
                    set(AttName4);
                    break;
                case '=':
                    *a_ = '\0'; // a_ now holds the tag name
                    set(AttValue1);
                    break;
                default:
                    THROW_UNEXPECTED(*ch_);
                }
            break;
        case AttName4:
            switch (*ch_) {
            case ' ':
            case '\n':
            case '\r':
            case '\t':
                break;
            case '=':
                set(AttValue1);
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;
        case AttValue1:
            switch (*ch_) {
            case ' ':
            case '\n':
            case '\r':
            case '\t':
                break;
            case '"':
                set(AttValue2);
                break;
            default:
                THROW_UNEXPECTED(*ch_);
            }
            break;
        case AttValue2:
            if (isAttValueChar(*ch_)) {
                nextAtt();
                a_ = addChar(&att__state, a_);
                set(AttValue3);
            } else
                THROW_UNEXPECTED(*ch_);
            break;
        case AttValue3:
            if (isAttValueChar(*ch_)) {
                a_ = addChar(&att__state, a_);
            } else
                switch (*ch_) {
                case '"':
                    *a_ = '\0';
                    set(AttName1);
                    break;
                default:
                    THROW_UNEXPECTED(*ch_);
                }
            break;
        case Empty:
            switch (*ch_) {
            case '>':
                lastAtt();
                startElement(t_, const_cast<const char **>(atts_));
                endElement(t_);
                if (ti_ == (tags_ + 1))
                    set(Epilog);
                else {
                    popTag();
                    set(StateStart);
                }
                break;
            default:
                THROW_EXPECTED('>', *ch_);
            }
            break;
        case EndTag1:
            if (*ch_ == *t_) {
                e_ = (t_ + 1);
                set(EndTag2);
            } else
                THROW_EXPECTED(*t_, *ch_);
            break;
        case EndTag2:
            if (*ch_ != *e_) {
                switch (*ch_) {
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    if (*e_ == '\0')
                        set(EndTag3);
                    else
                        IT_PANIC("tag mismatch");
                    break;
                case '>':
                    if (*e_ == '\0') {
                        endElement(t_);
                        if (ti_ == (tags_ + 1)) {
                            set(Epilog);
                        } else {
                            popTag();
                            set(StateStart);
                        }
                    } else
                        THROW_EXPECTED("\\0", *e_);
                    break;
                default:
                    THROW_EXPECTED(*e_, *ch_);
                }
            } else
                e_++;
            break;
        case EndTag3:
            switch (*ch_) {
            case '>':
                endElement(t_);
                if (ti_ == (tags_ + 1))
                    set(Epilog);
                else {
                    popTag();
                    set(StateStart);
                }
                break;
            default:
                THROW_EXPECTED('>', *ch_);
            }
            break;
        case CommentStart1:
            if (*ch_ == '-')
                set(CommentStart2);
            else
                THROW_EXPECTED('-', *ch_);
            break;
        case CommentStart2:
            if (*ch_ == '-')
                set(CommentStart3);
            else
                THROW_EXPECTED('-', *ch_);
            break;
        case CommentStart3:
            if (*ch_ == '-')
                IT_PANIC("\"<!---\" is invalid XML");
            *c_++ = *ch_;
            set(CommentBody);
            break;
        case CommentBody:
            *c_++ = *ch_;
            if (*ch_ == '-')
                set(CommentEnd1);
            break;
        case CommentEnd1:
            *c_++ = *ch_;
            if (*ch_ == '-')
                set(CommentEnd2);
            else
                set(CommentBody);
            break;
        case CommentEnd2:
            *c_++ = *ch_;
            if (*ch_ == '>') {
                c_ -= 3;
                *c_ = '\0';
                comment(&comment_[0]);
                c_ = comment_.begin();
                if (done_)
                    set(Epilog);
                else
                    set(StateStart); // we're done with comment
            } else
                IT_PANIC("\"--\" cannot occur within a comment");
            break;
        case PI:
            *pi_++ = *ch_;
            if (*ch_ == '?')
                set(PIEnd1);
            break;
        case PIEnd1:
            if (*ch_ == '>') {
                if (done_)
                    set(Epilog);
                else
                    set(StateStart);
                --pi_;
                *pi_ = '\0';
                proc_inst(&proc__inst[0]);
                pi_ = proc__inst.begin();
            } else
                THROW_EXPECTED('>', *ch_);
            break;
        case Epilog: // only comments or PIs allowed
            done_ = true;
            switch (*ch_) {
            case ' ':
            case '\n':
            case '\r':
            case '\t':
                break;
            case '<':
                set(Epilog1);
                break;
            default:
                IT_THROW_XML("junk after doc element");
            }
            break;
        case Epilog1:
            switch (*ch_) {
            case '!':
                set(CommentStart1);
                break;
            case '?':
                set(PI);
                break;
            default:
                IT_THROW_XML("junk after doc element");
            }
            break;
        }
    }

    if (finished_ && (state() != Epilog))
        IT_PANIC("parsing incomplete");

    return;
}

// See http://www.w3.org/TR/REC-xml/REC-xml-20040204.xml for implementation
// details.
inline void xml_parser_base::parse(const char *s, size_t len, bool final) {
    try {
        xparse(s, len, final);
    } catch (xenon::xml_error &e) {
        IT_FATAL(e.description << " in [" << e.xml_line << ':' << e.xml_column
                               << ']');
    }
}

} // namespace xenon
