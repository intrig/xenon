//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <algorithm>
#include <ict/spec.h>
#include <ict/node.h>
#include <ict/xddl.h>
#include <ict/bitstring.h>
#include <ict/message.h>
#include <ict/ximsi.h>
#include <ict/DateTime.h>
#include <set>

// xddl script
#include <ict/xddl.h>
#include <ict/lua.hpp>

namespace ict {

const int DateTime::daysmonth[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
const int DateTime::daysmonthleap[13] = { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

template <typename State>
inline message::cursor get_cursor(State * L) {
    ict::lua::lua_getglobal(L, "node");
    return *static_cast<message::cursor *>(lua_touserdata(L, -1));
}

// get a timestamp from a .Net encoded date (don't ask)
static int script_datetime(ict::lua::lua_State *L) {
    auto n = get_cursor(L);
    auto value = ict::to_integer<int64_t>(n->bits);
    auto dt = DateTime(value);
    std::ostringstream os;
    os << dt;
    ict::lua::lua_pushstring(L, os.str().c_str());
    return 1;
}

// get the description of a previous node
static int script_Description(ict::lua::lua_State *L) {
    const char * s = ict::lua::luaL_checklstring(L, 1, NULL);
    auto n = get_cursor(L);
    auto c = rfind(n, s);
    if (c.is_root()) ict::lua::lua_pushstring(L, "");
    else ict::lua::lua_pushstring(L, description(c).c_str());
    return 1;
}

// get the enum string for this node
static int script_EnumValue(ict::lua::lua_State *L) {
    auto n = get_cursor(L);
    ict::lua::lua_pushstring(L, n->elem->v->venum_string(n->elem, n).c_str());
    return 1;
}

// get the value of a previous node
static int script_Value(ict::lua::lua_State *L) {
    const char * s = ict::lua::luaL_checklstring(L, 1, NULL);
    auto n = get_cursor(L);
    auto c = rfind(n, s);
    if (c.is_root()) ict::lua::lua_pushnumber(L, 0);
    else ict::lua::lua_pushnumber(L, c->value());
    return 1;
}

static int script_Slice(ict::lua::lua_State *L) {
    int index = static_cast<int>(ict::lua::luaL_checknumber(L, 1));
    int length = static_cast<int>(ict::lua::luaL_checknumber(L, 2));
    auto n = get_cursor(L);
    auto subs = n->bits.substr(index, length);
    auto num = ict::to_integer<int64_t>(subs);
    ict::lua::lua_pushnumber(L, num);
    return 1;
}


static int script_TwosComplement(ict::lua::lua_State * L) {
    auto n = get_cursor(L);
    auto value = ict::to_integer<int>(n->bits);
    value <<= (32 - n->bits.bit_size());
    value >>= (32 - n->bits.bit_size());
    ict::lua::lua_pushnumber(L, value);
    return 1;
}

static int script_Ascii(ict::lua::lua_State * L) {
    auto & bits = get_cursor(L)->bits;
    auto s = std::string(bits.begin(), bits.bit_size() / 8);
    for (auto & c : s) if (!isprint(c)) c = '.';
    ict::lua::lua_pushstring(L, s.c_str());
    return 1;
}

static int script_Imsi_S(ict::lua::lua_State * L) {
    auto & bits = get_cursor(L)->bits;
    ict::lua::lua_pushstring(L, decode_imsi(bits).c_str());
    return 1;
}

static int script_Gsm7(ict::lua::lua_State * L) {
    int fill = 0;
    const char * s = ict::lua::luaL_checklstring(L, 1, NULL);
    if (s) {
        auto c = ict::rfind(get_cursor(L), s);
        if (!c.is_root()) fill = to_integer<int>(c->bits); 
    }
    std::string sms = ict::gsm7(get_cursor(L)->bits, fill);
    ict::lua::lua_pushstring(L, sms.c_str());
    return 1;
}

static int script_find(ict::lua::lua_State *L) {
    auto n = ict::get_root(get_cursor(L));
    const char * s = luaL_checkstring(L, 1);

    auto c = ict::find(n, s);
    if (c == n.end()) {
        lua_pushstring(L, "");
        return 1;
    }
    lua_pushstring(L, description(c).c_str());
    return 1;
}

static int script_Search(ict::lua::lua_State *) {
    IT_PANIC("search() lua function is no longer supported, use find().");
}

void element::to_string(std::ostream & os) const {
    os << tag();
    if (name_.empty()) os  << " (unnamed)";
    else os << " " << name();
    os << " ";
    if (v) v->vto_string(os);
    else os << "(nullptr)";
}

void recdef::vto_string(std::ostream & os) const {
    os << id;
}

void type::vto_string(std::ostream & os) const {
    os << id << '\n';
    for (auto & i : items) os << "  item: " << i.first << " " << i.second.value << '\n';
    for (auto & i : ranges) os << "  range: " << i.second.value << '\n';
}

// xddl load time
void xcase::vend_handler(xddl::cursor, xddl & parser) {
    auto & xs = parser.xswitch_stack.back();
    if (xs->has_default) IT_PANIC("<default> must be the last case");
    auto i = xs->cases.size(); // get the index of the current case
    auto it = xs->cases.find(value);
    if (it != xs->cases.end()) {
        IT_PANIC("duplicate case value found for " << value);
    }
    xs->cases[value] = i;
}

template <typename T>
void build_type_struct(xddl::cursor parent, T type_ptr) {
    for (auto c = parent.begin(); c!= parent.end(); ++c) {
        auto t = c->tag();
        if (t == "script") {
            if (type_ptr->l) IT_PANIC("multiple scripts in type definition");
            type_ptr->l = get_ptr<script>(c->v)->l;
        }
        else if (t == "item") {
            auto item = get_ptr<xitem>(c->v);
            auto i = type_ptr->items.find(item->key);
            if (i != type_ptr->items.end()) IT_PANIC("<item> \"" << item->value << "\" with key " << 
                item->key << " already defined");
            type_ptr->items[item->key] = type::item_data { item->value, parent.end(), item->href };
        }
        else if (t == "range") {
            auto r = get_ptr<range>(c->v);
            if (r->start > r->end) IT_PANIC("invalid <range>");
            auto key = std::make_pair(r->start, r->end);
            auto i = type_ptr->ranges.find(key);
            if (i != type_ptr->ranges.end()) IT_PANIC("<range> already defined");
            type_ptr->ranges[key] = type::item_data { r->value, parent.end(), r->href };
        }
    }
}

template <typename T>
void create_anon_type(T * self_ptr, xddl::cursor self, xddl & parser) {
    self_ptr->href= "anon";
    std::shared_ptr<var_type> v = std::make_shared<type>("anon");

    build_type_struct(self, std::dynamic_pointer_cast<type>(v));

    // we now have a complete type struct.  Remove all the current children and replace them with it.
    self.clear();
    auto e = self.emplace(v, "type", "anon");
    e->parser = &parser;
    e->line = self->line;
}

inline bool has_anon_type(xddl::cursor self) {
    if (self.empty()) return false;
    for (auto first = self.begin(); first != self.end(); ++first) {
        if (first->tag() != "comment") return true;
    }
    return false;
}

void field::vend_handler(xddl::cursor self, xddl &parser) {
    if (has_anon_type(self)) {
        create_anon_type(this, self, parser);
    }
}

void xdefault::vend_handler(xddl::cursor, xddl & parser) {
    auto & xs = parser.xswitch_stack.back();
    xs->has_default = true;
}

template <typename State, typename Op> 
void reg_func(State l, Op func, const char * name) {
    ict::lua::lua_pushcfunction(l, func); \
    ict::lua::lua_setglobal(l, name);
}

void script::vend_handler(xddl::cursor, xddl & dom) {
    l = lua::luaL_newstate();
    lua::luaL_openlibs(l);

    reg_func(l, script_Description, "Description");
    reg_func(l, script_datetime, "dot_net_time");
    reg_func(l, script_EnumValue, "EnumValue");
    reg_func(l, script_Value, "Value");
    reg_func(l, script_Slice, "slice");
    reg_func(l, script_TwosComplement, "TwosComplement");
    reg_func(l, script_Ascii, "ascii");
    reg_func(l, script_Gsm7, "gsm7");
    reg_func(l, script_Search, "search");
    reg_func(l, script_find, "find");
    reg_func(l, script_Imsi_S, "imsi_s");

    if (ict::lua::luaL_loadstring(l, dom.cdata.c_str())) IT_PANIC(ict::lua::lua_tostring(l, -1));

    // pop the compiled script off the stack and set it to the global "f"
    ict::lua::lua_setglobal(l, "f");
}

void type::vend_handler(xddl::cursor self, xddl &) {
    build_type_struct(self, this);
    // now that we built all the needed data structures, we can delete the child items.
    // TODO: get this to work
    if (!self.empty()) self.clear();
}

template <typename T, typename Cursor, typename Map>
void create_url_map(Cursor self, Map & m, string64 tag) {
    for (auto i = self.cbegin(); i!= self.cend(); ++i) {
        if (i->tag() == tag) {
            if (auto f = get_ptr<T>(i->v)) {
                auto c = m.find(f->id);
                if (c == m.end()) m[f->id] = i;
                else IT_PANIC("id " << f->id << " already used");
            }
        }
    }
}

template <typename Cursor, typename Map>
void link_ref(Cursor & self, ict::url & url, Map & m) {
    //IT_WARN("linking ref for " << url);
    //IT_WARN("self is " << *self);
    if (url.is_local()) {
        auto i = m.find(url);
        if (i == m.end()) IT_PANIC("cannot find local reference: " << url);
        else self->v->vset_ref(i->second);
    } else self->v->vset_ref(self.end());
}

template <typename Cursor, typename T, typename Map>
void link_type_ref(Cursor & self, T first, T last, Map &recmap) {
    while (first != last) {
        if (first->second.href.is_local()) {
            auto i = recmap.find(first->second.href);
            if (i == recmap.end()) IT_PANIC("cannot find local reference: " << first->second.href);
            first->second.ref = i->second;
        } else first->second.ref = self.end();
        ++first;
    }
}

template <typename Cursor, typename Map>
void link_type_refs(Cursor & self, Map &recmap) {
    auto p = get_ptr<type>(self->v);
    if (p) {
        link_type_ref(self, p->items.begin(), p->items.end(), recmap);
        link_type_ref(self, p->ranges.begin(), p->ranges.end(), recmap);
    } else IT_PANIC("internal panic error");
}

// If the hrefs are local, then validate them and link the ref.
template <typename Cursor, typename RecMap, typename TypeMap>
void link_local_refs(Cursor parent, RecMap & rec_map, TypeMap & type_map) {
    ict::recurse(parent, [&](xddl::cursor self, xddl::cursor) {
        auto url = self->v->vhref();
        if (!url.empty()) {
            if (self->tag() == "fragment" || self->tag() == "record") link_ref(self, url, rec_map);
            else if (self->tag() == "field" || self->tag() == "prop" || self->tag() == "setprop") 
                link_ref(self, url, type_map);
        }

        else if (self->tag() == "type") link_type_refs(self, rec_map);
    });
}

void link_anon_types(xddl::cursor parent) {
    static auto field_tag = string64("field");
    static auto prop_tag = string64("prop");
    ict::recurse(parent, [&](xddl::cursor self, xddl::cursor) {
        auto t = self->tag();
        if ((t == field_tag || t == prop_tag) && has_anon_type(self)) self->v->vset_ref(--self.end());
    });
}

void link_reflective_properties(xddl::cursor doc_root) {
    // create a set of names for the globals
    auto globs = std::set<std::string>();

    // add exports
    auto x = find(doc_root, "export", tag_of);
    if (x != doc_root.end()) for (auto & prop : x) globs.insert(prop.name());
    if (globs.empty()) return;

    ict::recurse(doc_root, [&](xddl::cursor self, xddl::cursor) {
        auto & e = elem_of(self);
        if (e.tag() == "field" || e.tag() == "prop" || e.tag() == "setprop") {
            if (globs.find(e.name()) != globs.end()) e.flags.set(element::global_flag);
        }
    });
}

// set a flag of all descendents
template <typename Cursor, typename T>
void set_flag(Cursor parent, T flag) {
    recurse(parent, [&](xddl::cursor self, xddl::cursor) {
        self->flags.set(flag);
    });
}

void oob::vend_handler(xddl::cursor self, xddl &) {
    set_flag(self, element::oob_flag);
    promote_last(self.parent());
}

void per::vend_handler(xddl::cursor self, xddl &) {
    set_flag(self, element::per_flag);
    promote_last(self.parent());

}

void xddl_root::vend_handler(xddl::cursor self, xddl & parser) {
    create_url_map<recdef>(self, parser.recdef_map, "record");
    create_url_map<type>(self, parser.type_map, "type");

    link_local_refs(self, parser.recdef_map, parser.type_map);

    link_anon_types(self);

    link_reflective_properties(self);

    //IT_WARN("verifying " << parser.file);
    //ict::verify(parser.ast);
    //IT_WARN("ok.");
}


// runtime parsing
template<typename SpecCursor, typename MessageCursor, typename B>
inline void add_extra(SpecCursor self,  MessageCursor parent, B & bs) {
    if (!bs.remaining()) return;
    parent.emplace_back(node::extra_node, self, bs.read(bs.remaining()));
}

inline void parse_children(xddl::cursor self, message::cursor parent, ibitstream & bs) {
    if (parent.empty()) parent.reserve(self.size());
    bitmarker mk{bs};
    for (auto c = self.begin(); c != self.end(); ++c) {
        parse(c, parent, bs);
    }
}

// the default vparse is just to parse the children (<xddl> for example)
void var_type::vparse(xddl::cursor self, message::cursor parent, ibitstream & bs) const {
    parse_children(self, parent, bs);
}

void xddl_root::vparse(xddl::cursor self, message::cursor parent, ibitstream & bs) const {
    auto st = find(self, "start", tag_of);
    if (st == self.end()) IT_PANIC("no <start> element in " << self->parser->file);
    parse(st, parent, bs);
    add_extra(self, parent, bs);
}

template <typename Parser>
void parse_ref(xddl::cursor self, message::cursor parent, ibitstream &bs, xddl::cursor & ref,
    const url& href, Parser parser) {
    try {
        if (ref == self.end()) {
            ref = parser->open("record", href);
        }
        parse_children(ref, parent, bs);
    } catch (ict::exception & e) {
        auto n = parent.emplace(node::error_node, self);
        std::ostringstream os;
        os << e.what() << " [" << n->file() << ":" << n->line() << "]";
        n->desc = os.str();
        IT_WARN("caught exception: " << n->desc);
    }
}

void fragment::vparse(xddl::cursor self, message::cursor parent, ibitstream &bs) const {
    parse_ref(self, parent, bs, ref, href, self->parser);
}

void jump::vparse(xddl::cursor self, message::cursor parent, ibitstream &bs) const {
    try { 
        // get the field this jump is based on
        auto c = rfind(leaf(parent), base); // c is a field node
        if (c.is_root()) IT_PANIC("cannot find " << base);

        auto elem = c->elem;

        // get the type of the field
        auto f = std::dynamic_pointer_cast<field>(elem->v);
        if (!f) IT_PANIC(base << " is not a field");

        if (f->ref == elem.end()) f->ref = elem->parser->open("type", f->href);

        // f->ref is the type cursor

        // get the item out of the type with this value (or range)
        auto t = std::dynamic_pointer_cast<type>(f->ref->v);
        auto & info = t->item_info(c->value());

        parse_ref(f->ref, parent, bs, info.ref, info.href, self->parser);

    } catch (ict::exception & e) {
        IT_WARN(e.what());
    }
}

void recref::vparse(xddl::cursor self, message::cursor parent, ibitstream &bs) const {
    auto rec = parent.emplace(node::record_node, self);
    constraint ct(bs, length.value(rec));
    parse_ref(self, rec, bs, ref, href, self->parser);
    if (!length.empty()) add_extra(self, parent, bs);
}

void record::vparse(xddl::cursor self, message::cursor parent, ibitstream & bs) const {
    auto rec = parent.emplace(node::record_node, self);
    constraint ct(bs, length.value(rec));
    parse_children(self, rec, bs);
    if (!length.empty()) add_extra(self, parent, bs);
}

void pad::vparse(xddl::cursor self, message::cursor parent, ibitstream &bs) const {
    size_t l = mod - ((bs.tellg() - bs.last_mark() - offset) % mod);
    if (l > 0 && l < mod) {
        l = std::min(l, bs.remaining());
        if (l) parent.emplace_back(node::field_node, self, bs.read(l));
    }
}

void peek::vparse(xddl::cursor self, message::cursor parent, ibitstream &bs) const {
    auto l = length.value(leaf(parent));
    auto bits = bs.peek(l, offset);
    parent.emplace_back(node::prop_node, self, bits);
}

void prop::vparse(xddl::cursor self, message::cursor parent, ibitstream &) const {
    auto v = value.value(leaf(parent));
    parent.emplace_back(node::prop_node, self, ict::from_integer(v));
}

#if 0
inline message::cursor get_variable(const std::string & name, message::cursor context) {
    auto first = ict::rfind(context, name);
    if (!first.is_root()) return first;

    // first is now pointing at message root
    auto globs = first.begin();
    auto g = find(globs, name);
    if (g == globs.end()) {
        auto xddl_root = ict::get_root(context->elem).begin();
        try {
            g = create_global(xddl_root, globs, name);
        } catch (ict::exception & e) {
            std::ostringstream os;
            os << e.what() << " [" << context->file() << ":" << context->line() << "]";
            ict::exception e2(os.str());
            throw e2;
        }
    }
    return g;
}
#endif

// same as get_variable but filtered on props only (lambda param?)
message::cursor get_prop(message::cursor first, const std::string & name) {
    auto r = message::ascending_cursor(first);
    while (!r.is_root()) {
        if (r->name() == name && r->type == node::prop_node) return r;
        ++r;
    }
    if (!r.is_root()) return r;

    auto globs = message::cursor(r).begin();
    auto g = find(globs, name);
    if (g != globs.end()) return g;

    return r; // just return root
}

void setprop::vparse(xddl::cursor self, message::cursor parent, ibitstream &) const {
    auto v = value.value(leaf(parent));
    auto c = parent.emplace(node::set_prop_node, self, ict::from_integer(v));
    auto i = get_prop(previous(c), "Name");
    if (!i.is_root()) { 
        i->bits = c->bits;
        i->elem = self;
    }
    if (c->elem->flags.test(element::global_flag)) set_global(self, c, self);
}

void field::vparse(xddl::cursor self, message::cursor parent, ibitstream & bs) const {
    if (size_t l = std::max((int64_t) 0, length.value(leaf(parent)))) {
        auto c = parent.emplace(node::field_node, self, bs.read(l));
        if (c->bits.bit_size() < l) c->type = node::incomplete_node;
        if (c->elem->flags.test(element::global_flag)) set_global(self, c, self);
    }
}

void xif::vparse(xddl::cursor self, message::cursor parent, ibitstream & bs) const {
    if (expr.value(leaf(parent)) != 0) parse_children(self, parent, bs);
}

inline message::cursor add_repeat_record(xddl::cursor self, message::cursor parent, ibitstream & bs) {
    auto rec = parent.emplace(node::repeat_record_node, self);
    parse_children(self, rec, bs);
    return rec;
}

void repeat::vparse(xddl::cursor self, message::cursor parent, ibitstream & bs) const {
    auto rec = parent.emplace(node::repeat_node, self);
    while (bs.remaining() > (size_t) minlen) add_repeat_record(self, rec, bs);
}

void num_repeat::vparse(xddl::cursor self, message::cursor parent, ibitstream & bs) const {
    auto num_value = num.value(leaf(parent));
    if (num_value > 0) {
        auto rep = parent.emplace(node::repeat_node, self);
        rep.reserve(num_value);
        for (int64_t i = 0; i < num_value; ++i) add_repeat_record(self, rep, bs);
    }
}

void bound_repeat::vparse(xddl::cursor self, message::cursor parent, ibitstream & bs) const {
    auto lb = min.value(leaf(parent));
    auto ub = max.value(leaf(parent));
    auto rep = parent.emplace(node::repeat_node, self);
    auto start_index = bs.tellg(); // needed for testing against minimum
    int64_t count = 0;

    // add the minimum for sure
    while ((count < lb) && ((int64_t)((bs.tellg() - start_index)) < minlen)) {
        add_repeat_record(self, rep, bs);
        ++count;
    }

    // add up to maximum as long as there are bits remaining
    while ((count < ub) && bs.remaining() > (size_t) minlen) {
        add_repeat_record(self, rep, bs);
        ++count;
    }
}

void xswitch::vparse(xddl::cursor self, message::cursor parent, ibitstream & bs) const {
    auto val = expr.value(leaf(parent));
    auto i = cases.find(val);
    if (i != cases.end()) {
        auto case_rec = self.begin() + i->second;
        while (case_rec.empty()) ++case_rec; // fall-through of empty cases
        if (case_rec != self.end()) parse_children(case_rec, parent, bs);
    }
    else if (has_default) parse_children(self.end() - 1, parent, bs);
}

void xwhile::vparse(xddl::cursor self, message::cursor parent, ibitstream & bs) const {
    auto rec = parent.emplace(node::record_node, self);
    auto cont = expr.value(leaf(parent));
    while (cont) {
        auto r = add_repeat_record(self, rec, bs);
        cont = expr.value(leaf(r));
    }
}

// member functions
type::~type() {
    if (l) ict::lua::lua_close(l);
}

type::item_data const & type::item_info(int64_t key) const {
    if (!items.empty()) {
        auto i = items.find(key);
        if (i!=items.end()) return i->second;
    }
    if (!ranges.empty()) {
        for (auto & range : ranges) {
            if (key >= range.first.first && key <= range.first.second) {
                return range.second;
            }
        }
    }
    IT_PANIC("invalid index to type: " << key);
}

std::string type::venum_string(xddl_cursor, msg_const_cursor c) const {
    //IT_WARN("type::venum_string");
    auto key = c->value();
    if (!items.empty()) {
        auto i = items.find(key);
        if (i!=items.end()) return i->second.value;
    }

    if (!ranges.empty()) {
        for (auto & range : ranges) {
            if (key >= range.first.first && key <= range.first.second) {
                return range.second.value;
            }
        }
    }
    return "invalid value";
}

std::string type::value(xddl_cursor, message::const_cursor c) const {
    ict::lua::lua_pushnumber(l, c->value());
    ict::lua::lua_setglobal(l, "key"); 

    // push the script back on the stack and call it
    ict::lua::lua_getglobal(l, "f");
    
    // set the node to a global for function to use
    ict::lua::lua_pushlightuserdata(l, &c);
    ict::lua::lua_setglobal(l, "node");

    int s = ict::lua::lua_pcall(l, 0, 0, 0);
    if (s) {
        // print error string and return it
        IT_WARN("error: " << ict::lua::lua_tostring(l, -1));
        std::string v = ict::lua::lua_tostring(l, -1);
        ict::lua::lua_pop(l, 1);
        return v;
    } else {
        ict::lua::lua_getglobal(l, "description");
        if (ict::lua::lua_isstring(l, -1)) {
            std::string v = ict::lua::lua_tostring(l, -1);
            ict::lua::lua_pop(l, 1);
            return v;
        } else {
            return "<script> must set description to a string";
        }
    }

    return std::string();
}

std::string type::vdescription(xddl::cursor referer, message::const_cursor c) const {
    if (l) return value(referer, c);
    return enum_string(referer, c);
}



template <typename T>
std::string get_description(const T * self_ptr, xddl::cursor self, message::const_cursor c) {
    if (!c->desc.empty()) return c->desc;
    if (self_ptr->href.empty()) return "";
    else if (self_ptr->ref == self.end()) {
        try { 
            self_ptr->ref = self->parser->open("type", self_ptr->href);
        } catch (ict::exception & e) {
            return e.what();
        }
    }
    return description(self_ptr->ref, self, c);
}

std::string field::vdescription(xddl::cursor self, message::const_cursor c) const {
    return get_description(this, self, c);
}

std::string field::venum_string(xddl::cursor self, message::const_cursor c) const {
    if (ref == self.end()) return "";
    else return ref->v->venum_string(self, c);
}

std::string prop::vdescription(xddl::cursor self, message::const_cursor c) const {
    return get_description(this, self, c);
}

std::string prop::venum_string(xddl::cursor self, message::const_cursor c) const {
    if (ref == self.end()) return "";
    else return ref->v->venum_string(self, c);
}

std::string setprop::vdescription(xddl::cursor self, message::const_cursor c) const {
    return get_description(this, self, c);
}

std::string setprop::venum_string(xddl::cursor self, message::const_cursor c) const {
    if (ref == self.end()) return "";
    else return ref->v->venum_string(self, c);
}

// node
ict::string64 node::tag() const { return elem_of(elem).tag(); }

std::string node::name() const { 
    switch (type) {
        case extra_node : return "extra";
        case repeat_record_node : return "record";
        case error_node : return "error";
        default: break;
    };
    return elem_of(elem).name(); 
}

int64_t node::value() const { 
    if (type == node::field_node) {
        if (auto f = get_ptr<field>(elem->v)) {
            auto b = f->bias;
            return to_integer<int64_t>(bits) + b; 
        }
    }
    return to_integer<int64_t>(bits); 
}

size_t node::line() const {return elem->line; }
std::string node::file() const { return elem->parser->file; }
xddl::cursor xddl::open(string64 tag, const url & href) { return owner->add_spec(tag, file, href); }

xddl::cursor spec::add_spec(string64 tag, const std::string & file, const url & href) {
    auto url = ict::relative_url(file, href);
    auto full = url.path + url.file;
    // TODO create these tag constants automatically in xddl.h
    static auto type_tag = string64("type");
    static auto record_tag = string64("record");

    xddl::cursor root;
    auto i = std::find_if(doms.begin(), doms.end(), [&](const xddl & dom){ return dom.file == full;} );
    if (i == doms.end()) {
        // not found, let's load the spec
        add_spec(full);
        root = doms.back().ast.root();
        // file was found but not even the <xddl> tag was correct
        if (root.empty()) IT_THROW("invalid root node: " << href); 
    } else {
        root = i->ast.root();
    }
    
    auto p = root.begin()->parser;
    if (tag == type_tag) { 
        auto i = p->type_map.find(href.anchor);
        if (i != p->type_map.end()) return i->second;
    } else if (tag == record_tag) {
        auto i = p->recdef_map.find(href.anchor);
        if (i != p->recdef_map.end()) return i->second;
    }
    IT_THROW("cannot locate anchor: " << href);
}

#if 0
spec::spec(const spec & b) : xddl_path(b.xddl_path), doms(b.doms) {
    for (auto & d : doms) {
        d.recdef_map.clear();
        d.type_map.clear();
        d.owner = this;
        auto doc_root = d.ast.root().begin(); // the <xddl> element
        auto v = std::dynamic_pointer_cast<xddl_root>(doc_root->v);
        assert(v);
        v->vend_handler(doc_root, d);
    }
}

spec& spec::operator=(const spec & b) {
    doms.clear();

}
#endif

// algos
std::ostream& operator<<(std::ostream& os, const node & n) {
    auto v = to_integer<int64_t>(n.bits);
    os << n.name() << " " << n.bits.bit_size() << " " << v << " " << std::hex << v << " (" << n.mnemonic() << ")" <<
        std::dec;
    return os;
}

std::string to_string(const node & n) {
    std::ostringstream os;
    os << n;
    return os.str();
}

} // namespace

