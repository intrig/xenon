#pragma once
#include <xenon/ict/unit.h>

class unit {
  public:
    void register_tests(ict::unit_test<unit> &ut) {
        ut.add(&unit::find_first_test);
        ut.add(&unit::find_first_again);
        ut.add(&unit::for_each_path_test);
        ut.skip();
        ut.cont();
    }
    void find_first_test();
    void find_first_again();
    void for_each_path_test();
};
