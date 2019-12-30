#include "xspx_parser.h"
#include <xenon/recref.h>

elem_type merge_elems(const elem_type &base, const elem_type &b) {
    elem_type dest = base;
    dest.tag = b.tag;
    dest.isa = b.isa;

    // mark all the dest attributes as hidden so they don't show in docs
    // merge the attributes
    auto dest_atts = b.attributes;
    for (auto a : base.attributes) {
        // a.hidden = true;
        dest_atts.push_back(a);
    }
    std::stable_sort(dest_atts.begin(), dest_atts.end());
    auto last = std::unique(dest_atts.begin(), dest_atts.end());
    dest_atts.erase(last, dest_atts.end());
    dest.attributes = dest_atts;

    // TODO: merge the children vector as well (if ever needed)
    return dest;
}

xsp_parser::xsp_parser() {
    elems.emplace_back(); // add the initial list of all elements

    // validate
    p.root_tag("xspec");
    p.add_children("xspec", {"nspace", "root", "base", "header", "type",
                             "element", "choice", "group", "code", "parser"});
    p.add_children("element", {"att", "child", "class", "constr", "display",
                               "start", "end", "public", "group", "code"});
    p.add_children("choice", {"element", "group"});
    p.add_children("group", {"child"});
    p.add_children("parser", {"name", "code"});
    p.add_children("type", {"name", "desc"});

    // handlers
    p.cdata_handler([&](const char *cdata) { this->cdata += cdata; });

    p.end_handler("nspace", [&] {
        name_space = ict::normalize(cdata);
        uc_name_space = ict::uppercase(name_space);
        cdata.clear();
    });

    p.end_handler("base", [&] {
        base = ict::normalize(cdata);
        cdata.clear();
    });

    p.start_handler("att", [&](const xenon::att_list &atts) {
        auto a = atts_.emplace(atts_.end());
        a->type_name = find_required_att(atts, "type");
        a->fixed = find_att(atts, "fixed");
        a->member_name = find_att(atts, "member");
        a->local = xenon::to_boolean(find_att(atts, "local", "true"));
        a->required = xenon::to_boolean(find_att(atts, "req", "false"));
        a->def = find_att(atts, "def");
        auto &ct = xenon::find_by_name(custom_types, a->type_name);
        if (a->def.empty())
            a->def = ct.def;
    });

    p.end_handler("att", [&] {
        auto &a = atts_.back();
        a.name = ict::normalize(cdata);
        if (a.member_name.empty())
            a.member_name = a.name.c_str();
        cdata.clear();
    });

    p.start_handler("type", [&](const xenon::att_list &atts) {
        auto i = custom_types.emplace(custom_types.end());
        i->def = find_att(atts, "def");
        i->cpp_name = find_required_att(atts, "cpp");
        i->cpp_func = find_att(atts, "func");
    });

    p.end_handler("type", [&] {
        auto &a = custom_types.back();
        a.name = xspx::pop_last(names);
        a.desc = xspx::pop_last(desc);
        if (a.cpp_func.empty())
            a.cpp_func = "create_" + a.name;
        cdata.clear();
    });

    p.start_handler("choice", [&](const xenon::att_list &atts) {
        choices.emplace_back();
        choices.back().tag = find_required_att(atts, "tag");
        choices.back().name = find_required_att(atts, "tag");
        elems.emplace_back(); // add a new list of elements for this choice
    });

    p.end_handler("choice", [&] {
        if (elems.back().empty())
            IT_PANIC("empty <choice>");
        choices.back().elems = elems.back();
        choices.back().children = children;
        children.clear();
        elems.pop_back();
        // set the tag for all the elements to the choice's tag
        for (auto &e : choices.back().elems)
            e.tag = choices.back().tag;
    });

    p.end_handler("class", [&] { cdata.clear(); });

    p.end_handler("header", [&] {
        head += ict::normalize(cdata);
        cdata.clear();
    });

    p.start_handler("code", [&](const xenon::att_list &atts) {
        curr_code_atts =
            xenon::find_exclusive_att<std::string>(atts, "id", "href");
    });

    p.end_handler("code", [&] {
        if (!curr_code_atts.first.empty()) { // id
            code_refs[curr_code_atts.first] += cdata;
        } else { // href, adding code to current element
            auto &e = elems.back().back();
            e.pub_code += code_refs[curr_code_atts.second];
        }
        cdata.clear();
    });

    p.start_handler("element", [&](const xenon::att_list &atts) {
        elems.back().emplace_back();
        auto &e = elems.back().back();
        e.tag = find_att(atts, "tag");
        e.name = find_att(atts, "name", e.tag.c_str());
        e.end_handler =
            xenon::to_boolean(find_att(atts, "end_handler", "false"));
        e.has_stack = xenon::to_boolean(find_att(atts, "stack", "false"));
        e.has_cdata = xenon::to_boolean(find_att(atts, "has_cdata", "false"));
        e.is_mod = xenon::to_boolean(find_att(atts, "is_mod", "false"));
        e.isa = find_att(atts, "isa");
    });

    p.end_handler("name", [&] {
        names.push_back(cdata);
        cdata.clear();
    });
    p.end_handler("desc", [&] {
        desc.push_back(cdata);
        cdata.clear();
    });

    p.end_handler("parser", [&] { class_name = xspx::pop_last(names); });

    p.end_handler("root", [&] {
        root = ict::normalize(cdata);
        cdata.clear();
    });

    p.end_handler("element", [&] {
        auto &e = elems.back().back();
        std::sort(children.begin(), children.end());
        e.children = children;
        e.attributes = atts_;
        e.is_base = (e.name == base);

        std::sort(e.attributes.begin(), e.attributes.end());
        if (!e.isa.empty()) {
            // find the base type and make a copy.
            auto i = std::find_if(
                elems.back().begin(), elems.back().end(),
                [&](elem_type &elem) { return elem.name == e.isa; });
            if (i == elems.back().end())
                IT_PANIC("base element " << e.isa << " not defined");
            e = merge_elems(*i, e);
        }

        e.group_hrefs = group_hrefs;
        group_hrefs.clear();

        children.clear();
        atts_.clear();
    });

    p.end_handler("child", [&] {
        children.push_back(ict::normalize(cdata));
        cdata.clear();
    });

    p.end_handler("display", [&] {
        auto &e = elems.back().back();
        e.display = ict::normalize(cdata);
        cdata.clear();
    });

    p.end_handler("public", [&] {
        auto &e = elems.back().back();
        e.pub_code += cdata;
        cdata.clear();
    });

    p.end_handler("start", [&] {
        auto &e = elems.back().back();
        e.start_code += cdata;
        cdata.clear();
    });

    p.end_handler("end", [&] {
        auto &e = elems.back().back();
        e.end_code += cdata;
        cdata.clear();
    });

    p.start_handler("group", [&](const xenon::att_list &atts) {
        auto p = xenon::find_exclusive_att<std::string>(atts, "name", "href");

        if (!p.first.empty()) { // "name"
            in_group_def = true;
            auto g = groups.emplace(groups.end());
            g->name = p.first;
        } else { // href
            in_group_def = false;
            xenon::recref url(p.second.c_str());
            if (url.anchor.empty())
                IT_PANIC("invalid href: " << p.second);
            group_hrefs.push_back(
                std::string(url.anchor.begin() + 1, url.anchor.end()));
            auto i = xenon::find_by_name(groups.begin(), groups.end(),
                                         group_hrefs.back());
            if (i == groups.end())
                IT_PANIC("cannot find group with name " << p.second);
            children.insert(children.end(), i->children.begin(),
                            i->children.end());
        }
    });

    p.end_handler("group", [&] {
        if (in_group_def) {
            if (children.empty())
                groups.pop_back();
            else {
                std::sort(children.begin(), children.end());
                groups.back().children = children;
                children.clear();
            }
        }
    });
    p.end_handler("xspec", [&] {
        int i = 1;
        xspx::for_each_element(*this, [&](elem_type &x) { x.uid = i++; });
    });
}

template <typename T>
std::string code_seg(const T &refs, const std::string &name) {
    auto seg = refs.find(name);
    if (seg != refs.end())
        return seg->second;
    return "";
}

template <typename S, typename T>
// S is an output stream
// T is an iterator to sorted elem_type vector
void generate_uid_constants(S &os, T first, T last) {
    std::for_each(first, last, [&](decltype(*first) &x) {
        os << "const uint8_t " << x.name << "_uid = " << x.uid << ";\n";
    });
}

void xsp_parser::header(std::ostream &h, st::type t) const {
    std::ostringstream os;
    os << "#pragma once //\n";
    os << code_seg(code_refs, "head");

    os << "namespace " << name_space << " {";

    os << "struct spec;";
    auto f = xspx::unique_elems(*this);
    for (auto &x : f)
        os << "struct " << x.name << ";";

    generate_uid_constants(os, f.begin(), f.end());

    for (const auto &elem : elems.back())
        to_decl(os, elem, root);

    for (const auto &x : choices) {
        for (const auto &elem : x.elems)
            to_decl(os, elem, root);
    }

    os << "}";

    os << parser_impl(t);

    os << code_seg(code_refs, "tail");

    xenon::cpp_code code;
    code.add(os.str());
    h << code.str();
}

void xsp_parser::to_stream(std::ostream &h) const {
    header(h, st::header_decl);
}

void xsp_parser::src_to_stream(std::ostream &s) const {
    std::ostringstream os;
    xenon::cpp_code code;
    os << "#include <xenon/xenon.h>";
    os << "namespace xenon {";
    parser_const(os, st::source_impl);
    os << "}";
    code.add(os.str());
    s << code.str();
}

std::ostream &xsp_parser::to_decl(std::ostream &os, const elem_type &elem,
                                  const std::string & /* root */) const {
    if (!elem.isa.empty())
        return os;

    os << "struct " << elem.name;
    if (!elem.is_base)
        os << " : public element::var_type";

    os << " {";
    if (elem.is_base) {
        os << "struct var_type;";
        // os << "template <typename T> struct model;";
        os << "inline " << elem.name << "();";
        os << "inline " << elem.name
           << "(std::shared_ptr<var_type> v, ict::string64 tag, const "
              "std::string & name = \"\""
           << ");";
        os << "template <typename T> \n";
        os << "inline " << elem.name
           << "(T x, ict::string64 tag, const std::string & name = \"\""
           << ");";
    } else {
        auto plist = to_param_list(elem.attributes, custom_types);
        if (!plist.empty()) {
            os << elem.name << "(" << plist << ")";
            os << ": " << to_init_list(elem) << "{}";
        }
    }
    // if (elem.end_handler) os << "void end_handler(" << root << "_cursor self,
    // " << root << " & parser);";

    if (elem.is_base) {
        os << "ict::string64 tag() const { return tag_;}";
        os << "std::string name() const { return name_.empty() ? tag_.c_str() "
              ": name_;}";
        os << "std::shared_ptr<var_type> v;";
        // os << "std::shared_ptr<model<var_type>> v2;";
        os << "size_t line = 0;" << class_name << " * parser = nullptr;"
           << "ict::string64 tag_;"
           << "std::string name_;";
    }
    for (const auto &a : elem.attributes)
        to_code(os, a, custom_types);

    os << elem.pub_code;
    os << "};"; // end of class

    if (elem.is_base) {
        os << "typedef ict::multivector<element>::cursor spec_cursor;";
        os << "typedef ict::multivector<element>::const_cursor "
              "spec_const_cursor;";
        os << "typedef ict::multivector<element>::ascending_cursor "
              "spec_ascending_cursor;";
        os << "}";
        os << "#include <xenon/node.h>";
        os << "namespace xenon {";
        os << "struct element::var_type {" << code_seg(code_refs, "var_type")
           << "};";
        // os << "template <typename T> struct element::model :
        // element::var_type {" << code_seg(code_refs, "model") << "};";

        os << R"(
        inline element::element() : v(std::make_shared<var_type>()) {}
        inline element::element(std::shared_ptr<var_type> v, ict::string64 tag, const std::string & name) : 
            v(v), tag_(tag), name_(name) {}
        template <typename T> 
        inline element::element(T x, ict::string64 tag, const std::string & name) : 
            v(std::make_shared<T>(x)), 
            tag_(tag), name_(name) { }
        )";
    }

    return os;
}

void xsp_parser::parser_destructor(std::ostream &) const {}

void xsp_parser::att_param(std::ostream &os, const std::string &start,
                           const xml_att_type &att) const {
    auto &ct = xenon::find_by_name(custom_types, att.type_name);
    os << ct.cpp_func;
    if (att.required) {
        os << "(" << start << ", find_required_att(atts, " << qt(att.name)
           << "))";
    } else {
        os << "(" << start << ", find_att(atts, " << qt(att.name) << ", "
           << qt(att.def) << "))";
    }
}

std::ostream &xsp_parser::start_handler_contents(std::ostream &os,
                                                 const elem_type &elem) const {
    os << "validate_att_list(atts, {";
    for (auto i = elem.attributes.begin(); i != elem.attributes.end(); ++i) {
        os << "\"" << i->name << "\"";
        if (i != elem.attributes.end())
            os << ", ";
    }
    os << "});";

    if (!elem.is_mod) {
        std::vector<std::string> params;

        os << "auto parent = parents.back();";
        for (const auto &att : elem.attributes) {
            if (att.local) {
                std::ostringstream param;
                att_param(param, "first", att);
                params.push_back(param.str());
            }
        }
        if (!params.empty())
            os << "auto first = leaf(parent);";

        os << "auto c = parent.emplace(" << elem.name << "("
           << ict::join(params.begin(), params.end(), ", //;") << "), "
           << qt(elem.tag);

        params.clear();
        for (const auto &att : elem.attributes) {
            if (!att.local) {
                std::ostringstream param;
                att_param(param, "first", att);
                params.push_back(param.str());
            }
        }
        if (!params.empty())
            os << ", " << ict::join(params.begin(), params.end(), ", //;");
        os << ");";
        params.clear();

        os << "c->uid = " << elem.name << "_uid;";
        os << R"(
            c->parser = this;
            c->line = p.line();
            parents.push_back(c);
            )";
        if (elem.has_stack)
            os << elem.name << "_stack.push_back(static_cast<" << elem.name
               << "*>(c->v.get()));";
        if (elem.has_cdata)
            os << "cdata.clear();";
    }
    os << elem.start_code;

    return os;
}

std::ostream &xsp_parser::to_init(std::ostream &os,
                                  const elem_type &elem) const {
    if (elem.is_base)
        return os;
    os << "p.start_handler(" << qt(elem.tag)
       << ", [&](const xenon::att_list &atts) {";

    start_handler_contents(os, elem);

    os << "});";

    os << "p.end_handler(" << qt(elem.tag) << ", [&]{";
    os << elem.end_code;
    if (elem.end_handler) {
        os << "end_handler(parents.back(), *this);";
    }
    if (!elem.is_mod)
        os << "parents.pop_back();";
    if (elem.has_stack)
        os << elem.name << "_stack.pop_back();";

    os << "});";
    return os;
}

std::ostream &xsp_parser::to_choice_init(std::ostream &os,
                                         const choice_type &choice) const {
    os << "p.start_handler(" << qt(choice.name)
       << ", [&](const xenon::att_list & atts) {";
    os << "switch (" << choice.name << "_test(atts)) {";
    for (size_t i = 0; i < choice.elems.size(); ++i) {
        os << "case " << i << " : {";
        start_handler_contents(os, choice.elems[i]);
        os << "break; }";
    }
    os << "default: IT_PANIC(\"unmatched choice\");";
    os << "}";
    os << "});";
    os << "p.end_handler(" << qt(choice.name)
       << ", [&]{ parents.pop_back(); });";

    return os;
}

std::string xsp_parser::parser_header() const {
    std::ostringstream os;
    os << "#pragma once\n"
       << "#include \"xddl.h\"";
    return os.str();
}

void xsp_parser::const_content(std::ostream &os) const {
    os << "{ ";
    os << "parents.push_back(ast.root());";
    os << "p.root_tag(" << qt(root) << ");";

    for (const auto &elem : elems.back())
        add_children(os, elem);

    for (const auto &choice : choices)
        add_children(os, choice);

    os << "p.cdata_handler([&](const char * cstr){cdata += cstr; });";

    for (const auto &elem : elems.back())
        to_init(os, elem);

    for (const auto &choice : choices)
        to_choice_init(os, choice);

    os << "}";
}
void xsp_parser::parser_const(std::ostream &os, st::type t) const {
    switch (t) {
    case st::header_decl:
        os << class_name << "();";
        break;
    case st::header_impl:
        os << class_name << "()";
        const_content(os);
        break;
    case st::source_impl:
        os << class_name << "::" << class_name << "()";
        const_content(os);
    }
}

std::string xsp_parser::parser_impl(st::type t) const {
    std::ostringstream os;
    os << "#include <xenon/xml_parser.h>\n"
       << head << "namespace " << name_space
       << " {"
          "struct "
       << class_name
       << " {"
          "public:";

    parser_const(os, t);
    os << class_name << "(const std::string & filename) : " << class_name
       << "() {"
       << "file = filename; p.open(filename); }";

    parser_destructor(os);

    os << R"(
    void clear() { p.reset(); ast.clear(); parents.resize(1); file = ""; }

    void open(const std::string & filename) 
        { clear();  file = filename; p.open(filename); }

    template <typename T>
    void open(T first, T last, const std::string & filename) { 
        clear(); file = filename; p.open(first, last, file); }

    )";

    os << "xml_parser p;";
    os << "std::string cdata;";
    os << "std::string file;";
    os << "typedef ict::multivector<" << base << "> multivector_type;";
    os << "typedef multivector_type::cursor cursor;";
    os << "typedef multivector_type::const_cursor const_cursor;";
    os << "typedef multivector_type::ascending_cursor ascending_cursor;";
    os << "multivector_type "
       << "ast;"
       << "std::vector<multivector_type::cursor> parents;";

    for (const auto &elem : elems.back()) {
        if (elem.has_stack)
            os << "std::vector<" << elem.name << "*>" << elem.name << "_stack;";
    }
    os << code_seg(code_refs, "class");

    os << "}; }";

    xenon::cpp_code code;
    code << os.str();
    return code.str();
}

namespace xspx {

ict::multivector<elem_type> elem_tree(const xsp_parser &xspx) {
    ict::multivector<elem_type> tree;
    std::vector<elem_type> elems = xspx.elems[0];

    // add the choices
    for (auto &choice : xspx.choices) {
        elem_type ch;
        ch.tag = choice.tag;
        elems.push_back(ch);
    }

    // now sort according to the tag
    std::sort(
        elems.begin(), elems.end(), [](elem_type const &a, elem_type const &b) {
            return std::string(a.tag.c_str()) < std::string(b.tag.c_str());
        });

    // now convert to a multivector
    for (auto &i : elems) {
        if (!i.is_base)
            tree.root().emplace_back(i);
    }

    // find the choices and add their subchildren
    for (auto &choice : xspx.choices) {
        auto c = std::find_if(tree.begin(), tree.end(),
                              [&](auto &e) { return e.tag == choice.tag; });
        for (auto &i : choice.elems) {
            c.emplace_back(i);
        }
    }
    return tree;
}

std::vector<elem_type> unique_elems(const xsp_parser &xspx) {
    auto v = std::vector<elem_type>();

    for_each_element(xspx, [&](const elem_type &x) { v.push_back(x); });

    std::sort(v.begin(), v.end(), [](elem_type const &a, elem_type const &b) {
        return a.name < b.name;
    });
    v.erase(std::unique(v.begin(), v.end()), v.end());

    return v;
}

} // namespace xspx
