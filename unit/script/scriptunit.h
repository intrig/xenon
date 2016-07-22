#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.

#include <ict/unit.h>

class script_unit 
{
    public:
    void register_tests(ict::unit_test<script_unit> & ut) 
    {
        ut.skip();
        ut.cont();
        ut.add(&script_unit::sanity);
        ut.add(&script_unit::imsi);
    }

    void sanity();
    void imsi();
};
