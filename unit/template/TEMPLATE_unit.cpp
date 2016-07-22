//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "TEMPLATE_unit.h"

void TEMPLATE_unit::sanity() {
    IT_ASSERT(1);
}


int main (int, char **)
{
    TEMPLATE_unit test;
    IT::UnitTest<TEMPLATE_unit> ut(&test);
    return ut.run();
}
