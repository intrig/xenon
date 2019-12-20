#include "specunit.h"
#include <cstdint>
#include <string>
#include <vector>
#include <xenon/xenon.h>

void doc_unit::fieldtypes() {

    std::string xddl = R"(
      <xddl>
        <start>
            <uint8 name="byte"/>
        </start>
      </xddl>
    )";

    xenon::spec_server spec(xddl.begin(), xddl.end());

    auto m = parse(spec, ict::bitstring("#08"));

    auto n = xenon::find_first(m.root(), "byte");
    IT_ASSERT(n != m.end());
    IT_ASSERT(n->value() == 0x08);
}

void doc_unit::allTypes() {

    std::string xddl = R"(
      <xddl>
        <start>
            <bit name="bit"/>
            <uint8 name="uint8"/>
            <uint16 name="uint16"/>
            <uint32 name="uint32"/>
            <uint64 name="uint64"/>
        </start>
      </xddl>
    )";

    xenon::spec_server doc(xddl.begin(), xddl.end());

    ict::obitstream ob;
    ob << ict::bitstring("@1") << ict::from_integer<uint8_t>(8)
       << ict::from_integer<uint16_t>(16) << ict::from_integer<uint32_t>(32)
       << ict::from_integer<uint64_t>(64);

    auto bs = ob.bits();

    IT_ASSERT_MSG("bs.length == " << bs.bit_size(),
                  bs.bit_size() == (1 + 8 + 16 + 32 + 64));

    auto m = xenon::parse(doc, bs);

    auto n = xenon::find_first(m.root(), "bit");
    IT_ASSERT(n != m.root().end());
    IT_ASSERT(n->length() == 1);

    IT_ASSERT(xenon::find_first(m.root(), "bit")->value() == 1);
    IT_ASSERT(xenon::find_first(m.root(), "uint8")->value() == 8);
    IT_ASSERT(xenon::find_first(m.root(), "uint16")->value() == 16);
    IT_ASSERT(xenon::find_first(m.root(), "uint32")->value() == 32);
    IT_ASSERT(xenon::find_first(m.root(), "uint64")->value() == 64);
}
