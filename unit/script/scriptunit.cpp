//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "scriptunit.h"

#include <ict/xenon.h>

void script_unit::sanity() {
    IT_ASSERT(1);
}

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

    ict::spec spec(xddl.begin(), xddl.end());

    ict::message m;

    std::vector<std::pair<std::string, ict::bitstring> > tests;
    tests.push_back(std::make_pair("(012) 345-6789", "@1110000101001110101001101010100110"));
    tests.push_back(std::make_pair("(760) 536-3081", "@1010010011011010100100111111001010"));
    tests.push_back(std::make_pair("(760) 419-1278", "@1010010011010011010000010010100111"));

    for (auto & t : tests) {
        m = ict::parse(spec, t.second);
        auto n = ict::find(m.root(), ict::path("/IMSI_S"));
        IT_ASSERT(n != m.end());
        IT_ASSERT_MSG(ict::description(n) << " != " << t.first, ict::description(n) == t.first);
    }
}

int main (int, char **) {
    script_unit test;
    ict::unit_test<script_unit> ut(&test);
    return ut.run();
}
