#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license
#include <ict/unit.h>

class itu_unit {
    public:
    void register_tests(ict::unit_test<itu_unit> & ut) {
        ut.add(&itu_unit::host);
        ut.add(&itu_unit::bit_lengths);
    }

    /* Tests */
    void host();
    void bit_lengths();
};

