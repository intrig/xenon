//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "ituunit.h"
#include <ict/ict.h>
#include <xenon/recref.h>

using std::string;
using std::vector;

void itu_unit::host()
{
    ict::Platform p = ict::hostPlatform();
#if defined(_MSC_VER) 
    IT_ASSERT(p == ict::MSVC);    
#endif
#if defined(__APPLE__) 
    IT_ASSERT(p == ict::Apple);    
    IT_ASSERT(p != ict::MSVC);    
    IT_ASSERT(p != ict::Linux);
    IT_ASSERT(!(p & ict::MSVC));
    IT_ASSERT(!(p & ict::Linux));
#endif
#if defined(__linux__)
    IT_ASSERT(p == ict::Linux);
#endif
    IT_ASSERT(p != ict::Unknown);
}

void itu_unit::bit_lengths() {
    struct range_type {
        int64_t lower;
        int64_t upper;
        int bit_size;
    };

    range_type tests[] = {
        {0, 1, 1},
        {0, 2, 2},
        {0, 3, 2},
        {0, 4, 3},
        {0, 5, 3},
        {0, 6, 3},
        {0, 7, 3},
        {0, 8, 4},
        {0, 549755813887, 39},
        {0, 9, 4},
        {0, 19, 5},
        {0, 31, 5},
        {0, 39, 6},
        {0, 63, 6},
        {0, 79, 7},
        {0, 127, 7},
        {0, 159, 8},
        {0, 255, 8},
        {0, 256, 9},
        {0, 316, 9},
        {0, 317, 9},
        {0, 318, 9},
        {0, 319, 9},
        {0, 320, 9},
        {0, 511, 9},
        {0, 639, 10},
        {0, 1023, 10},
        {0, 1279, 11},
        {0, 2047, 11},
        {0, 2559, 12}
    };

    int64_t v = ict::to_integer<int64_t>("549755813887");
    IT_ASSERT(v == 549755813887);
    IT_ASSERT(ict::is_integer("0"));

    int c = 0;
    range_type * first = tests;
    range_type * last = tests + sizeof(tests) / sizeof(tests[0]);

    while (first != last) {
        int bits = ict::required_bits(first->lower, first->upper);
        IT_ASSERT_MSG("test " << c << " (" << first->lower << ", " << first->upper << "): calculated " << 
            bits << ", expected " << first->bit_size,  
            bits == first->bit_size);
        ++first;
        ++c;
    }
}

void itu_unit::splits() {
    struct split_type {
        split_type(char c, const string & s, const vector<string> & expected) : c(c), s(s), expected(expected) {}
        split_type(const string & any, const string & s, const vector<string> & expected) : 
            any(any), s(s), expected(expected) {}
        char c;
        string any;
        string s;
        vector<string> expected;

        void confirm() {
            auto r = vector<string>();
            if (any.empty()) r = ict::split(s, c); // split on single char
            else r = ict::split(s, any.c_str());

            IT_ASSERT_MSG(s << ": " << ict::to_json(r) << " == " << ict::to_json(expected), r == expected);
        }
    };

    auto tests = vector<split_type>{ 
        { ' ', "split on spaces", {"split", "on", "spaces"}}, 
        { ' ', "split on spaces ", {"split", "on", "spaces"}}, 
        { '.', "split.on.dots", {"split", "on", "dots"}},
        { "./", "split.on.any/test", {"split", "on", "any", "test"}},
        { "./-", "split.on.any/test-", {"split", "on", "any", "test"}},
        { "./-_", "pcapng/pcapng_interface_description_block", 
            {"pcapng", "pcapng", "interface", "description", "block"}}

    };

    for (auto & i : tests) i.confirm();
}

void itu_unit::sanity()
{
    xenon::recref x;
    IT_ASSERT(x.empty());

    xenon::recref a{"a/3GPP2/TS-23.038.xddl#4d"};
    IT_ASSERT(!a.empty());

    xenon::recref b{"a/3GPP2/TS-23.038.xddl#4d"};
    IT_ASSERT(a == b);

    xenon::recref c = a;
    IT_ASSERT(c == b);
}

struct url_unit_type {
    std::string str() const {
        return path + file + anchor;
    }

    std::string path;
    std::string file;
    std::string anchor;
};

bool operator==(const xenon::recref & x, const url_unit_type & y) {
    return (x.path == y.path) && (x.file == y.file) && (x.anchor == y.anchor);
}

void itu_unit::create_url()
{
    std::vector< std::pair<xenon::recref, url_unit_type> > url_tests = 
        { 
            { { "just_a_file" }, {"", "just_a_file", ""} },
            { { "#4d" }, {"", "", "#4d"} },
            { { "TS-23.038.xddl#4d" }, {"", "TS-23.038.xddl", "#4d"} },
            { { "3GPP2/TS-23.038.xddl#4d" }, {"3GPP2/", "TS-23.038.xddl", "#4d"} },
            { { "a/3GPP2/TS-23.038.xddl#4d" }, {"a/3GPP2/", "TS-23.038.xddl", "#4d"} },
        };

    for (const auto & i : url_tests) { // vs2014 won't compile this without this brace
        IT_ASSERT_MSG(i.first.str() << " != " << i.second.str(), i.first == i.second);
    }
}

struct relative_url_type {
    xenon::recref base;
    xenon::recref offset;
    xenon::recref result;
};
    

void itu_unit::relative_url() {
    std::vector<relative_url_type> url_tests = {
        { "", "" , "" },
        { "", "#3.1.1" , "#3.1.1" },
        { "", "C.R1001-G.xddl" , "C.R1001-G.xddl" },
        { "", "C.R1001-G.xddl#3.1.1" , "C.R1001-G.xddl#3.1.1" },

        { "C.S0005.xddl", "" , "C.S0005.xddl" },
        { "C.S0005.xddl", "#3.1.1" , "C.S0005.xddl#3.1.1" },
        { "C.S0005.xddl", "C.R1001-G.xddl" , "C.R1001-G.xddl" },
        { "C.S0005.xddl", "C.R1001-G.xddl#3.1.1" , "C.R1001-G.xddl#3.1.1" },

        { "3GPP2/C.S0005.xddl", "" , "3GPP2/C.S0005.xddl" },
        { "3GPP2/C.S0005.xddl", "#3.1.1" , "3GPP2/C.S0005.xddl#3.1.1" },
        { "3GPP2/C.S0005.xddl", "C.R1001-G.xddl" , "3GPP2/C.R1001-G.xddl" },
        { "3GPP2/C.S0005.xddl", "C.R1001-G.xddl#3.1.1" , "3GPP2/C.R1001-G.xddl#3.1.1" },

        { "3GPP2/C.S0005.xddl", "foo/b.xddl" , "3GPP2/foo/b.xddl" },

        { "icd.xddl", "#zero", "icd.xddl#zero" },
        { "3GPP/TS-24.008.xddl", "TS-24.007.xddl#11.2", "3GPP/TS-24.007.xddl#11.2" },
        { "3GPP/TS-24.008.xddl", "TS-24.007.xddl#11.2.2.1c", "3GPP/TS-24.007.xddl#11.2.2.1c" },
    };

    for (const auto & i : url_tests) {
        auto result = xenon::relative_url(i.base, i.offset);
        IT_ASSERT_MSG(result.str() << " == " << i.result.str(), result == i.result);
    }
}

int main (int, char **) {
    itu_unit test;
    ict::unit_test<itu_unit> ut(&test);
    return ut.run();
}
