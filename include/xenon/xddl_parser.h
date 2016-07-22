#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/xml_parser.h>
#include <ict/multivector.h>
#include <ict/xddl.h>
namespace ict {
    class xddl_parser {
        public:
        xddl_parser() {
            parents.push_back(ast.root());
            p.root_tag("xddl");
            p.add_children("field", {
            "comment", "item", "range", "script"}
            );
            p.add_children("record", {
            "field", "record", "comment"}
            );
            p.add_children("start", {
            "field", "record", "comment"}
            );
            p.add_children("xddl", {
            "field", "record", "comment", "start", "fragment"}
            );
            p.add_children("fragment", {
            "field", "record", "comment"}
            );
            p.start_handler("comment", [&](const att_list &) {
                auto e = std::make_shared<comment>();
                e->parser = this;
                e->line = p.line();
                auto c = parents.back().add(e);
                parents.push_back(c);
            }
            );
            p.end_handler("comment", [&]{
                parents.pop_back();
            }
            );
            p.start_handler("field", [&](const att_list &atts) {
                auto e = std::make_shared<field>(create_integer(find_att(atts, "bias", "0")), //;
                create_expression(find_att(atts, "default", "")), //;
                create_expression(find_required_att(atts, "length")), //;
                create_string(find_required_att(atts, "name")), //;
                create_type_proxy(find_att(atts, "type", "0")));
                e->parser = this;
                e->line = p.line();
                auto c = parents.back().add(e);
                parents.push_back(c);
            }
            );
            p.end_handler("field", [&]{
                parents.pop_back();
            }
            );
            p.start_handler("record", [&](const att_list &atts) {
                auto e = std::make_shared<record>(create_expression(find_att(atts, "length", "")), //;
                create_string(find_required_att(atts, "name")));
                e->parser = this;
                e->line = p.line();
                auto c = parents.back().add(e);
                parents.push_back(c);
            }
            );
            p.end_handler("record", [&]{
                parents.pop_back();
            }
            );
            p.start_handler("start", [&](const att_list &) {
                auto e = std::make_shared<start>();
                e->parser = this;
                e->line = p.line();
                auto c = parents.back().add(e);
                parents.push_back(c);
            }
            );
            p.end_handler("start", [&]{
                parents.pop_back();
            }
            );
            p.start_handler("xddl", [&](const att_list &) {
                auto e = std::make_shared<xddl>();
                e->parser = this;
                e->line = p.line();
                auto c = parents.back().add(e);
                parents.push_back(c);
            }
            );
            p.end_handler("xddl", [&]{
                parents.pop_back();
            }
            );
            p.start_handler("fragment", [&](const att_list & atts) {
                switch (fragment_test(atts)) {
                    case 0 : {
                        auto e = std::make_shared<fragment>(create_string(find_required_att(atts, "id")), //;
                        create_string(find_att(atts, "name", "")));
                        e->parser = this;
                        e->line = p.line();
                        auto c = parents.back().add(e);
                        parents.push_back(c);
                        break;
                    }
                    case 1 : {
                        auto e = std::make_shared<fragref>(create_fragment_proxy(find_required_att(atts, "href")));
                        e->parser = this;
                        e->line = p.line();
                        auto c = parents.back().add(e);
                        parents.push_back(c);
                        break;
                    }
                    default: IT_PANIC("unmatched choice");
                }
            }
            );
            p.end_handler("fragment", [&]{
                parents.pop_back();
            }
            );
        }
        xddl_parser(const ict::string_type & filename) : xddl_parser() {
            p.open(filename);
            file = filename;
        }
        void clear() {
            p.reset();
            ast.clear();
            parents.resize(1);
            file = "";
        }
        void open(const ict::string_type & filename) {
            clear();
            p.open(filename);
            file = filename;
        }
        xml_parser p;
        string_type file;
        typedef multivector<element::ptr> multivector_type;
        multivector_type ast;
        std::vector<multivector_type::cursor> parents;
    };
}
