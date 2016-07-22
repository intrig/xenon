//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.

#include <xenon/xml_parser_base.h>

#include <ict/ict.h>

namespace xenon {
// order matters here, index is state()
const char * state_names[] = {
        "First",
        "StateStart",
        "Start1",
        "CommentStart1",
        "CommentStart2",
        "CommentStart3",
        "CommentBody",
        "CommentEnd1",
        "CommentEnd2",
        "PI",
        "PIEnd1",
        "StartTag1",
        "FirstAtt1",
        "FirstAtt2",
        "AttName1",
        "AttName2",
        "AttName2a",
        "AttName3",
        "AttName4",
        "AttValue1",
        "AttValue2",
        "AttValue3",
        "Empty",
        "EndTag1",
        "EndTag2",
        "EndTag3",
        "Epilog",
        "Epilog1"
    };

xml_parser_base::xml_parser_base()
{
    int i;
    // init once stuff
    _atts = new char * [MaxAtts];
    if (!_atts) { IT_PANIC("out of memory"); }
    for (i=0; i<MaxAtts; ++i) 
    {
        _atts[i] = new char [MaxAttSize];
        if (!_atts[i]) IT_PANIC("out of memory:" << i);
    }
    
    _tags = new char * [MaxTagDepth];
    if (!_tags) IT_PANIC("out of memory");
    for (i=0; i<MaxTagDepth; ++i) 
    {
        _tags[i] = new char [MaxTagSize];
        if (!_tags[i]) IT_PANIC("out of memory:" << i);
    }
    
    reset();
}

// this doen't make sense since its pretty much impossible to copy the state of the parser right now
// but at least we won't get any bad deallocs
xml_parser_base::xml_parser_base(const xml_parser_base &) : xml_parser_base() {
}

xml_parser_base & xml_parser_base::operator=(const xml_parser_base &) {
    reset();
    return *this;
}

xml_parser_base::~xml_parser_base()
{
    int i;
    for (i=0; i<MaxAtts; ++i) 
    {
        delete [] _atts[i];
    }
    delete [] _atts;
    
    for (i=0; i<MaxTagDepth; ++i) 
    {
        delete [] _tags[i];
    }
    delete [] _tags;
    
}

void xml_parser_base::stop()
{
    if (_finished) IT_PANIC("cannot stop a finished parser");
    if (_suspended) IT_PANIC("cannot suspend a suspended parser");
    _suspended = true;
}

void xml_parser_base::resume()
{
    if (_finished) IT_PANIC("cannot resume a finished parser");
    if (!_suspended) IT_PANIC("cannot resume a running parser");
    _suspended = false;
}

void xml_parser_base::reset()
{
    _done = false;
    _finished = false;

    _line=1;
    _column=0;
    _byte=0;

    _cdata.assign(MaxCDataSize, '\0');
    _d = _cdata.begin();

    _comment.assign(MaxCommentSize, '\0');
    _c = _comment.begin();
    
    _proc_inst.assign(MaxCommentSize, '\0');
    _pi = _proc_inst.begin();

    resetTags();
    resetAtts();

    _state = First;
    _cd_state = Idle;
    _att_state = Idle;

    _suspended = false;
}

// See http://www.w3.org/TR/REC-xml/REC-xml-20040204.xml for implementation
// details.
void xml_parser_base::parse(const char * s, int len, bool final) {
    try {
        xparse(s, len, final);
    } catch (xenon::xml_error & e) {
        IT_FATAL(e.description << " in [" << e.xml_line << ':' << e.xml_column << ']'); 
    }
}

void xml_parser_base::xparse(const char * s, int len, bool final) {
    if (_finished) IT_PANIC("finished parsing");
    if (_suspended) IT_PANIC("parsing suspended");
    if (!len) return;
    if (final) _finished = true;

    for (_ch = s; _ch != (s + len); ++_ch)
    {
        ++_byte;
        if (*_ch == '\n')
        {
            ++_line;
            _column = 0;
        } else {
            ++_column;
        }

        switch (state())
        {
            case First: // looking for first tag
                switch (*_ch) {
                    case ' ': case '\n': case '\r': case '\t': break;
                    case '<' : // starting a tag
                        set(Start1); break;
                    default :
                        THROW_EXPECTED("whitespace or '<'", *_ch);
                }
                break;
            case StateStart: // expect a start tag
                switch (*_ch) {
                    case '<' : // starting a tag
                        // if there is cdata, then call handler
                        if (_d != _cdata.begin())
                        {
                            *_d = '\0';
                            characterData(&_cdata[0]);
                            _d = _cdata.begin();
                        }
                        set(Start1); break;
                    case '>' : THROW_UNEXPECTED(*_ch);
                    default :  // just cdata
                        _d = addChar(&_cd_state, _d);
                }
                break;
            case Start1 :
                switch (*_ch) {
                    case '!' : set(CommentStart1); break;
                    case '?' : set(PI); break;
                    case '/' : 
                        set(EndTag1); break;
                    default: // must be a start of an element name
                        if (isNameStartChar(*_ch))
                        {    
                            pushTag();
                            *_t++ = *_ch;
                            set(StartTag1);
                        } else THROW_EXPECTED("start char", *_ch);
                }
                break;
            case StartTag1 :
                if (isNameChar(*_ch)) *_t++ = *_ch;
                else switch (*_ch) {
                    case ' ': case '\n': case '\r': case '\t':
                        *_t = '\0';
                        _t = *_ti; // _t now holds the tag name
                        set(FirstAtt1);
                        break;
                    case '/':  // we have an empty element with no attributes
                        *_t = '\0';
                        _t = *_ti; // _t now holds the tag name
                        resetAtts();
                        set(Empty);
                        break;
                    case '>':  // done, call handler with no attributes
                        *_t = '\0';
                        _t = *_ti; // _t now holds the tag name
                        resetAtts();
                        startElement(_t, (const char **) _atts);
                        set(StateStart);
                        break;
                    default:
                        THROW_UNEXPECTED(*_ch);
                }
                break;
            case FirstAtt1 : 
                if (isNameStartChar(*_ch)) 
                {
                    resetAtts();
                    _a = addChar(&_att_state, _a);
                    set(AttName3);
                } else switch (*_ch) {
                    case ' ': case '\n': case '\r': case '\t': 
                        set(FirstAtt2);
                        break;
                    case '>':  // (same as above case)
                        resetAtts();
                        startElement(_t, (const char **) _atts);
                        set(StateStart);
                        break;
                    default: 
                        THROW_UNEXPECTED(*_ch);
                }
                break;
            case FirstAtt2 : 
                if (isNameStartChar(*_ch)) 
                {
                    resetAtts();
                    _a = addChar(&_att_state, _a);
                    set(AttName3);
                } else switch (*_ch) {
                    case ' ': case '\n': case '\r': case '\t': break;
                    default: 
                        THROW_UNEXPECTED(*_ch);
                }
                break;
            case AttName1 :
                if (isNameStartChar(*_ch)) 
                {
                    nextAtt();
                    _a = addChar(&_att_state, _a);
                    set(AttName3);
                } 
                else switch (*_ch) {
                    case '/' : set(Empty); break;
                    case ' ': case '\n': case '\r': case '\t': 
                        set(AttName2);
                        break;
                    case '>':  // done, call handler with attributes
                        lastAtt();
                        startElement(_t, (const char **) _atts);
                        resetAtts();
                        set(StateStart);
                        break;
                    default : 
                        THROW_UNEXPECTED(*_ch);
                }
                break;
            case AttName2 :
                if (isNameStartChar(*_ch)) 
                {
                    nextAtt();
                    _a = addChar(&_att_state, _a);
                    set(AttName3);
                } else switch (*_ch) {
                    case '/' : set(Empty); break;
                    case ' ': case '\n': case '\r': case '\t': 
                        set(AttName2a);
                        break;
                    case '>':  // done, call handler with attributes
                        lastAtt();
                        startElement(_t, (const char **) _atts);
                        resetAtts();
                        set(StateStart);
                        break;
                    default:
                        THROW_UNEXPECTED(*_ch);
                }
                break;
            case AttName2a :
                if (isNameStartChar(*_ch)) 
                {
                    nextAtt();
                    _a = addChar(&_att_state, _a);
                    set(AttName3);
                } else switch (*_ch) {
                    case ' ': case '\n': case '\r': case '\t':  
                        break;
                    default:
                        THROW_UNEXPECTED(*_ch);
                }
                break;
            case AttName3 :
                if (isNameChar(*_ch)) {
                    _a = addChar(&_att_state, _a);
                }
                else switch (*_ch) {
                    case ' ': case '\n': case '\r': case '\t':
                        *_a = '\0'; // _a now holds the tag name
                        set(AttName4);
                        break;
                    case '=': 
                        *_a = '\0'; // _a now holds the tag name
                        set(AttValue1);
                        break;
                    default:
                        THROW_UNEXPECTED(*_ch);
                }
                break;
            case AttName4 :
                switch (*_ch) {
                    case ' ': case '\n': case '\r': case '\t':
                        break;
                    case '=': 
                        set(AttValue1);
                        break;
                    default:
                        THROW_UNEXPECTED(*_ch);
                }
                break;
            case AttValue1 : 
                switch (*_ch) {
                    case ' ': case '\n': case '\r': case '\t': break;
                    case '"' : set(AttValue2); break;
                    default:
                        THROW_UNEXPECTED(*_ch);
                }
                break;
            case AttValue2 :
                if (isAttValueChar(*_ch)) 
                {
                    nextAtt();
                    _a = addChar(&_att_state, _a);
                    set(AttValue3);
                } else 
                    THROW_UNEXPECTED(*_ch);
                break;
            case AttValue3 : 
                if (isAttValueChar(*_ch)) 
                {
                    _a = addChar(&_att_state, _a);
                }
                else switch (*_ch) {
                    case '"':
                        *_a = '\0';
                        set(AttName1);
                        break;
                    default:
                        THROW_UNEXPECTED(*_ch);
                }
                break;
            case Empty :
                switch (*_ch) {
                    case '>' :
                        lastAtt();
                        startElement(_t, (const char **) _atts);
                        endElement(_t);
                        if (_ti==(_tags+1)) set(Epilog);
                        else {
                            popTag();
                            set(StateStart);
                        }
                        break;
                    default: THROW_EXPECTED('>', *_ch);
                }
                break;
            case EndTag1:
                if (*_ch == *_t)
                {
                    _e = (_t + 1);
                    set(EndTag2);
                } else THROW_EXPECTED(*_t, *_ch);
                break;
            case EndTag2 :
                if (*_ch != *_e) 
                {
                    switch (*_ch) {
                        case ' ': 
                        case '\n': 
                        case '\r': 
                        case '\t':
                            if (*_e == '\0') set(EndTag3);
                            else IT_PANIC("tag mismatch");
                            break;
                        case '>' : 
                            if (*_e == '\0') 
                            {
                                endElement(_t);
                                if (_ti==(_tags+1)) 
                                {
                                    set(Epilog);
                                }
                                else {
                                    popTag();
                                    set(StateStart);
                                }
                            } else THROW_EXPECTED("\\0", *_e);
                            break;
                        default:
                            THROW_EXPECTED(*_e, *_ch);
                            
                    }
                } else _e++;
                break;
            case EndTag3 :
                switch (*_ch) {
                    case '>' :
                        endElement(_t);
                        if (_ti==(_tags+1)) set(Epilog);
                        else {
                            popTag();
                            set(StateStart);
                        }
                        break;
                    default: THROW_EXPECTED('>', *_ch);
                }
                break;
            case CommentStart1 : 
                if (*_ch == '-') set(CommentStart2);
                else THROW_EXPECTED('-', *_ch);
                break;
            case CommentStart2 : 
                if (*_ch == '-') set(CommentStart3);
                else THROW_EXPECTED('-', *_ch);
                break;
            case CommentStart3 : 
                if (*_ch == '-') IT_PANIC("\"<!---\" is invalid XML");
                *_c++ = *_ch;
                set(CommentBody); 
                break;
            case CommentBody : 
                *_c++ = *_ch;
                if (*_ch == '-') set(CommentEnd1);
                break;
            case CommentEnd1 : 
                *_c++ = *_ch;
                if (*_ch == '-') set(CommentEnd2);
                else set(CommentBody);
                break;
            case CommentEnd2 : 
                *_c++ = *_ch;
                if (*_ch == '>') 
                {
                    _c -=3;
                    *_c = '\0';
                    comment(&_comment[0]);
                    _c = _comment.begin();
                    if (_done) set(Epilog);
                    else set(StateStart); // we're done with comment
                }
                else IT_PANIC("\"--\" cannot occur within a comment");
                break;
            case PI : 
                *_pi ++ = *_ch;
                if (*_ch == '?') set(PIEnd1);
                break;
            case PIEnd1:
                if (*_ch == '>') 
                {
                    if (_done) set(Epilog);
                    else set(StateStart);
                    --_pi;
                    *_pi = '\0';
                    proc_inst(&_proc_inst[0]);
                    _pi = _proc_inst.begin();
                }
                else THROW_EXPECTED('>', *_ch);
                break;
            case Epilog: // only comments or PIs allowed
                _done = true;
                switch (*_ch) {
                    case ' ': case '\n': case '\r': case '\t': break;
                    case '<' : set(Epilog1); break;
                    default : IT_THROW_XML("junk after doc element");
                }
                break;
            case Epilog1:
                switch (*_ch) {
                    case '!' : set(CommentStart1); break;
                    case '?' : set(PI); break;
                    default : IT_THROW_XML("junk after doc element");
                }
                break;
        }
    }

    if (_finished && (state() != Epilog)) 
        IT_PANIC("parsing incomplete");
        
    return;
}



}
