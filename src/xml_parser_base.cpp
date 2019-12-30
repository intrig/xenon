#include <xenon/ict/ict.h>
#include <xenon/xml_parser_base.h>

namespace xenon {
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

} // namespace xenon
