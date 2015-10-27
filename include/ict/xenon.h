#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license
#include <ict/message.h>
#include <ict/spec_server.h>

namespace ict {
// TODO make this work with const reference
inline bitstring serialize(message & m) {
    obitstream bs;
    recurse(m.root(), [&](const message::cursor & self, const message::cursor &) {
        if (self->consumes()) bs << self->bits;
    });
    return bs.bits();
}

message parse(spec::cursor start, ibitstream & bs);

message parse(spec::cursor start, const bitstring & bits);

message parse(spec_server &, const bitstring & bits);

spec::cursor get_record(spec_server &, const url & href);

spec::cursor get_type(spec_server &, const url & href);

}

