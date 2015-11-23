#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license
#include <ict/bitstring.h>
#include <ict/multivector.h>
#include <ict/string64.h>
#include <memory>
#include <functional>
#include <iostream>

namespace ict {
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

    inline const node_info_type & info() const;
    node() = default;
    node(node_type type, xddl_cursor elem, bitstring bs = bitstring()) : type(type), elem(elem), bits(bs) {
        flags.set(type);
    };

    bool empty() { return bits.empty(); }
    string64 tag() const;
    std::string name() const;
    size_t line() const;
    std::string file() const;
    size_t length() const { return bits.bit_size(); }
    int64_t value() const;
    bool is_field() const { return flags.test(field_node); }
    bool is_prop() const { return flags.test(prop_node) || flags.test(setprop_node) || flags.test(peek_node); }
    bool is_extra() const { return flags.test(extra_node); }
    bool is_incomplete() const { return flags.test(incomplete_node); }
    bool consumes() const { return is_field() || is_incomplete() || is_extra(); }
    bool is_terminal() const { return consumes() || is_prop(); }

    bool is_per() const { return elem->flags.test(element::per_flag); }
    bool is_oob() const { return elem->flags.test(element::oob_flag); }
    bool is_pof() const { return is_field() && !elem->flags.test(element::dependent_flag); } // "plain ol' field"

    void set_incomplete() {
        // type = incomplete_node;
        flags.reset(field_node);
        flags.set(incomplete_node);
    }

    const char * mnemonic() const {
        if (flags.test(nil_node)) return "EMP";
        if (flags.test(root_node)) return "ROT";
        if (flags.test(extra_node)) return "EXT";
        if (flags.test(field_node)) return "FLD";
        if (flags.test(float_node)) return "FLT";
        if (flags.test(incomplete_node)) return "INC";
        if (flags.test(message_node)) return "MSG";
        if (flags.test(record_node)) return "REC";
        if (flags.test(repeat_node)) return "REP";
        if (flags.test(repeat_record_node)) return "RPR";
        if (flags.test(prop_node)) return "PRP";
        if (flags.test(setprop_node)) return "SET";
        if (flags.test(peek_node)) return "PEK";
        if (flags.test(error_node)) return "ERR";
        return "err";
    }

    friend bool operator==(const node & a, int64_t b) {
        return a.value() == b;
    }

    std::bitset<node_count> flags;
    node_type type = nil_node;
    xddl_cursor elem;
    bitstring bits;
    std::string desc;
    size_t dom_length = 0; 
};

typedef multivector<node>::cursor msg_cursor;
typedef multivector<node>::const_cursor msg_const_cursor;

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

template <>
inline std::string name_of(const node & n) {
    return n.name();
}

template <typename Stream>
Stream & to_debug(Stream & os, const node & n) {
    os << n.tag() << " " << n.mnemonic() << " " << n.name() << " (" <<
        (n.is_field() ? "field " : "") <<
        (n.is_prop() ? "prop " : "") <<
        (n.is_extra() ? "extra " : "") <<
        (n.consumes() ? "consumes " : "") <<
        (n.is_terminal() ? "term " : "") <<
        (n.is_per() ? "per " : "") <<
        (n.is_oob() ? "oob " : "") <<
        (n.is_pof() ? "pof " : "") << ')';
    return os;
}
} // namespace

