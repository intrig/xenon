#include "scriptunit.h"
#include <xenon/xenon.h>

void script_unit::imsi() {
    std::string xddl = R"(
<xddl>
  <type name="IMSI_S_type" id="IMSI_S-type">
    <script>
      local v = imsi_s();
      description = string.format("(%s) %s-%s", v:sub(1, 3), v:sub(4, 6), v:sub(7, 10))
    </script>
  </type>
  <start>
    <field name="IMSI_S" length="34" type="#IMSI_S-type"/>
  </start>
</xddl>)";

    xenon::spec_server spec(xddl.begin(), xddl.end());

    xenon::message m;

    std::vector<std::pair<std::string, ict::bitstring>> tests;
    tests.push_back(std::make_pair("(012) 345-6789",
                                   "@1110000101001110101001101010100110"));
    tests.push_back(std::make_pair("(760) 536-3081",
                                   "@1010010011011010100100111111001010"));
    tests.push_back(std::make_pair("(760) 419-1278",
                                   "@1010010011010011010000010010100111"));

    for (auto &t : tests) {
        m = xenon::parse(spec, t.second);
        auto n = xenon::find_first(m.root(), "/IMSI_S");
        IT_ASSERT(n != m.end());
        IT_ASSERT_MSG(xenon::description(n) << " != " << t.first,
                      xenon::description(n) == t.first);
    }
}

int main(int, char **) {
    script_unit test;
    ict::unit_test<script_unit> ut(&test);
    return ut.run();
}
