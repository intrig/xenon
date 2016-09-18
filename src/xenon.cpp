//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <algorithm>
#include <ict/bitstring.h>
#include <xenon/xenon.h>
#include <xenon/node.h>
#include <xenon/xddl.h>
#include <xenon/ximsi.h>
#include <xenon/DateTime.h>
#include <set>

// xddl script
#include <xenon/lua.hpp>

namespace xenon {

const int DateTime::daysmonth[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
const int DateTime::daysmonthleap[13] = { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

template <typename State>
inline message::cursor get_cursor(State * L) {
    lua::lua_getglobal(L, "node");
    return *static_cast<message::cursor *>(lua_touserdata(L, -1));
}

// get a timestamp from a .Net encoded date (don't ask)
static int script_datetime(lua::lua_State *L) {
    auto n = get_cursor(L);
    auto value = ict::to_integer<int64_t>(n->bits);
    // TODO: scripts should just throw exceptions and not return error codes
    try { 
        auto dt = DateTime(value);
        std::ostringstream os;
        os << dt;
        auto s = os.str();
        lua::lua_pushstring(L, s.c_str());
    } catch (std::exception & e) {
        lua::lua_pushstring(L, e.what());
    }
    return 1;
}

// get the description of a previous node
static int script_Description(lua::lua_State *L) {
    const char * s = lua::luaL_checklstring(L, 1, NULL);
    auto n = get_cursor(L);
    auto c = rfind_first(n, s);
    if (c.is_root()) lua::lua_pushstring(L, "");
    else lua::lua_pushstring(L, description(c).c_str());
    return 1;
}

// get the enum string for this node
static int script_EnumValue(lua::lua_State *L) {
    auto n = get_cursor(L);
    lua::lua_pushstring(L, n->elem->v->venum_string(n->elem, n).c_str());
    return 1;
}

#ifdef _MSC_VER
#pragma warning(once : 4244)
#endif

// get the value of a previous node
static int script_Value(lua::lua_State *L) {
    const char * s = lua::luaL_checklstring(L, 1, NULL);
    auto n = get_cursor(L);
    auto c = rfind_first(n, s);
    if (c.is_root()) lua::lua_pushnumber(L, 0);
    else lua::lua_pushnumber(L, c->value());
    return 1;
}

static int script_Slice(lua::lua_State *L) {
    int index = static_cast<int>(lua::luaL_checknumber(L, 1));
    int length = static_cast<int>(lua::luaL_checknumber(L, 2));
    auto n = get_cursor(L);
    auto subs = n->bits.substr(index, length);
    auto num = ict::to_integer<int64_t>(subs);
    lua::lua_pushnumber(L, num);
    return 1;
}

static int script_TwosComplement(lua::lua_State * L) {
    auto n = get_cursor(L);
    auto value = ict::to_integer<int>(n->bits);
    value <<= (32 - n->bits.bit_size());
    value >>= (32 - n->bits.bit_size());
    lua::lua_pushnumber(L, value);
    return 1;
}

static int script_Ascii(lua::lua_State * L) {
    auto & bits = get_cursor(L)->bits;
    auto s = std::string(bits.begin(), bits.bit_size() / 8);
    for (auto & c : s) if (!isprint(c)) c = '.';
    lua::lua_pushstring(L, s.c_str());
    return 1;
}

static int script_Imsi_S(lua::lua_State * L) {
    auto & bits = get_cursor(L)->bits;
    lua::lua_pushstring(L, decode_imsi(bits).c_str());
    return 1;
}

static int script_Gsm7(lua::lua_State * L) {
    int fill = 0;
    const char * s = lua::luaL_checklstring(L, 1, NULL);
    if (s) {
        auto c = rfind_first(get_cursor(L), s);
        if (!c.is_root()) fill = ict::to_integer<int>(c->bits); 
    }
    std::string sms = ict::gsm7(get_cursor(L)->bits, fill);
    lua::lua_pushstring(L, sms.c_str());
    return 1;
}

static int script_find(lua::lua_State *L) {
    auto n = ict::get_root(get_cursor(L));
    const char * s = luaL_checkstring(L, 1);

    auto c = find_first(n, s);
    if (c == n.end()) {
        // IT_WARN(s << " not found");
        lua_pushstring(L, "");
        return 1;
    }
    // IT_WARN("found " << s << ": " << *c);
    lua_pushstring(L, description(c).c_str());
    return 1;
}

static int script_Search(lua::lua_State *) {
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
void xcase::vend_handler(spec::cursor, spec & parser) {
    auto & xs = parser.xswitch_stack.back();
    if (xs->has_default) IT_PANIC("<default> must be the last case");
    auto i = xs->cases.size(); // get the index of the current case
    auto it = xs->cases.find(value);
    if (it != xs->cases.end()) {
        IT_PANIC("duplicate case value found for " << value);
    }
    xs->cases[value] = i;
}

type create_type_struct(recref url, spec::cursor parent) {
    std::shared_ptr<lua::state_wrapper> l = 0;
    std::map<int, type::item_data> items;
    std::map<std::pair<int, int>, type::item_data> ranges;

    for (auto c = parent.begin(); c!= parent.end(); ++c) {
        auto t = c->tag();
        if (t == "script") {
            if (l) IT_PANIC("multiple scripts in type definition");
            l = get_ptr<script>(c->v)->l;
        }
        else if (t == "item") {
            auto item = get_ptr<xitem>(c->v);
            auto i = items.find(item->key);
            if (i != items.end()) IT_PANIC("<item> \"" << item->value << "\" with key " << 
                item->key << " already defined");
            items[item->key] = type::item_data { item->value, parent.end(), item->href };
        }
        else if (t == "range") {
            auto r = get_ptr<range>(c->v);
            if (r->start > r->end) IT_PANIC("invalid <range>");
            auto key = std::make_pair(r->start, r->end);
            auto i = ranges.find(key);
            if (i != ranges.end()) IT_PANIC("<range> already defined");
            ranges[key] = type::item_data { r->value, parent.end(), r->href };
        }
    }
    return type(url, l, items, ranges);
}
inline bool has_anon_type(spec::cursor self) {
    if (self.empty()) return false;
    for (auto first = self.begin(); first != self.end(); ++first) {
        if (first->tag() != "comment") return true;
    }
    return false;
}

void field::vend_handler(spec::cursor self, spec &parser) {
    if (has_anon_type(self)) {
        auto mv = ict::multivector<element>(self); // copy into temp multivector
        self.clear();

        href= "anon";
        auto e = self.emplace(create_type_struct("anon", mv.root()), "type", "anon");
        e->parser = &parser;
        e->line = self->line;
    }
}

void xdefault::vend_handler(spec::cursor, spec & parser) {
    auto & xs = parser.xswitch_stack.back();
    xs->has_default = true;
}

template <typename Op> 
void reg_func(lua::lua_State * state, Op func, const char * name) {
    lua::lua_pushcfunction(state, func);
    lua::lua_setglobal(state, name);
}

void script::vend_handler(spec::cursor, spec & dom) {
    auto p = lua::luaL_newstate();
    l = std::make_shared<lua::state_wrapper>(p);
    p = lua::get(l); // should be the same
    lua::luaL_openlibs(p);

    reg_func(p, script_Description, "Description");
    reg_func(p, script_datetime, "dot_net_time");
    reg_func(p, script_EnumValue, "EnumValue");
    reg_func(p, script_Value, "Value");
    reg_func(p, script_Slice, "slice");
    reg_func(p, script_TwosComplement, "TwosComplement");
    reg_func(p, script_Ascii, "ascii");
    reg_func(p, script_Gsm7, "gsm7");
    reg_func(p, script_Search, "search");
    reg_func(p, script_find, "find");
    reg_func(p, script_Imsi_S, "imsi_s");

    if (lua::luaL_loadstring(p, dom.cdata.c_str())) IT_PANIC(lua::lua_tostring(p, -1));

    // pop the compiled script off the stack and set it to the global "f"
    lua::lua_setglobal(p, "f");
}

void type::vend_handler(spec::cursor self, spec &parser) {
    auto mv = ict::multivector<element>(self); // copy into temp multivector
    self.clear();
    self->v = std::make_shared<type>(create_type_struct(id, mv.root()));
}

template <typename T, typename Cursor, typename Map>
void create_url_map(Cursor self, Map & m, ict::string64 tag) {
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
void link_ref(Cursor & self, recref & url, Map & m) {
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
    ict::recurse(parent, [&](spec::cursor self) {
        auto url = self->v->vhref();
        if (!url.empty()) {
            if (self->tag() == "fragment" || self->tag() == "record") link_ref(self, url, rec_map);
            else if (self->uid == field_uid || self->tag() == "prop" || self->tag() == "setprop") 
                link_ref(self, url, type_map);
        }

        else if (self->tag() == "type") link_type_refs(self, rec_map);
    });
}

void link_anon_types(spec::cursor parent) {
    ict::recurse(parent, [&](spec::cursor self) {
        auto t = self->uid;
        if ((t == field_uid || t == prop_uid) && has_anon_type(self)) self->v->vset_ref(--self.end());
    });
}

// return a first level child with the specified tag. 
template <typename Cursor>
Cursor find_child_with_tag(const Cursor & parent, std::string const & tag) {
    return std::find_if(parent.begin(), parent.end(), 
        [&](const element & c){ return c.tag() == tag; });
}

void link_reflective_properties(spec::cursor doc_root) {
    // create a set of names for the globals
    auto globs = std::set<std::string>();

    // add exports
    auto x = find_child_with_tag(doc_root, "export");
    if (x != doc_root.end()) for (auto & prop : x) globs.insert(prop.name());
    if (globs.empty()) return;

    ict::recurse(doc_root, [&](spec::cursor self) {
        auto & e = elem_of(self);
        if (e.uid == field_uid || e.tag() == "prop" || e.tag() == "setprop") {
            if (globs.find(e.name()) != globs.end()) e.flags.set(element::global_flag);
        }
    });
}

// set a flag of all descendents
template <typename Cursor, typename T>
void set_flag(Cursor parent, T flag) {
    recurse(parent, [&](spec::cursor self) {
        self->flags.set(flag);
    });
}

void oob::vend_handler(spec::cursor self, spec &) {
    set_flag(self, element::oob_flag);
    promote_last(self.parent());
}

void enc::vend_handler(spec::cursor self, spec &) {
    set_flag(self, element::enc_flag);
    promote_last(self.parent());

}

void xddl::vend_handler(spec::cursor self, spec & parser) {
    create_url_map<recdef>(self, parser.recdef_map, "record");
    create_url_map<type>(self, parser.type_map, "type");

    // If there is a start, then it gets its own record reference.
    auto st = find_child_with_tag(self, "start");
    if (st != self.end()) {
        parser.recdef_map["#start"] = st;
    }

    link_local_refs(self, parser.recdef_map, parser.type_map);

    link_anon_types(self);

    link_reflective_properties(self);
}


// runtime parsing
template<typename SpecCursor, typename MessageCursor, typename B>
inline void add_extra(SpecCursor self,  MessageCursor parent, B & bs) {
    if (!bs.remaining()) return;
    parent.emplace_back(node::extra_node, self, bs.read(bs.remaining()));
}

inline void parse_children(spec::cursor self, message::cursor parent, ict::ibitstream & bs) {
    if (parent.empty()) parent.reserve(self.size());
    ict::bitmarker mk{bs};
    for (auto c = self.begin(); c != self.end(); ++c) {
        parse(c, parent, bs);
    }
}

// the default vparse is just to parse the children 
void element::var_type::vparse(spec::cursor self, message::cursor parent, ict::ibitstream & bs) const {
    parse_children(self, parent, bs);
}

void xddl::vparse(spec::cursor self, message::cursor parent, ict::ibitstream & bs) const {
    auto st = find_child_with_tag(self, "start");
    if (st == self.end()) IT_PANIC("no <start> element in " << self->parser->file);
    parse(st, parent, bs);
}

void start::vparse(spec::cursor self, message::cursor parent, ict::ibitstream & bs) const {
    parse_children(self, parent, bs);
    add_extra(self, parent, bs);
}

template <typename Parser>
void parse_ref(spec::cursor self, message::cursor parent, ict::ibitstream &bs, spec::cursor & ref,
    const recref& href, Parser parser) {
    try {
        if (ref == self.end()) {
            auto url = relative_url(parser->file, href); // create an abs url.
            ref = get_record(*parser->owner, url);
        }
        parse_children(ref, parent, bs);
    } catch (std::exception & e) {
        auto n = parent.emplace(node::error_node, self);
        ict::osstream os;
        os << e.what() << " [" << n->file() << ":" << n->line() << "]";
        n->desc = os.take();
        IT_WARN("caught exception: " << n->desc);
    }
}

void fragment::vparse(spec::cursor self, message::cursor parent, ict::ibitstream &bs) const {
    parse_ref(self, parent, bs, ref, href, self->parser);
}

void jump::vparse(spec::cursor self, message::cursor parent, ict::ibitstream &bs) const {
    try { 
        // get the field this jump is based on
        auto c = rfind_first(leaf(parent), base); // c is a field node
        if (c.is_root()) IT_PANIC("cannot find " << base);

        auto elem = c->elem;

        // get the type of the field
        auto f = std::dynamic_pointer_cast<field>(elem->v);
        if (!f) IT_PANIC(base << " is not a field");

        if (f->ref == elem.end()) {
            auto url = relative_url(elem->parser->file, f->href); // create an abs url.
            f->ref = get_type(*elem->parser->owner, url);
        }

        // get the item out of the type with this value (or range)
        auto t = std::dynamic_pointer_cast<type>(f->ref->v);
        auto & info = t->item_info(c->value());

        if (!info.href.empty()) parse_ref(f->ref, parent, bs, info.ref, info.href, self->parser);

    } catch (std::exception & e) {
        IT_WARN(e.what());
    }
}

void reclink::vparse(spec::cursor self, message::cursor parent, ict::ibitstream &bs) const {
    auto rec = parent.emplace(node::record_node, self);
    auto l = length.value(rec);
    ict::constraint ct(bs, length.value(rec));
    parse_ref(self, rec, bs, ref, href, self->parser);
    if (!length.empty()) add_extra(self, rec, bs);
}

void record::vparse(spec::cursor self, message::cursor parent, ict::ibitstream & bs) const {
    auto rec = parent.emplace(node::record_node, self);
    ict::constraint ct(bs, length.value(rec));
    parse_children(self, rec, bs);
    if (!length.empty()) add_extra(self, parent, bs);
}

void pad::vparse(spec::cursor self, message::cursor parent, ict::ibitstream &bs) const {
    size_t l = mod - ((bs.tellg() - bs.last_mark() - offset) % mod);
    if (l > 0 && l < mod) {
        l = std::min(l, bs.remaining());
        if (l) parent.emplace_back(node::field_node, self, bs.read(l));
    }
}

void peek::vparse(spec::cursor self, message::cursor parent, ict::ibitstream &bs) const {
    auto l = length.value(leaf(parent));
    auto bits = bs.peek(l, offset);
    parent.emplace_back(node::prop_node, self, bits);
}

void prop::vparse(spec::cursor self, message::cursor parent, ict::ibitstream &) const {
    auto v = value.value(leaf(parent));
    auto c = parent.emplace(node::prop_node, self, ict::from_integer(v));
    c->set_visible(visible);
}

// same as get_variable but filtered on props only
message::cursor get_prop(message::cursor first, const std::string & name) {
    auto r = message::ascending_cursor(first);
    while (!r.is_root()) {
        if (r->name() == name && r->type == node::prop_node) return r;
        ++r;
    }
    if (!r.is_root()) return r;

    auto globs = message::cursor(r).begin();
    auto g = find_first(globs, name);
    if (g != globs.end()) return g;

    return r; // just return root
}

void setprop::vparse(spec::cursor self, message::cursor parent, ict::ibitstream &) const {
    auto v = value.value(leaf(parent));
    auto c = parent.emplace(node::setprop_node, self, ict::from_integer(v));
    auto i = get_prop(previous(c), "Name");
    if (!i.is_root()) { 
        i->bits = c->bits;
        i->elem = self;
    }
    if (c->elem->flags.test(element::global_flag)) set_global(self, c, self);
}

void field::vparse(spec::cursor self, message::cursor parent, ict::ibitstream & bs) const {
    if (size_t l = std::max((int64_t) 0, length.value(leaf(parent)))) {
        auto c = parent.emplace(node::field_node, self, bs.read(l));
        if (c->bits.bit_size() < l) c->set_incomplete();
        if (c->elem->flags.test(element::global_flag)) set_global(self, c, self);
    }
}

void cstr::vparse(spec::cursor self, message::cursor parent, ict::ibitstream & bs) const {
    auto c = parent.emplace(node::field_node, self, bs.read_to('\0'));
}

std::string cstr::vdescription(spec::cursor referer, message::const_cursor c) const {
    return std::string(c->bits.begin(), c->bits.end()-1);
}

void xif::vparse(spec::cursor self, message::cursor parent, ict::ibitstream & bs) const {
    if (expr.value(leaf(parent)) != 0) parse_children(self, parent, bs);
}

inline message::cursor add_repeat_record(spec::cursor self, message::cursor parent, ict::ibitstream & bs) {
    auto rec = parent.emplace(node::repeat_record_node, self);
    parse_children(self, rec, bs);
    return rec;
}

void repeat::vparse(spec::cursor self, message::cursor parent, ict::ibitstream & bs) const {
    auto rec = parent.emplace(node::repeat_node, self);
    while (bs.remaining() > (size_t) minlen) add_repeat_record(self, rec, bs);
}

void num_repeat::vparse(spec::cursor self, message::cursor parent, ict::ibitstream & bs) const {
    auto num_value = num.value(leaf(parent));
    if (num_value > 0) {
        auto rep = parent.emplace(node::repeat_node, self);
        rep.reserve(num_value);
        for (int64_t i = 0; i < num_value; ++i) add_repeat_record(self, rep, bs);
    }
}

void bound_repeat::vparse(spec::cursor self, message::cursor parent, ict::ibitstream & bs) const {
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

void xswitch::vparse(spec::cursor self, message::cursor parent, ict::ibitstream & bs) const {
    auto val = expr.value(leaf(parent));
    auto i = cases.find(val);
    if (i != cases.end()) {
        auto case_rec = self.begin() + i->second;
        while (case_rec.empty()) ++case_rec; // fall-through of empty cases
        if (case_rec != self.end()) parse_children(case_rec, parent, bs);
    }
    else if (has_default) parse_children(self.end() - 1, parent, bs);
}

void xwhile::vparse(spec::cursor self, message::cursor parent, ict::ibitstream & bs) const {
    auto rec = parent.emplace(node::record_node, self);
    auto cont = expr.value(leaf(parent));
    while (cont) {
        auto r = add_repeat_record(self, rec, bs);
        cont = expr.value(leaf(r));
    }
}

// member functions
type::~type() {
    if (l.unique() && lua::get(l)) lua::lua_close(lua::get(l));
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

std::string type::venum_string(spec::cursor, msg_const_cursor c) const {
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

std::string type::value(spec::cursor x, message::const_cursor c) const {
    auto p = lua::get(l);
    lua::lua_pushnumber(p, c->value());
    lua::lua_setglobal(p, "key"); 

    // push the script back on the stack and call it
    lua::lua_getglobal(p, "f");
    
    // set the node to a global for function to use
    lua::lua_pushlightuserdata(p, &c);
    lua::lua_setglobal(p, "node");

    int s = lua::lua_pcall(p, 0, 0, 0);
    if (s) {
        // print error string and return it
        IT_WARN("error: " << x->parser->file << ":" << x->line << " " << lua::lua_tostring(p, -1));
        std::string v = lua::lua_tostring(p, -1);
        lua::lua_pop(p, 1);
        return v;
    } else {
        lua::lua_getglobal(p, "description");
        if (lua::lua_isstring(p, -1)) {
            std::string v = lua::lua_tostring(p, -1);
            lua::lua_pop(p, 1);
            return v;
        } else {
            return "<script> must set description to a string";
        }
    }

    return std::string();
}

std::string type::vdescription(spec::cursor referer, message::const_cursor c) const {
    if (l) return value(referer, c);
    return enum_string(referer, c);
}



template <typename T>
std::string get_description(const T * self_ptr, spec::cursor self, message::const_cursor c) {
    if (!c->desc.empty()) return c->desc;
    if (self_ptr->href.empty()) return "";
    else if (self_ptr->ref == self.end()) {
        try { 
            auto url = relative_url(self->parser->file, self_ptr->href); // create an abs file path.
            self_ptr->ref = get_type(*self->parser->owner, url);
        } catch (std::exception & e) {
            return e.what();
        }
    }
    return description(self_ptr->ref, self, c);
}

std::string field::vdescription(spec::cursor self, message::const_cursor c) const {
    return get_description(this, self, c);
}

std::string field::venum_string(spec::cursor self, message::const_cursor c) const {
    if (ref == self.end()) return "";
    else return ref->v->venum_string(self, c);
}

std::string prop::vdescription(spec::cursor self, message::const_cursor c) const {
    return get_description(this, self, c);
}

std::string prop::venum_string(spec::cursor self, message::const_cursor c) const {
    if (ref == self.end()) return "";
    else return ref->v->venum_string(self, c);
}

std::string setprop::vdescription(spec::cursor self, message::const_cursor c) const {
    return get_description(this, self, c);
}

std::string setprop::venum_string(spec::cursor self, message::const_cursor c) const {
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
            return ict::to_integer<int64_t>(bits) + b; 
        }
    }
    return ict::to_integer<int64_t>(bits); 
}

size_t node::line() const {return elem->line; }
std::string node::file() const { return elem->parser->file; }

spec::cursor get_record(spec_server & spec, const recref & href) {
    auto full = href.path + href.file; // get the filename

    auto root = spec.add_spec(full);
    if (href.anchor.empty()) return root;
    auto p = root.begin()->parser;
    auto j = p->recdef_map.find(href.anchor);
    if (j != p->recdef_map.end()) return j->second;
    IT_PANIC("cannot locate anchor: " << href);
}

spec::cursor get_type(spec_server & spec, const recref & href) {
    auto full = href.path + href.file; // get the filename

    auto root = spec.add_spec(full);
    auto p = root.begin()->parser;
    auto j = p->type_map.find(href.anchor);
    if (j != p->type_map.end()) return j->second;
    IT_PANIC("cannot locate anchor: " << href);
}

// algos
std::ostream& operator<<(std::ostream& os, const node & n) {
    auto v = ict::to_integer<int64_t>(n.bits);
    os << n.name() << " " << n.bits.bit_size() << " " << v << " " << std::hex << v << " (" << n.mnemonic() << ")" <<
        std::dec;
    return os;
}

std::string to_string(const node & n) {
    std::ostringstream os;
    os << n;
    return os.str();
}

void to_html(spec::const_cursor self, std::ostream & os) {
    if (self->v) self->v->vto_html(self, os);
    else os << "(nullptr)";
}

std::string to_html(const spec & s) {
    std::ostringstream os;
    os << "<h2>" << s.file << "</h2><ul>";
    ict::recurse(s.ast.root(), 
        [&](spec::const_cursor self, int level) {
            auto space = ict::spaces(level * 2);
            os << space << "<li>";
            to_html(self, os);
            if (!self.empty()) os << "<ul>\n";
        },
        [&](spec::const_cursor self, int level) {
            auto space = ict::spaces(level * 2);
            os << space << "</li>";
            if (!self.empty()) os << "</ul>";
            os << "\n";
        });
    os <<"</ul>\n";
    return os.str();
}
        
void xexport::vto_html(spec::const_cursor self, std::ostream & os) const {
    os << "global properties";
}

void prop::vto_html(spec::const_cursor self, std::ostream & os) const {
    os << self->name() << " | " << value << " | " << href;
}

void type::vto_html(spec::const_cursor self, std::ostream & os) const {
    os << self->name();
}
} // namespace
