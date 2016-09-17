#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.

#include <ict/unit.h>

class doc_unit
{
    public:
    doc_unit();

    void register_tests(ict::unit_test<doc_unit> & ut) {
        ut.skip();
        ut.cont();
        ut.add(&doc_unit::sanity);
        ut.add(&doc_unit::constructor_file);
        ut.add(&doc_unit::parse_file);

        ut.skip(&doc_unit::absolute);
        ut.add(&doc_unit::index2);
        ut.add(&doc_unit::index3);

        ut.add(&doc_unit::ip_protocol);
        ut.skip(&doc_unit::search_paths);

        ut.add(&doc_unit::fail1);
        ut.add(&doc_unit::fail2);
        ut.add(&doc_unit::fail3);

        ut.add(&doc_unit::fieldtypes);
        ut.add(&doc_unit::allTypes);
        ut.add(&doc_unit::get_record);
        ut.skip();
        ut.cont();

    }

    /* Tests */
    void sanity();
    void constructor_file();
    void parse_file();
    void absolute();
    void index2();
    void index3();
    void ip_protocol();
    void search_paths();
    void fail1();
    void fail2();
    void fail3();

    void fieldtypes();
    void allTypes();
    void recref_regex();
    void get_record();
    void relative_url();
};
