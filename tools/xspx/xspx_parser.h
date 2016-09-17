#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.

#include <xenon/cpp_code.h>

#include <vector>
#include <algorithm>
#include <sstream>

#include <xenon/xml_parser.h>
#include <ict/string64.h>

#define qt(x) "\"" << x << "\""

// an xml attribute
struct xml_att_type {
    ict::string64 name;
    std::string fixed; 
    std::string type_name; 
    std::string member_name;
    bool local = true;
    bool required = false;
    std::string def;
    friend bool operator<(const xml_att_type & a, const xml_att_type & b) {
        return a.name < b.name;
    }
    friend bool operator>(const xml_att_type & a, const xml_att_type & b) {
        return a.name > b.name;
    }
    friend bool operator==(const xml_att_type & a, const xml_att_type & b) {
        return a.name == b.name;
    }
    friend bool operator!=(const xml_att_type & a, const xml_att_type & b) {
        return !(a.name == b.name);
    }
};

typedef std::vector<xml_att_type> xml_att_list;

struct custom_type {
    std::string name;
    std::string cpp_name;
    std::string cpp_func;
    std::string def;
};

typedef std::vector<custom_type> custom_type_list;

// an xml element
struct elem_type {
    friend bool operator<(const elem_type & a, const elem_type & b) {
        return a.name < b.name;
    }
    friend bool operator>(const elem_type & a, const elem_type & b) {
        return a.name > b.name;
    }
    friend bool operator==(const elem_type & a, const elem_type & b) {
        return a.name == b.name;
    }
    friend bool operator!=(const elem_type & a, const elem_type & b) {
        return !(a.name == b.name);
    }
    ict::string64 tag; // the xml tag
    std::string name; // the cpp type, same as tag by defualt
    bool end_handler;
    bool has_stack;
    bool has_cdata;
    bool is_mod;
    std::vector<std::string> children;
    xml_att_list attributes;
    std::string isa; // inheritance, defaults to element
    std::string pub_code;
    std::string start_code;
    std::string end_code;

    // derived
    bool is_base = false;
    int uid = 0;
};

typedef std::vector<elem_type> elem_list;
typedef std::vector<elem_list> elem_stack;

struct group_type {
    std::string name;
    std::vector<std::string> children;
};

typedef std::vector<group_type> group_list;

struct choice_type {
    ict::string64 tag;
    std::string name;
    elem_list elems;
    std::vector<std::string> children;
};

typedef std::vector<choice_type> choice_list;

inline std::ostream& to_code(std::ostream& os, const xml_att_type & att, const custom_type_list & custom_types) {
    if (att.local) {
        auto ct = xenon::find_by_name(custom_types, att.type_name);
        os << ct.cpp_name << " " << att.member_name;
        if (!att.def.empty())  os << " = " << att.def;
        else if (!ct.def.empty()) os << " = " << ct.def;
        os  << ";";
    }
    return os;
}

inline std::string to_param_list(const xml_att_list & atts, const custom_type_list & custom_types) {
    std::vector<std::string> params;
    
    //params.push_back("string64 tag");
    for (const auto & att : atts) {
        if (att.local) {
            std::ostringstream s;
            auto ct = xenon::find_by_name(custom_types, std::string(att.type_name));
            s << ct.cpp_name << " " << att.member_name;
            params.push_back(s.str());
        }
    }
    return ict::join(params.begin(), params.end(), ", ");
}

inline std::string to_init_list(const elem_type & elem) {
    auto atts = elem.attributes;
    std::vector<std::string> params;
    std::vector<std::string> base_params;
    //base_params.push_back("tag");
    for (const auto & att : atts) {
        if (att.local) {
            std::ostringstream s;
            s << att.member_name << "(" << att.member_name << ")";
            params.push_back(s.str());
        } else {
            base_params.push_back(att.member_name);
        }
    }
    std::ostringstream s;
    //s << "element(" << ict::join(base_params.begin(), base_params.end(), ", ") << ")";
    if (!params.empty()) s << ict::join(params.begin(), params.end(), ", ");
    return s.str();
}



template <typename T>
inline std::ostream& add_children(std::ostream & os, const T & elem) {
    if (!elem.children.empty()) {
        os << "p.add_children(" << qt(elem.tag) << ", {\"" << 
        ict::join(elem.children.begin(), elem.children.end(), "\", \"") << "\"});";
    }
    return os;
}

elem_type merge_elems(const elem_type & a, const elem_type & b);

namespace st { // source type
enum type {
    header_decl,
    header_impl,
    source_impl
};
}

class xsp_parser {
    public:
    xsp_parser();

    void open(const std::string & filename) {
        p.open(filename);
    }

    void parser_destructor(std::ostream & os) const;

    void to_stream(std::ostream & h) const;
    void to_stream(std::ostream & h, std::ostream & s) const;

    void header(std::ostream &, st::type) const;
    std::string parser_header() const;
    std::string parser_impl(st::type) const;
    void const_content(std::ostream & os) const;
    void parser_const(std::ostream & os, st::type t = st::header_impl) const;

    std::ostream& to_init(std::ostream& os, const elem_type & elem) const;
    std::ostream& to_decl(std::ostream& os, const elem_type & elem, const std::string & root) const;
    std::ostream& to_choice_init(std::ostream& os, const choice_type & choice) const;
    std::ostream& start_handler_contents(std::ostream& os, const elem_type & elem) const;
    void att_param(std::ostream& os, const std::string & start, const xml_att_type & att) const;

    // state
    std::string name_space;
    std::string uc_name_space;
    std::string base;
    xml_att_list attribute_types;
    elem_stack elems; // a vector of lists of elem_type
    std::string root; // root element, like xddl
    custom_type_list custom_types;
    std::string class_name;
    std::string head;
    group_list groups;
    choice_list choices;
    std::map<std::string, std::string> code_refs;
    std::pair<std::string, std::string> curr_code_atts;

    private:

    // temporary used while parsing
    std::string cdata;
    xml_att_list atts_;
    std::vector<std::string> names;
    std::vector<std::string> children;
    bool in_group_def = false;

    xenon::xml_parser p;
};

// algo
namespace xspx {
template <typename Xsp, typename Pred>
void for_each_element(Xsp & xspx, Pred op) {
    for (auto & i : xspx.elems) {
        for (auto & j : i) {
            if (!j.is_base) op(j);
        }
    }    

    for (auto & choice : xspx.choices) {
        for (auto & i : choice.elems) op(i);
    }
}


    std::vector<elem_type> unique_elems(const xsp_parser & xspx);

template <typename Os, typename Xsp>
void to_dispatch(Os & os, const Xsp & xsp, std::string const & name) {
    os << "template <typename Cursor, typename... Args>\n";
    os << "void " << name << "(Cursor c, Args&&... args) {\n";
    os << "    switch(c->uid) {\n";

    auto uniqs = unique_elems(xsp);

    for (auto & x : uniqs) {
        if (!x.is_mod) {
            os << "        ";
            os << "case ict::" << x.name << "_uid : " << name << 
                "(c, static_cast<ict::" << x.name << "*>(c->v.get()), std::forward<Args>(args)...); ";
            os << "break;\n";
        }
    }

    os << "\n} // end switch";
}

} // namespace xspx





