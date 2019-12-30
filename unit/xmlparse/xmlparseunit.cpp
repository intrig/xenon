#include "xmlparseunit.h"
#include <cstring>
#include <errno.h>
#include <fstream>
#include <map>
#include <string>
#include <typeinfo>
#include <vector>
#include <xenon/xml_parser_base.h>

class Scanner : public xenon::xml_parser_base {
  public:
    void startElement(const char *s, const char **atts) {
        const char **ai;
        tag = s;
        for (ai = atts; (*ai)[0] != '\0'; ai += 2)
            att_map[*ai] = *(ai + 1);
    }
    void endElement(const char *) { att_map.clear(); }
    void characterData(const char *s) { cdata_vec.push_back(s); }
    void comment(const char *){}

    std::string tag;
    std::vector<std::string> cdata_vec;
    std::map<std::string, std::string> att_map;
};

void xml_parse_unit::sanity() {
    Scanner p;

    try {
        p.parse("", 0, true);
    } catch (std::exception &e) {
        IT_FORCE_ASSERT(e.what());
    }

    try {
        p.reset();
        const char *str = "<a></a>";
        p.parse(str, strlen(str), true);
    } catch (std::exception &e) {
        IT_FORCE_ASSERT(e.what());
    }

    try {
        const char *str = "<a></a>";
        p.parse(str, strlen(str), true);
    } catch (std::exception &e) {
        if (!ict::contains(e.what(), "finished parsing"))
            IT_FORCE_ASSERT(e.what());
    }

    try {
        p.reset();
        const char *str = "<a></a";
        p.parse(str, strlen(str), true);
    } catch (std::exception &e) {
        if (!ict::contains(e.what(), "parsing incomplete"))
            IT_FORCE_ASSERT(e.what());
    }

    try {
        p.reset();
        const char *str = "<a/>";
        p.parse(str, strlen(str), true);
    } catch (std::exception &e) {
        IT_FORCE_ASSERT(e.what());
    }
}

struct test {
  public:
    char const *d;
    bool is_error;
};

const test parse_tests[] = {

    // empty elements
    {"<aa></aa>", false},
    {"<message></message>", false},
    {"<abcdddddddddddddddddddddddddddddddddddd>"
     "</abcdddddddddddddddddddddddddddddddddddd>",
     false},
    {"< aa></aa>", true},
    {"<aa ></aa>", false},
    {"<aa  ></aa>", true}, // can have 1 space but not two
    {"<postage ></postage >", false},
    {"<letter     ></letter >", true},
    {"<aa>< /aa>", true},
    {"<aa></ aa>", true},
    {"<aa></aa >", false},
    {"<aa></aa  >", true},
    {"<aa>></aa>", true},
    {"< aa><!--- hello --></aa>", true},
    {"<aa><!-- -- hello --></aa>", true},
    {"<aa><!-- hello ---></aa>", true},
    {"<aa></aa><bb></bb>", true},
    {"<a></a><b></b><c></c>", true},
    {"b></b><c></c>", true},
    {"></b><c></c>", true},
    {"</b><c></c>", true},
    {"/b><c></c>", true},
    {"b><c></c>", true},

    // comments
    {"<!-- hello --> <a ></a>", false},
    {"<!-- hello --> <a > <!-- goodbye --> </a>", false},
    {"<!-- hello --> <a > <!-- goodbye --> </a> <!-- done -->", false},

    // nested elements
    {"<a><b></b></a>", false},
    {"<a><b><c></c></b></a>", false},
    {"<?xml version = '1.0' encoding = 'iso-8859-1'?>\n"
     "<!-- comment -->\n"
     "<abc>\n"
     "  <abc>\n"
     "  </abc>\n"
     "</abc>\n",
     false},
    {"<a><b><c></b></c></a>", true},

    // attributes
    {"<elem></elem>", false},
    {"<elem att=\"a\"/>", false},
    {"<elem  att=\"value\"/>", false},
    {"<elem   att=\"anothervalue\"/>", false},
    {"<elem att=\"a\" />", false},
    {"<elem att=\"a\"  />", true},

    {"<elem att=\"a\"></elem>", false},
    {"<elem att=\"a\" att2=\"b\"></elem>", false},
    {"<elem att=\"a\" att2=\"b\" ></elem>", false},
    {"<elem att=\"a\" att2=\"b\"  ></elem>", true},
    {"<elem att=\"a\"  att2=\"b\"/>", false},

};

void xml_parse_unit::syntax() {
    try {
        unsigned i;
        bool error;
        for (i = 0; i < sizeof(parse_tests) / sizeof(test); ++i) {
            error = false;
            try {
                Scanner p;
                p.parse(parse_tests[i].d, strlen(parse_tests[i].d), true);
            } catch (std::exception &e) {
                error = true;
                if (!parse_tests[i].is_error)
                    IT_FORCE_ASSERT(parse_tests[i].d << "\n" << e.what());
            }
            if (!error && parse_tests[i].is_error) {
                IT_FORCE_ASSERT(parse_tests[i].d << "\n"
                                                 << "expected error");
            }
        }
    } catch (std::exception &e) {
        IT_FORCE_ASSERT(e.what());
    }
}

// same as above, but call reset between calls instead of constructing new
// object
void xml_parse_unit::reset() {
    Scanner p;
    unsigned i;
    bool error;
    for (i = 0; i < sizeof(parse_tests) / sizeof(test); ++i) {
        error = false;
        try {
            p.reset();
            p.parse(parse_tests[i].d, strlen(parse_tests[i].d), true);
        } catch (std::exception &e) {
            error = true;
            if (!parse_tests[i].is_error)
                IT_FORCE_ASSERT(parse_tests[i].d << "\n" << e.what());
        }
        if (!error && (parse_tests[i].is_error)) {
            IT_FORCE_ASSERT(parse_tests[i].d << "\n"
                                             << "expected error");
        }
    }
}

void xml_parse_unit::cdata() {
    {
        // TODO: this isn't testing cdata ???
        const char *xml = "<a><b>b</b><c>c</c><d>d</d></a>";
        try {
            Scanner p;
            p.parse(xml, strlen(xml), true);

            std::vector<std::string> e; // expected results;
            e.push_back("b");
            e.push_back("c");
            e.push_back("d");

            IT_ASSERT(p.cdata_vec == e);

        } catch (std::exception &e) {
            IT_FORCE_ASSERT(e.what());
        }
    }
    {
        const char *xml = "<a>&lt;b&gt;&amp;</a>";
        try {
            Scanner p;
            p.parse(xml, strlen(xml), true);

            std::vector<std::string> e; // expected results;
            e.push_back("<b>&");

            IT_ASSERT_MSG(p.cdata_vec[0] << " != " << e[0], p.cdata_vec == e);

        } catch (std::exception &e) {
            IT_FORCE_ASSERT(e.what());
        }
    }
}

void xml_parse_unit::atts() {
    {
        const char *xml1 = "<root><a a=\"a\" b=\"b\" c=\"c\">";
        const char *xml2 = " <b d=\"d\" e=\"e\" f=\"f\" g=\"g\">";
        const char *xml3 = "</b></a></root>";

        try {
            Scanner p;
            p.parse(xml1, strlen(xml1), false);

            std::map<std::string, std::string> e; // expected results;
            e["a"] = "a";
            e["b"] = "b";
            e["c"] = "c";

            IT_ASSERT(p.tag == "a");
            IT_ASSERT(p.att_map == e);

            p.att_map.clear();
            e.clear();
            p.parse(xml2, strlen(xml2), false);
            e["d"] = "d";
            e["e"] = "e";
            e["f"] = "f";
            e["g"] = "g";
            IT_ASSERT(p.tag == "b");
            IT_ASSERT(p.att_map == e);

            p.parse(xml3, strlen(xml3), true);

        } catch (std::exception &e) {
            IT_FORCE_ASSERT(e.what());
        }
    }
    {
        const char *xml1 = "<root><a a=\"a&amp;b\">";
        const char *xml2 = "</a></root>";
        try {
            Scanner p;
            p.parse(xml1, strlen(xml1), false);
            std::map<std::string, std::string> e; // expected results;
            e["a"] = "a&b";

            IT_ASSERT_MSG(p.att_map["a"] << " != " << e["a"], p.att_map == e);

            p.parse(xml2, strlen(xml2), true);
        } catch (std::exception &e) {
            IT_FORCE_ASSERT(e.what());
        }
    }
}

static const char *xml_files[] = {"simple.xml",
                           "../xddlunit/bias01.xddl",
                           "../xddlunit/bias02.xddl",
                           "../xddlunit/expression01.xddl",
                           "../xddlunit/expression02.xddl",
                           "../xddlunit/expression03.xddl",
                           "../xddlunit/expression04.xddl",
                           "../xddlunit/field01.xddl",
                           "../xddlunit/field02.xddl",
                           "../xddlunit/field03.xddl",
                           "../xddlunit/format01.xddl",
                           "../xddlunit/format02.xddl",
                           "../xddlunit/fragment01.xddl",
                           "../xddlunit/fragment02.xddl",
                           "../xddlunit/fragment03.xddl",
                           "../xddlunit/fragment05.xddl",
                           "../xddlunit/fragment06.xddl",
                           "../xddlunit/function01.xddl",
                           "../xddlunit/function02.xddl",
                           "../xddlunit/if01.xddl",
                           "../xddlunit/if02.xddl",
                           "../xddlunit/if03.xddl",
                           "../xddlunit/insert01.xddl",
                           "../xddlunit/insert03.xddl",
                           "../xddlunit/pad01.xddl",
                           "../xddlunit/pad02.xddl",
                           "../xddlunit/recurse.xddl",
                           "../xddlunit/repeat01.xddl",
                           "../xddlunit/repeat02.xddl",
                           "../xddlunit/repeat03.xddl",
                           "../xddlunit/repeat04.xddl",
                           "../xddlunit/repeat05.xddl",
                           "../xddlunit/repeat07.xddl",
                           "../xddlunit/subfields01.xddl",
                           "../xddlunit/subfields02.xddl",
                           "../xddlunit/subfields05.xddl",
                           "../xddlunit/subfields07.xddl",
                           "../xddlunit/switch01.xddl",
                           "../xddlunit/switch02.xddl",
                           "../xddlunit/switch03.xddl",
                           "../xddlunit/test01.xddl",
                           "../xddlunit/test02.xddl",
                           "../xddlunit/test03.xddl",
                           "../xddlunit/type01.xddl",
                           "../xddlunit/type02.xddl",
                           "../xddlunit/type03.xddl",
                           "../xddlunit/type04.xddl",
                           "../xddlunit/type05.xddl",
                           "../xddlunit/type06.xddl",
                           "../xddlunit/type07.xddl",
                           "../xddlunit/type08.xddl",
                           "../xddlunit/type09.xddl",
                           "../xddlunit/variable01.xddl",
                           "../xddlunit/variable02.xddl",
                           "../xddlunit/variable03.xddl",
                           "../xddlunit/variable05.xddl",
                           "../xddlunit/variable06.xddl",
                           "../xddlunit/variable08.xddl",
                           "../xddlunit/variable09.xddl",
                           "\0"};

void xml_parse_unit::files() {
    unsigned i = 0;
    try {
        for (i = 0; xml_files[i][0] != '\0'; ++i) {
            Scanner p;
            auto b = ict::read_file(std::string(xml_files[i]));
            p.parse(b.data(), b.size(), true);
        }
    } catch (std::exception &e) {
        IT_FORCE_ASSERT(e.what() << " in file: " << xml_files[i]);
    }
}

void xml_parse_unit::junk() {
    const char *xml = "<a></a><b></b><c></c>";
    const char *ch;

    Scanner p;
    ch = xml;
    do {
        try {
            p.parse(ch, strlen(ch), true);
        } catch (std::exception &e) {
            if (!ict::contains(e.what(), "junk after"))
                IT_FORCE_ASSERT(e.what());
            ch = ch + p.byte() - 1;
            ch--;
            p.reset();
        }
    } while (!p.finished());
}

void xml_parse_unit::big() {
    try {
        Scanner p;
        auto b = ict::read_file("cdma2000.xddl");
        p.parse(b.data(), b.size(), true);
    } catch (std::exception &e) {
        IT_FORCE_ASSERT(e.what());
    }
}

int main() {
    xml_parse_unit test;
    ict::unit_test<xml_parse_unit> ut(&test);
    return ut.run();
}
