#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
// #include <ict/multivector.h>
#include <ict/bitstring.h>
#include "xddl.h"
#include <xenon/node.h>

namespace xenon {

using ict::bitstring;
using message = ict::multivector<node>;

// message algorithms

inline message::const_cursor find_first(const message & m, const path & path) {
    return find_first(m.root(), path);
}

inline message::cursor find_first(message & m, const path & path) {
    return find_first(m.root(), path);
}

std::string to_xml(message::cursor c);

// TODO: gcc 5.4 ambiguity issue
// inline std::string description(const message & m) { return ""; }

inline int64_t bit_size(message::cursor parent) {
    int64_t total = 0;
    for (message::linear_cursor i = parent.begin(); i != parent.end(); ++i) {
        if (i->consumes()) total += i->bits.bit_size();
    }
    return total;
}

// Create a global prop and set it equal to the given bitstring, or the default given by the spec.
// Precondition: The global does not yet exist.
message::cursor create_global(spec::cursor xddl_root, message::cursor globs, const std::string & name, 
    bitstring bits = bitstring());

message::cursor set_global(spec::cursor self, message::cursor value);

inline message::cursor set_global(spec::cursor self, message::cursor value, spec::cursor elem) {
    auto g = set_global(self, value);
    if (!elem->v->vhref().empty()) g->elem = elem;
    return g;
}

// load time 
spec::cursor get_variable(const std::string & name, spec::cursor context);

// parse time
message::cursor get_variable(const std::string & name, message::cursor context);

// load time validation
inline int64_t eval_variable(const std::string &name, spec::cursor context) {
    auto c = get_variable(name, context);

    // Tell the message parser is another node that is dependent on the value of nodes based on this element
    // i.e., its not a plain old field.
    c->flags.set(element::dependent_flag);
    return 1;
}

int64_t eval_variable_list(const std::string &first, const std::string &second, spec::cursor context); 

// TODO: move this one to spec.h, otherwise loading a doc is dependent on message.h
inline int64_t eval_function(const std::string &name, spec::cursor, 
    const std::vector<ict::expression::param_type> &params) {
    if ((name == "sizeof") || (name == "defined") || (name == "value") || (name == "gsm7")) {
        if ((params.size() == 1) && (!params[0].name.empty())) return 1;
    }
    IT_PANIC("load time eval_function not implemented for " << name);
}

// parse time validation
inline int64_t eval_variable(const std::string &name, message::cursor context) {
    return get_variable(name, context)->value();
}

inline int64_t eval_variable_list(const std::string &first, const std::string &second, message::cursor context) {
    auto f = get_variable(first, context);
    auto s = find_first(f, second);
    if (s == f.end()) IT_PANIC("unmatched second variable: " << second);
    return ict::to_integer<int64_t>(s->bits);
}

int64_t eval_function(const std::string &name, message::cursor context,  
    const std::vector<ict::expression::param_type> &params);

template <typename S, typename C>    
inline void description_xml(S & os, C c) {
    auto d = c->elem->v->vdescription(c->elem, c);
    if (!d.empty()) os << "<description>" << ict::xmlize(d) << "</description>";
}

namespace ht { // heading type
    // TODO: these enums need to be a dynamic data structure.  Eventually supporting user added items.
    enum index {
        name,
        row,
        mnemonic,
        length,
        value,
        
        hex,
        file,
        line,

        description,

        timestamp,
        source,
        dest,
        tech,

        column_count,
    };

    inline const char * to_name(ht::index h) {
        switch (h) {
            case ht::mnemonic : return "Mnemonic"; break;
            case ht::name : return "Name"; break;
            case ht::length : return "Length"; break;
            case ht::value : return "Value"; break;
            case ht::hex : return "Hex"; break;
            case ht::line : return "Line"; break;
            case ht::file : return "File"; break;
            case ht::row : return "Row"; break;
            case ht::description : return "Description"; break;
            case ht::timestamp : return "Timestamp"; break;
            case ht::source : return "Source"; break;
            case ht::dest : return "Dest"; break;
            case ht::tech : return "Tech"; break;

            case ht::column_count : return "Column Count"; break;
        }
        return "unknown heading";
    }
    typedef std::vector<std::string> text_row;
    typedef std::vector<text_row> text_rows;
    struct heading {
        heading() : width(0) {}
        ht::index type;
        size_t width;
    };
};

} // namespace
