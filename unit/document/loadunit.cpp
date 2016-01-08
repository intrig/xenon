//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "loadunit.h"

#include <ict/xenon.h>
#include <boost/filesystem.hpp>

void load_unit::sanity() {
    ict::spec_server doc;
    IT_ASSERT(doc.empty());
}


int main (int, char **) {
    load_unit test;
    ict::unit_test<load_unit> ut(&test);
    return ut.run();
}
