//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "specunit.h"

#include <xenon/xenon.h>

#include <vector>
#include <string>
#include <cstdint>

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

    auto n = xenon::find(m.root(), "byte");
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
    ob << ict::bitstring("@1") << ict::from_integer<uint8_t>(8) << ict::from_integer<uint16_t>(16) <<
        ict::from_integer<uint32_t>(32) << ict::from_integer<uint64_t>(64);

    auto bs = ict::bitstring(ob.bits().begin(), ob.bits().bit_size());

    IT_ASSERT_MSG("bs.length == " << bs.bit_size(), bs.bit_size() == (1 + 8 + 16 + 32 + 64));

    auto m = xenon::parse(doc, bs);

    auto n = xenon::find(m.root(), "bit");
    IT_ASSERT(n != m.root().end());
    IT_ASSERT(n->length() == 1);

    IT_ASSERT(xenon::find(m.root(), "bit")->value() == 1);
    IT_ASSERT(xenon::find(m.root(), "uint8")->value() == 8);
    IT_ASSERT(xenon::find(m.root(), "uint16")->value() == 16);
    IT_ASSERT(xenon::find(m.root(), "uint32")->value() == 32);
    IT_ASSERT(xenon::find(m.root(), "uint64")->value() == 64);
}

// endian not supported for now in mt
#if 0
void doc_unit::endian() {
    spec doc;

    doc.parseString(R"(
      <xddl>
        <uint32 name="magic"/>
        <endian big="magic == #1A2B3C4D"/>
        <uint16 name="one"/>
      </xddl>
    )");

    message m(doc);

    m = bitstring_type("1A2B3C4D0001");

    auto magic = m["magic"];

    IT_ASSERT_MSG(magic.bitString().toHexString(), magic == 0x1A2B3C4D);
    IT_ASSERT(m["one"] == 1);

    // now switch the endianness
    m = bitstring_type("4D3C2B1A0100");

    // bitstring should remain the same
    IT_ASSERT(magic.bitString() == bitstring_type("4D3C2B1A"));

    // but integer value changes
    IT_ASSERT(magic == 0x1A2B3C4D);

    IT_ASSERT_MSG(m["one"], m["one"] == 0x0001); 
    IT_ASSERT(m["one"] == 0x0001); 
}
#endif
