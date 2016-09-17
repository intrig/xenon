#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/string64.h>
#include <ict/ict.h>
#include <ict/multivector.h>
#include <string>
#include <ict/expr.h>
#include <xenon/att_pair.h>
#include <xenon/functions.h>
#include <xenon/recref.h>

namespace xenon {
    namespace lua {
        struct lua_State;
        // using this wrapper instead of just lua_State is required since lua_State is opaque.
        struct state_wrapper {
            state_wrapper(lua_State * p) : p(p) {}
            lua::lua_State * p;
        };

        inline lua::lua_State * get(std::shared_ptr<state_wrapper> const & l) { return l.get()->p; }
    }


// convert attribute strings to custom types
template <typename Cursor>
inline recref create_url(Cursor, const std::string & x) {
    if (x.empty()) return recref();
    recref a(x);
    if (a.empty()) IT_PANIC("invalid url: " << x);
    return a;
}

template <typename Cursor>
inline recref create_id_url(Cursor parent, std::string x) {
    x = "#" + x;
    return create_url(parent, x);
}

template <typename Cursor>
std::string create_jump_name(Cursor start, const std::string & value);

template <typename Cursor>
inline bool create_bool(Cursor, const std::string & b) {
    if (b == "false") return false;
    if (b == "true") return true;
    IT_PANIC("invalid boolean value: " << b << " (must be 'true' or 'false')");
}


template <typename Cursor>
inline ict::expression create_expression(Cursor start, const std::string & value) {
    ict::expression exp;
    if (value.empty()) exp = ict::expression();
    else exp = ict::expression(value, start);
    return exp;
}

template <typename Cursor>
inline std::string create_string(Cursor, const std::string &s) {
    return s;
}

template <typename Cursor>
inline std::string create_setprop_name(Cursor parent, const std::string &s) {
    // make an expression of the prop to see if the prop exists
    std::string v = '{' + s + '}';
    ict::expression{v, leaf(parent)};
    return s;
}

template <typename Cursor>
inline int64_t create_integer(Cursor, const std::string &s) {
    int64_t v;
    switch (s[0]) {
        case '#' :
            if (ict::string_to_int64(v, s.c_str() + 1, 16)) return v;
            break;
        case '@' :
            if (ict::string_to_int64(v, s.c_str() + 1, 2)) return v;
            break;
        default :
            if (ict::string_to_int64(v, s.c_str())) return v;
            break;
    }
    IT_PANIC("cannot convert " << s << " to integer");
}

template <typename Cursor>
inline size_t create_pos_integer(Cursor parent, const std::string &s) {
    auto v = create_integer(parent, s);
    if (v <= 0) IT_PANIC("expected positive integer, got " << s);
    return (size_t) v;
}

template <typename Cursor>
inline size_t create_size(Cursor parent, const std::string &s) {
    auto v = create_integer(parent, s);
    if (v < 0) IT_PANIC("expected non-negative integer, got " << s);
    return (size_t) v;
}

// test functions for <choice> distinction
template <typename AttList>
inline size_t fragment_test(const AttList &) {
    return 0;
}

template <typename AttList>
inline size_t record_test(const AttList & atts) {
    auto id = find_att(atts, "id");
    auto href = find_att(atts, "href");
    if (id.empty() && href.empty()) return 0;
    if (!id.empty() && !href.empty()) IT_PANIC("id and href are mutually exclusive attributes");
    if (!href.empty()) return 1;
    return 2;
}

template <typename AttList>
inline size_t repeat_test(const AttList & atts) {
    int ret = 0; 
    auto num = att_exists(atts, "num");
    auto max = att_exists(atts, "max");
    auto min = att_exists(atts, "min");
        
    if (num && (min || max)) IT_PANIC("<repeat> with num attribute cannot have min or max attribute");
    if (num) ret = 1;
    if (max || min) ret = 2;
    return ret;
}

template <typename Cursor, typename Parser>
inline void end_handler(Cursor self, Parser & parser) {
    //IT_WARN("calling end_handler for " << *self);
    self->v->vend_handler(self, parser);
}

struct element;
inline std::ostream& operator<<(std::ostream & os, const element &);

} // namespace
