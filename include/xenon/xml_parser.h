#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <vector>
#include <functional>

#include <xenon/xml_parser_base.h>

#include <ict/string64.h>
#include <xenon/att_pair.h>

// TODO: rewrite XmlParserBase to be 
// template <typename T>
// class xml_parser { ... };
// 
// with T as the xml_parser class (renamed) below, and no virtual functions.

namespace xenon {

template <typename Op>
struct named_handler {
    named_handler(const ict::string64 & name, Op op) : name(name), op(op) {}
    ict::string64 name;
    Op op;
};

template <typename I, typename T>
I find_by_name(I first, I last, T name) {
    while (first != last) {
        if (first->name == name.c_str()) return first;
        ++first;
    }
    return last;
}

template <typename Cont, typename T>
const typename Cont::value_type & find_by_name(const Cont & c, T value) {
    auto i = find_by_name(c.begin(), c.end(), value);
    if (i == c.end()) IT_PANIC(value << " is not a valid name");
    return *i;
}

class xml_parser : public xml_parser_base {
    typedef std::function<void(const char *)> text_handler_type;

    typedef std::function<void(const att_list &)> start_handler_type;
    typedef std::function<void()> end_handler_type;

    typedef std::function<void(const ict::string64 & tag, const att_list &)> post_start_handler_type;
    typedef std::function<void(const ict::string64 & tag)> post_end_handler_type;

    typedef named_handler<start_handler_type> named_start_handler;
    typedef std::vector<named_start_handler> start_handler_list;

    typedef named_handler<end_handler_type> named_end_handler;
    typedef std::vector<named_end_handler> end_handler_list;

    typedef std::pair<ict::string64, std::vector<ict::string64>> named_string64_list;

public:

    xml_parser() {}

    xml_parser(const std::string & filename) {
        open(filename);
    }

    void cdata_handler(text_handler_type handler) {
        cdata_function = handler;
    }

    void start_handler(const ict::string64 & tag, start_handler_type handler) {
        start_handlers.emplace_back(tag, handler);
    }

    void post_start_handler(post_start_handler_type handler) {
        post_start = handler;
    }

    void end_handler(const ict::string64 & tag,  end_handler_type handler) {
        end_handlers.emplace_back(tag, handler);
    }

    void post_end_handler(post_end_handler_type handler) {
        post_end = handler;
    }

    void root_tag(const ict::string64 & value) { root = value; }

    void add_children(const ict::string64 & parent, const ict::string64_list & children) {
        auto i = std::find_if(child_table.begin(), child_table.end(), 
            [&](const named_string64_list & p) { return p.first == parent; });
        if (i != child_table.end()) IT_PANIC("children of " << parent << " already defined");
        child_table.emplace_back(std::make_pair(parent, children));
    }

    template <typename T>
    void open(T first, T last, const std::string & filename) {
        try {
            xparse(&(*first), last - first, true);
        } catch (xenon::xml_error & e) {
            IT_FATAL(e.description << " in [" << filename << ':' << e.xml_line << ':' << e.xml_column << ']'); 
        }
    }

    void open(const std::string & filename) {
        auto contents = ict::read_file(filename);
        open(contents.begin(), contents.end(), filename);
    }


private:
    //handlers
    text_handler_type cdata_function = [](const char *){};
    text_handler_type comment_handler = [](const char *){};
    start_handler_list start_handlers;
    post_start_handler_type post_start = [](const ict::string64, const att_list &){};
    end_handler_list end_handlers;
    post_end_handler_type post_end = [](const ict::string64 &){};

    // parser state
    std::vector<ict::string64> parents;
    ict::string64 root;
    std::vector<named_string64_list> child_table;

    // temporary 
    att_list atts;

    // TODO: handlers, make these non-virtual
    virtual void characterData(const char * s) {
        cdata_function(s);
    }

    virtual void comment(const char * s) {
        comment_handler(s);
    }

    virtual void startElement(const char *n, const char **att_array) {
        ict::string64 name(n);
        if (parents.empty()) {
            if (root != name) IT_PANIC(name << " is an invalid root node, expecting " << root);
            parents.push_back(root);
        } else {
            // verify this is a valid child of parent
            auto i = std::find_if(child_table.begin(), child_table.end(), 
                [&](const std::pair<ict::string64, std::vector<ict::string64>> & p){ return p.first == parents.back();});
            if (i == child_table.end()) IT_PANIC(name << " is not a valid element name");
            else {
                auto j = find(i->second.begin(), i->second.end(), name);
                if (j == i->second.end())  IT_PANIC(name << " is not a valid child of " << parents.back());
            }

            // push this element's name on parent stack
            parents.push_back(name);
        }

        create_att_list(atts, att_array);
        auto i = find_by_name(start_handlers.begin(), start_handlers.end(), name);
        if (i != start_handlers.end()) i->op(atts);
        post_start(name, atts);
    }

    virtual void endElement(const char *n) {
        ict::string64 name(n);
        parents.pop_back();
        auto i = find_by_name(end_handlers.begin(), end_handlers.end(), name);
        if (i != end_handlers.end()) i->op();
        post_end(name);
    }
};
} // namespace ict
