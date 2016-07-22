#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.

#include <ict/unit.h>

class xml_parse_unit 
{
    public:
    void register_tests(ict::unit_test<xml_parse_unit> & ut) 
    {
        ut.skip();
        ut.cont();
        ut.add(&xml_parse_unit::sanity);
        ut.add(&xml_parse_unit::syntax);
        ut.add(&xml_parse_unit::cdata);
        ut.add(&xml_parse_unit::atts);
        ut.add(&xml_parse_unit::reset);
        ut.add(&xml_parse_unit::files);
        ut.add(&xml_parse_unit::junk);
        ut.add(&xml_parse_unit::big);
        ut.skip();
        ut.cont();
    }

    /* Tests */
    void sanity();
    void syntax();
    void cdata();
    void atts();
    void reset();
    void files();
    void junk();
    void big();
};
