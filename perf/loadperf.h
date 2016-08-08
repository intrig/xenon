#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.

#include <ict/unit.h>
#include <xenon/xenon.h>

class load_unit {
    public:

    void register_tests(ict::unit_test<load_unit> & ut) {
        ut.add(&load_unit::load_all_specs);
    }

    /* Tests */
    void load_all_specs();

    int iterations = 5;
    std::string root;
};

