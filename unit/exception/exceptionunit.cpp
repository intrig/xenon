//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "exceptionunit.h"

#include <ict/xenon.h>

void exception_unit::invalidExpression() {
    std::string error;
    try {
        ict::spec_server doc("invalid01.xddl");
    } catch (ict::exception & e) {
        error = e.what();
    }
    IT_ASSERT(!error.empty());
}

void exception_unit::fileNotFound() {
    std::string error;
    try {
        ict::spec_server doc("notthere.xddl");
    } catch (ict::exception & e) {
        error = e.what();
    }
    IT_ASSERT(!error.empty());
}

int main (int, char **) {
    exception_unit test;
    ict::unit_test<exception_unit> ut(&test);
    return ut.run();
}
