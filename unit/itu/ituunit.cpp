//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "ituunit.h"

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
        {0, 0, 0},
        {0, 1, 1},
        {0, 2, 2},
        {0, 3, 2},
        {0, 4, 3},
        {0, 5, 3},
        {0, 6, 3},
        {0, 7, 3},
        {0, 8, 4},
        {0, 549755813887, 39}
    };

    int64_t v = ict::to_integer<int64_t>("549755813887");
    IT_ASSERT(v == 549755813887);
    IT_ASSERT(ict::is_integer("0"));

    int c = 0;
    range_type * first = tests;
    range_type * last = tests + sizeof(tests) / sizeof(tests[0]);

    while (first != last) {
        int bits = ict::required_bits(first->lower, first->upper);
        IT_ASSERT_MSG("test " << c << ": calculated " << bits << ", expected " << first->bit_size,  
            bits == first->bit_size);
        ++first;
        ++c;
    }
}



int main (int, char **) {
    itu_unit test;
    ict::unit_test<itu_unit> ut(&test);
    return ut.run();
}
