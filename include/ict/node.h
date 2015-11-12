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
        set_prop_node,
        peek_node,
        error_node
    };

    inline const node_info_type & info() const;
    node() = default;
    node(node_type type, xddl_cursor elem, bitstring bs = bitstring()) : type(type), elem(elem), bits(bs) {};

    bool empty() { return bits.empty(); }
    string64 tag() const;
    std::string name() const;
    size_t line() const;
    std::string file() const;
    size_t length() const { return bits.bit_size(); }
    int64_t value() const;
    bool is_field() const { return type == field_node; }
    bool is_prop() const { return type == prop_node; }
    bool is_extra() const { return type == extra_node; }
    bool consumes() const { return type == field_node || type == incomplete_node || type == extra_node; }
    bool is_terminal() const { return consumes() || type == prop_node || type == set_prop_node ||  type == peek_node; }

    bool is_per() const { return elem->flags[element::per_flag]; }
    bool is_oob() const { return elem->flags[element::oob_flag]; }
    bool is_pof() const { return is_field() && !elem->flags[element::dependent_flag]; } // "plain ol' field"

    const char * mnemonic() const {
        switch (type) {
            case node::nil_node : return "EMP";
            case node::root_node : return "ROT";
            case node::extra_node : return "EXT";
            case node::field_node : return "FLD";
            case node::float_node : return "FLT";
            case node::incomplete_node : return "INC";
            case node::message_node : return "MSG";
            case node::record_node : return "REC";
            case node::repeat_node : return "REP";
            case node::repeat_record_node : return "RPR";
            case node::prop_node : return "PRP";
            case node::set_prop_node : return "SET";
            case node::peek_node : return "PEK";
            case node::error_node : return "ERR";
        }
        return "err";
    }

    friend bool operator==(const node & a, int64_t b) {
        return a.value() == b;
    }

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

    typedef std::function<void(std::ostream&, msg_cursor)> start_tag_op;
    node_info_type(const char * name, flag_type flags) : name(name), flags(flags) {}
    node_info_type(const char * name, flag_type flags, start_tag_op op) : name(name), flags(flags), start_tag(op) {}
    const char * name;
    flag_type flags;
    start_tag_op start_tag = [&](std::ostream&, msg_cursor){};
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
} // namespace

