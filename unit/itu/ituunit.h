#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/unit.h>

class itu_unit {
    public:
    void register_tests(ict::unit_test<itu_unit> & ut) {
        ut.add(&itu_unit::host);
        ut.add(&itu_unit::bit_lengths);
        ut.add(&itu_unit::splits);
        ut.add(&itu_unit::sanity);
        ut.add(&itu_unit::create_url);
        ut.add(&itu_unit::relative_url);
    }

    /* Tests */
    void host();
    void bit_lengths();
    void splits();
    void sanity();
    void create_url();
    void relative_url();
};

