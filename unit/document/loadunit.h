#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license

#include <ict/unit.h>
#include <ict/xenon.h>

class load_unit {
    public:

    void register_tests(ict::unit_test<load_unit> & ut) {
        ut.skip();
        ut.cont();
        ut.add(&load_unit::sanity);
        ut.add(&load_unit::load_all_specs);
    }

    /* Tests */
    void sanity();
    void load_all_specs();

    ict::spec_server specs;
};
