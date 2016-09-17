#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.

#include <ict/unit.h>

class unit {
    public:

    void register_tests(ict::unit_test<unit> & ut) {
        ut.add(&unit::recref_regex);
        ut.add(&unit::relative_url);
        ut.skip();
        ut.cont();

    }
    void recref_regex();
    void relative_url();
};
