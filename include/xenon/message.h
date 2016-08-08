#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
// #include <ict/multivector.h>
#include <ict/bitstring.h>
#include <xenon/xddl.h>
#include <xenon/node.h>

namespace xenon {

using ict::bitstring;
using message = ict::multivector<node>;

// node info type

// message algorithms

std::string to_xml(message::cursor c);

inline int64_t bit_size(message::cursor parent) {
    int64_t total = 0;
    for (message::linear_cursor i = parent.begin(); i != parent.end(); ++i) {
        if (i->consumes()) total += i->bits.bit_size();
    }
    return total;
}

// Create a global prop and set it equal to the given bitstring, or the default given by the spec.
// Precondition: The global does not yet exist.
inline message::cursor create_global(spec::cursor xddl_root, message::cursor globs, const std::string & name, 
    bitstring bits = bitstring()) {
    static auto prop_path = path("export/prop");
    auto root = xddl_root;
    auto c = find(root, prop_path, tag_of, cmp_name(name));

    if (c == root.end()) { // must be an extern
        c = find(root, "extern", tag_of, cmp_name(name));
        if (c == root.end()) IT_PANIC("internal panic looking for " << name  << " in " << xddl_root->parser->file); 
    }

    if (bits.empty()) {
        if (auto prop_elem = get_ptr<prop>(c->v)) {
            if (!prop_elem) IT_PANIC("internal panic");
            auto v = prop_elem->value.value(leaf(globs));
            bits = ict::from_integer(v);
        } 
    }

    return globs.emplace(node::prop_node, c, bits);
}

inline message::cursor set_global(spec::cursor self, message::cursor value) {
    auto globs = ict::get_root(value).begin(); 
    auto g = find(globs, value->name());
    if (g == globs.end()) g = create_global(get_root(self).begin(), globs, value->name(), value->bits);
    else g->bits = value->bits;
    return g;
}

inline message::cursor set_global(spec::cursor self, message::cursor value, spec::cursor elem) {
    auto g = set_global(self, value);
    if (!elem->v->vhref().empty()) g->elem = elem;
    return g;
}

// load time 
inline spec::cursor get_variable(const std::string & name, spec::cursor context) {
    static auto prop_path = path("xddl/export/prop");
    static auto extern_path = path("xddl/extern");
    spec::ascending_cursor first(context);
    while (!first.is_root()) {
        if (name == first->name()) return first;
        ++first;
    }
    auto root = spec::cursor(first);
    auto x = find(root, prop_path, tag_of, cmp_name(name));
    if (x != root.end()) return x;

    // not found, search for <extern>
    // TODO: remove <extern> and just use global, if 2 globals have different defaults, then undefined
    // or get rid of global and just use props
    auto c = find(root, extern_path, tag_of, cmp_name(name));
    if (c != root.end()) return c;
    
    // verify(root);
    IT_PANIC("cannot find " << name << " starting at " << context->name());
}

// parse time
inline message::cursor get_variable(const std::string & name, message::cursor context) {
    auto first = rfind(context, name);
    if (!first.is_root()) return first;

    // first is now pointing at message root
    auto globs = first.begin();
    auto g = find(globs, name);
    if (g == globs.end()) {
        auto xddl_root = ict::get_root(context->elem).begin();
        try {
            g = create_global(xddl_root, globs, name);
        } catch (std::exception & e) {
            ict::osstream os;
            os << e.what() << " [" << context->file() << ":" << context->line() << "]";
            IT_FATAL(os.str());
        }
    }
    return g;
}

// load time validation
inline int64_t eval_variable(const std::string &name, spec::cursor context) {
    auto c = get_variable(name, context);

    // Tell the message parser is another node that is dependent on the value of nodes based on this element
    // i.e., its not a plain old field.
    c->flags.set(element::dependent_flag);
    return 1;
}

inline int64_t eval_variable_list(const std::string &first, const std::string &second, spec::cursor context) {
    auto f = get_variable(first, context);
    auto s = find(f, second);
    if (s == f.end()) { // second not found, it may be in another spec though (f may be a reclink)
        // f is a reclink, so get the record it is pointing to.
        if (auto r = get_ptr<reclink>(f->v)) {
            auto xddl = rfind(context, "xddl", tag_of);
            auto c = find(xddl, "record", tag_of, [&](const element &e) {
                if (auto rec = get_ptr<recdef>(e.v)) {
                    if (rec->id == r->href) return 1;
                }
                return 0;
            });
            if (c == xddl.end()) IT_PANIC("cannot find record: " << r->href);
            s = find(c, second);
            if (s == c.end()) IT_PANIC("cannot find " << second << " in " << r->href);
        }
    }
    s->flags.set(element::dependent_flag);
    return 1;
}


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
    auto s = find(f, second);
    if (s == f.end()) IT_PANIC("unmatched second variable: " << second);
    return ict::to_integer<int64_t>(s->bits);
}

inline int64_t eval_function(const std::string &name, message::cursor context,  
    const std::vector<ict::expression::param_type> &params) {
    if (name == "sizeof") {
        auto c = get_variable(params[0].name, context);
        return bit_size(c);
    }
    else if ((name == "defined")) {
        if (rfind(context, params[0].name).is_root()) return 0;
        return 1;
    }

    else if ((name == "value")) {
        auto c = rfind(context, params[0].name);
        if (!c.is_root()) return c->value();
        IT_WARN("cannot find " << params[0].name);
        return 0;
    }

    IT_PANIC("runtime eval_function not implemented for " << name);
}

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
