#include <xenon/ict/ict.h>
#include <xenon/xml_parser_base.h>

namespace xenon {
// order matters here, index is state()
const char *state_names[] = {"First",         "StateStart",
                             "Start1",        "CommentStart1",
                             "CommentStart2", "CommentStart3",
                             "CommentBody",   "CommentEnd1",
                             "CommentEnd2",   "PI",
                             "PIEnd1",        "StartTag1",
                             "FirstAtt1",     "FirstAtt2",
                             "AttName1",      "AttName2",
                             "AttName2a",     "AttName3",
                             "AttName4",      "AttValue1",
                             "AttValue2",     "AttValue3",
                             "Empty",         "EndTag1",
                             "EndTag2",       "EndTag3",
                             "Epilog",        "Epilog1"};

xml_parser_base::xml_parser_base() {
    int i;
    // init once stuff
    atts_ = new char *[MaxAtts];
    if (!atts_) {
        IT_PANIC("out of memory");
    }
    for (i = 0; i < MaxAtts; ++i) {
        atts_[i] = new char[MaxAttSize];
        if (!atts_[i])
            IT_PANIC("out of memory:" << i);
    }

    tags_ = new char *[MaxTagDepth];
    if (!tags_)
        IT_PANIC("out of memory");
    for (i = 0; i < MaxTagDepth; ++i) {
        tags_[i] = new char[MaxTagSize];
        if (!tags_[i])
            IT_PANIC("out of memory:" << i);
    }

    reset();
}

// this doen't make sense since its pretty much impossible to copy the state of
// the parser right now but at least we won't get any bad deallocs
xml_parser_base::xml_parser_base(const xml_parser_base &) : xml_parser_base() {}

xml_parser_base &xml_parser_base::operator=(const xml_parser_base &) {
    reset();
    return *this;
}

xml_parser_base::~xml_parser_base() {
    int i;
    for (i = 0; i < MaxAtts; ++i) {
        delete[] atts_[i];
    }
    delete[] atts_;

    for (i = 0; i < MaxTagDepth; ++i) {
        delete[] tags_[i];
    }
    delete[] tags_;
}

void xml_parser_base::stop() {
    if (finished_)
        IT_PANIC("cannot stop a finished parser");
    if (suspended_)
        IT_PANIC("cannot suspend a suspended parser");
    suspended_ = true;
}

void xml_parser_base::resume() {
    if (finished_)
        IT_PANIC("cannot resume a finished parser");
    if (!suspended_)
        IT_PANIC("cannot resume a running parser");
    suspended_ = false;
}

void xml_parser_base::reset() {
    done_ = false;
    finished_ = false;

    line_ = 1;
    column_ = 0;
    byte_ = 0;

    cdata_.assign(MaxCDataSize, '\0');
    d_ = cdata_.begin();

    comment_.assign(MaxCommentSize, '\0');
    c_ = comment_.begin();

    proc__inst.assign(MaxCommentSize, '\0');
    pi_ = proc__inst.begin();

    resetTags();
    resetAtts();

    state_ = First;
    cd__state = Idle;
    att__state = Idle;

    suspended_ = false;
}

// See http://www.w3.org/TR/REC-xml/REC-xml-20040204.xml for implementation
// details.
void xml_parser_base::parse(const char *s, int len, bool final) {
    try {
        xparse(s, len, final);
    } catch (xenon::xml_error &e) {
        IT_FATAL(e.description << " in [" << e.xml_line << ':' << e.xml_column
                               << ']');
    }
}

void xml_parser_base::xparse(const char *s, int len, bool final) {
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
                    startElement(t_, (const char **)atts_);
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
                    startElement(t_, (const char **)atts_);
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
                    startElement(t_, (const char **)atts_);
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
                    startElement(t_, (const char **)atts_);
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
                startElement(t_, (const char **)atts_);
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

} // namespace xenon
