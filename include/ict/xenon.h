#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license
#include <random>
#include <functional>
#include <ict/message.h>
#include <ict/spec_server.h>

namespace ict {
inline bitstring serialize(const message & m) {
    obitstream bs;
    recurse(m.root(), [&](message::const_cursor & self, message::const_cursor &) {
        if (self->consumes()) bs << self->bits;
    });
    return bs.bits();
}

inline ict::bitstring random_bitstring(size_t bit_len) {
    std::random_device engine;
    auto bytes = bit_len / 8 + 1;
    auto v = std::vector<unsigned char>(bytes);
    for (auto & b : v) b = engine();
    return ict::bitstring(v.begin(), bit_len);
}

template <typename Message>
inline void recombobulate(Message & a) {
    ict::recurse(a.root(), [&](ict::message::cursor c, ict::message::cursor) {
        if (c->is_pof()) c->bits = random_bitstring(c->bits.bit_size());
    });
}

std::string to_text(const message & m, const std::string & format = "nlvhs",
    std::function<bool(message::const_cursor c)> filter=[&](message::const_cursor){ return true; });
   
message parse(spec::cursor start, ibitstream & bs);

message parse(spec::cursor start, const bitstring & bits);

message parse(spec_server &, const bitstring & bits);

spec::cursor get_record(spec_server &, const url & href);

spec::cursor get_type(spec_server &, const url & href);

inline std::string to_hex_string(const bitstring & bits) {
    return to_hex_string(bits.begin(), bits.end());
}

}

