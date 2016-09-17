#pragma once 
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.

namespace xenon { 

template <>
inline std::string name_of(const xenon::element & e) {
    return e.name();
}
template <typename T, typename U>
inline T * get_ptr(const U & v) {
    return std::dynamic_pointer_cast<T>(v).get();
}

struct cmp_name {
    cmp_name(const std::string & name) :name(name) {}
    bool operator()(const element & e) { 
        return e.name() == name; 
    }
    std::string name;
};

inline std::string tag_of(const element & e) {
    return e.tag().c_str();
}

// inline methods needed by multivector 
inline std::ostream& operator<<(std::ostream & os, const element & e) {
    e.to_string(os);
    return os;
}

inline element& elem_of(spec::cursor c) {
    return *c;
}

inline void parse(spec::cursor self, msg_cursor parent, ict::ibitstream & bs) {
    self->v->vparse(self, parent, bs);
}

inline std::string description(spec::cursor self, spec::cursor referer, msg_const_cursor c) {
    if (!c->desc.empty()) return c->desc;
    return self->v->vdescription(referer, c);
}

inline std::string description(msg_const_cursor c) {
    return description(c->elem, c->elem, c);
}

inline std::string enum_string(spec::cursor self, msg_const_cursor c) {
    return self->v->venum_string(self, c);
}

// start -- a cursor anywhere in the tree
// returns start.end() upon failure
template <typename SpecCursor>
inline SpecCursor find_prop(SpecCursor start, const std::string & prop_name) {
    auto xddl = get_root(start).begin();
    auto exp = std::find_if(xddl.begin(), xddl.end(), [&](const element & c){ return c.tag() == "export"; });
    if (exp != xddl.end()) {
        auto prop = std::find_if(exp.begin(), exp.end(), [&](const element & c){ 
            return c.tag() == "prop" && c.name() == prop_name; });
        if (prop != exp.end()) return prop;
    }
    return start.end();
}

template <typename Cursor>
inline std::string create_jump_name(Cursor start, const std::string & value) {
    static auto prop_path = path("xddl/export/prop");
    auto c = rfind_first(start, value);
    if (c.is_root()) {
        auto root = spec::cursor(c);
        auto x = find_prop(root, value);
        if (x == root.end()) IT_PANIC("cannot find " << value);
    }        
    c->flags.set(element::dependent_flag);

    return value;
}


} // namespace
