#pragma once
#include <xenon/ict/unit.h>

class script_unit {
  public:
    void register_tests(ict::unit_test<script_unit> &ut) {
        ut.add(&script_unit::imsi);
    }

    void imsi();
};
