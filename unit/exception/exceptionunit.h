#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license
#include <ict/unit.h>

struct exception_unit {
    void register_tests(ict::unit_test<exception_unit> & ut) {
        ut.add(&exception_unit::invalidExpression);
        ut.add(&exception_unit::fileNotFound);
    }

    void invalidExpression();
    void fileNotFound();
};
