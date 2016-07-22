#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/ict.h>

#include <string>
#include <iostream>
#include <sstream>

namespace xenon {
    struct xml_error {
        std::string description;
        int xml_line;
        int xml_column;
    };
}
// macro to throw error with "expected X before Y token" description 
#define THROW_EXPECTED(X, Y) \
do { \
    std::ostringstream os; \
    os << "expected '" << (X) << "' before '" << (Y) << "' token"; \
    throw xenon::xml_error{os.str(), line(), column()}; \
} while (0);

// macro to throw error with "unexpected X token" description 
#define THROW_UNEXPECTED(X) \
do { \
    std::ostringstream os; \
    os << "unexpected '" << X << "' token"; \
    throw xenon::xml_error{os.str(), line(), column()}; \
} while (0);

// macro to throw an xml_exception with description
#define IT_THROW_XML(desc) \
do { \
    std::ostringstream os; \
    os << desc; \
    throw xenon::xml_error{os.str(), line(), column() }; \
} while (0)

namespace xenon {

/* http://www.w3.org/TR/2006/REC-xml-20060816/ */
class xml_parser_base
{
    public:

    xml_parser_base();

    // TODO: write copy constructor and copy assignment
    
    xml_parser_base(const xml_parser_base & b);

    xml_parser_base & operator=(const xml_parser_base & b);

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
    void parse(const char * s, int len, bool final);  

    /* These functions return information about the current parse
       location.  This location refers to the last character parsed.  For
       example, in a startElement() or endElement() handler, the current
       byte location will be referencing the '>' character.
    */
    int line() const { return _line; }
    int column() const { return _column; }
    int byte() const { return _byte; }

    /* Return true if parse() is called after it was called with the final
     * flag set */
    bool finished() const { return _finished; }

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
    virtual void startElement(const char * s, const char ** atts) = 0;
    virtual void endElement(const char * s) = 0;
    virtual void characterData(const char * s) = 0;
    virtual void comment(const char * s) = 0;
    virtual void proc_inst(const char *) {};

    enum { MaxAtts = 50 };
    enum { MaxCDataSize = 1024 * 10 };
    enum { MaxCommentSize = 1024 * 10 };
    enum { MaxTagDepth = 100 };
    enum { MaxTagSize = 1024 };
    enum { MaxAttSize = 1024 * 10 };
        
    // update the state_names array in the source file if you change this
    enum State 
    {
        First, // looking for S or first '<'
        StateStart, // parsing cdata, looking for '<' or '>'
        Start1, // found '<', looking for [!?/] or tag start char

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

        AttName1, // looking for subsequent Attribute start char or '>' or S
        AttName2, // found S, looking for subsequent Attribute start char
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
    void xparse(const char * s, int len, bool final);

    private:

    inline bool isNameStartChar(int c)
    {
        return (isalpha(c) || (c=='_') || (c == ':'));
    }

    inline bool isNameChar(int c)
    {
        return (isalnum(c)||(c=='.')||(c=='-')||(c=='_')||(c==':'));
    }

    inline bool isAttValueChar(int c)
    {
        return (!((c=='^') || (c=='"')));
    }

    inline void nextAtt() { _ac++; _ai++; _a = *_ai; }
    inline void resetAtts() { _ac=0; _ai = _atts; _a = *_ai; *_a = '\0'; }
    inline void lastAtt() { _ac++; _ai++; *(*_ai) = '\0'; }

    inline void resetTags() { _tc=0; _ti = _tags; _t = *_ti; *_t = '\0'; }
    inline void pushTag() { _tc++; _ti++; _t = *_ti; }
    inline void popTag() { _tc--; _ti-- ; _t = *_ti; }

    // handle &lt; &gt; &amp; &quot; &apos;
    enum CharState {
        Idle,
        Entity, // found '&', looking for ';' reading entity reference

        LT1, // "l"
        GT1, // "g"
        AMP1, // "a"

        LT2, // "lt"
        GT2, // "gt"
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
    template <typename T>
    T addChar(CharState *s, T d)
    {
        switch (*s)
        {
            case Idle:
                switch (*_ch) {
                    case '&' : *s = Entity; break;
                    default: 
                    {
                        *d++=*_ch;
                    }
                }
                break;
            case Entity:
                switch (*_ch) {
                    case 'l' : *s = LT1; break;
                    case 'g' : *s = GT1; break;
                    case 'a' : *s = AMP1; break;
                    case 'q' : *s = QUOT1; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;

            // lt;
            case LT1:
                switch (*_ch) {
                    case 't' : *s = LT2; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;
            case LT2:
                switch (*_ch) {
                    case ';' : *s = Idle; *d++ = '<'; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;

            // gt;
            case GT1:
                switch (*_ch) {
                    case 't' : *s = GT2; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;
            case GT2:
                switch (*_ch) {
                    case ';' : *s = Idle; *d++ = '>'; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;

            // amp;
            case AMP1:
                switch (*_ch) {
                    case 'm' : *s = AMP2; break;
                    case 'p' : *s = APOS2; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;
            case AMP2:
                switch (*_ch) {
                    case 'p' : *s = AMP3; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;
            case AMP3:
                switch (*_ch) {
                    case ';' : *s = Idle; *d++ = '&'; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;

            // quot
            case QUOT1:
                switch (*_ch) {
                    case 'u' : *s = QUOT2; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;
            case QUOT2:
                switch (*_ch) {
                    case 'o' : *s = QUOT3; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;
            case QUOT3:
                switch (*_ch) {
                    case 't' : *s = QUOT4; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;
            case QUOT4:
                switch (*_ch) {
                    case ';' : *s = Idle; *d++ = '"'; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;
            
            // apos
            case APOS2:
                switch (*_ch) {
                    case 'o' : *s = APOS3; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;
            case APOS3:
                switch (*_ch) {
                    case 's' : *s = APOS4; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;
            case APOS4:
                switch (*_ch) {
                    case ';' : *s = Idle; *d++ = '\''; break;
                    default : THROW_UNEXPECTED(*_ch);
                }
                break;

            default:
                THROW_UNEXPECTED(*_ch);
        }
        return d;
    }

    inline void set(State s) { _state = s; }
    inline State state() { return _state; }

    const char *_ch;  // current char

    std::vector<char> _cdata;
    std::vector<char>::iterator _d;

    std::vector<char> _comment;
    std::vector<char>::iterator _c;

    std::vector<char> _proc_inst;
    std::vector<char>::iterator _pi;

    char ** _tags; // tag stack
    char ** _ti; // tag iterator
    char * _t;
    int _tc; // tag depth count (for debugging)

    char * _e; // end tag char pointer

    char ** _atts; // attribute list and values
    char ** _ai; // atribute iterator
    char *_a; 
    int _ac; // att count (for debugging)

    //int _error;
    int _line;
    int _column;
    int _byte;
    bool _done;

    State _state;
    CharState _cd_state;
    CharState _att_state;

    bool _finished;
    bool _suspended;
};

}
