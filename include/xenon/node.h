#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/bitstring.h>
#include <ict/multivector.h>
#include <ict/string64.h>
#include <memory>
#include <functional>
#include <iostream>

namespace xenon {
struct node_info_type;
typedef std::vector<node_info_type> node_info_list;

struct node {
    enum node_type {
        nil_node = 0,
        root_node,
        extra_node,
        field_node,
        float_node,
        incomplete_node,
        message_node,
        record_node,
        repeat_node,
        repeat_record_node,
        prop_node,
        setprop_node,
        peek_node,
        error_node,
        node_count
    };

    enum node_flags {
        visible,
        flag_count
    };

    inline const node_info_type & info() const;
    node() = default;
    node(node_type type, spec_cursor elem, ict::bitstring bs = ict::bitstring()) : type(type), elem(elem), bits(bs) { };

    bool empty() { return bits.empty(); }
    ict::string64 tag() const;
    std::string name() const;
    size_t line() const;
    std::string file() const;
    size_t length() const { return bits.bit_size(); }
    int64_t value() const;
    bool is_field() const { return type == field_node; }
    bool is_prop() const { return type == prop_node || type == setprop_node || type == peek_node; }
    bool is_extra() const { return type == extra_node; }
    bool is_incomplete() const { return type == incomplete_node; }
    bool consumes() const { return is_field() || is_incomplete() || is_extra(); }
    bool is_terminal() const { return consumes() || is_prop(); }

    bool is_encoding() const { return elem->flags.test(element::enc_flag); }
    bool is_oob() const { return elem->flags.test(element::oob_flag); }
    bool is_pof() const { return is_field() && !elem->flags.test(element::dependent_flag); } // "plain ol' field"
    bool is_visible() const { return flags.test(visible); }
    void set_visible(bool yes) { flags.set(visible, yes); }

    void set_incomplete() {
        type = incomplete_node;
    }

    const char * mnemonic() const {
        switch (type) {
            case nil_node: return "EMP";
            case root_node: return "ROT";
            case extra_node: return "EXT";
            case field_node: return "FLD";
            case float_node: return "FLT";
            case incomplete_node: return "INC";
            case message_node: return "MSG";
            case record_node: return "REC";
            case repeat_node: return "REP";
            case repeat_record_node: return "RPR";
            case prop_node: return "PRP";
            case setprop_node: return "SET";
            case peek_node: return "PEK";
            case error_node: return "ERR";
            default: return "err";
        }
        return "err";
    }

    friend bool operator==(const node & a, int64_t b) {
        return a.value() == b;
    }

    std::bitset<flag_count> flags;
    node_type type = nil_node;
    spec_cursor elem;
    ict::bitstring bits;
    std::string desc;
    size_t dom_length = 0; 
};

typedef ict::multivector<node>::cursor msg_cursor;
typedef ict::multivector<node>::const_cursor msg_const_cursor;

struct node_info_type {
    enum flag_type {
        is_nil         = 1u << 0,
        is_terminal    = 1u << 1,
        is_parent      = 1u << 2,
        is_conditional = 1u << 3,
        is_property    = 1u << 4
    };

    typedef std::function<void(std::ostream&, msg_const_cursor)> start_tag_op;
    node_info_type(const char * name, flag_type flags) : name(name), flags(flags) {}
    node_info_type(const char * name, flag_type flags, start_tag_op op) : name(name), flags(flags), start_tag(op) {}
    const char * name;
    flag_type flags;
    start_tag_op start_tag = [&](std::ostream&, msg_const_cursor){};
};

inline std::string node_tag(const node & n) {
    return n.tag().c_str();
}

extern const node_info_list node_info;

inline const node_info_type & node::info() const { return node_info[type]; }

std::ostream& operator<<(std::ostream& os, const node & n);

template <typename S, typename C> 
// S: output stream
// C: message cursor
inline S& start_tag(S& os, C) {
    return os;
}

template <typename S, typename C> 
// S: output stream
// C: message cursor
inline S& end_tag(S& os, C) {
    return os;
}

template <typename Stream>
Stream & to_debug(Stream & os, const node & n) {
    os << n.tag() << " " << n.mnemonic() << " " << n.name() << " (" <<
        (n.is_field() ? "field " : "") <<
        (n.is_prop() ? "prop " : "") <<
        (n.is_extra() ? "extra " : "") <<
        (n.consumes() ? "consumes " : "") <<
        (n.is_terminal() ? "term " : "") <<
        (n.is_encoding() ? "encoding " : "") <<
        (n.is_oob() ? "oob " : "") <<
        (n.is_pof() ? "pof " : "") << ')';
    return os;
}

template <>
inline std::string name_of(const xenon::node & n) {
    return n.name();
}

} // namespace
