#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license
#include "../itunit.h"

namespace IT {
class CodeUnit 
{
    public:
    void register_tests(UnitTest<CodeUnit> & ut) 
    {
        ut.skip();
        ut.cont();
        ut.add(&CodeUnit::sanity);
    }

    void sanity();
};
}
