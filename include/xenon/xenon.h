#pragma once
#include "ict/bitstring.h"
#include "message.h"
#include "recref.h"
#include "spec_server.h"
#include <functional>
#include <random>

namespace xenon {
inline ict::bitstring serialize(const message &m) {
    ict::obitstream bs;
    recurse(m.root(), [&](message::const_cursor &self) {
        if (self->consumes())
            bs << self->bits;
    });
    return bs.bits();
}

template <typename Message> inline void recombobulate(Message &a) {
    ict::recurse(a.root(), [&](message::cursor c) {
        if (c->is_pof())
            c->bits = ict::random_bitstring(c->bits.bit_size());
    });
}

// TODO: change this to header only
std::string to_text(const message &m, const std::string &format = "nlvhs",
                    std::function<bool(message::const_cursor c)> filter =
                        [](message::const_cursor) { return true; });

// TODO: change this to header only
std::string to_xml(const message &m,
                   std::function<bool(message::const_cursor c)> filter =
                       [](message::const_cursor) { return true; });

namespace util {
template <typename Stream, typename Cursor, typename Filter>
void to_debug_text(Stream &os, Cursor parent, Filter filter, int level) {
    for (auto c = parent.begin(); c != parent.end(); ++c)
        if (filter(c)) {
            os << ict::spaces(level * 2);
            to_debug(os, *c);
            os << '\n';
            to_debug_text(os, c, filter, level + 1);
        }
}
} // namespace util

template <typename Filter>
inline std::string to_debug_text(const message &m, Filter filter) {
    std::ostringstream ss;
    util::to_debug_text(ss, m.root(), filter, 0);
    return ss.str();
}

inline message parse(spec::cursor start, ict::ibitstream &bs) {
    message m;
    m.root().emplace_back(node::prop_node, start);
    parse(start, m.root(), bs);
    return m;
}

inline message parse(spec::cursor start, const ict::bitstring &bits) {
    ict::ibitstream bs(bits);
    return parse(start, bs);
}

inline message parse(spec_server &spec, const ict::bitstring &bits) {
    return parse(spec.start(), bits);
}

spec::cursor get_record(spec_server &, const recref &href);

spec::cursor get_type(spec_server &, const recref &href);

inline message parse(spec_server &spec, const recref &rec_id,
                     const ict::bitstring &bits) {
    auto rec = get_record(spec, rec_id);
    return parse(rec, bits);
}

inline std::string to_hex_string(const ict::bitstring &bits) {
    return ict::to_hex_string(bits.begin(), bits.end());
}

std::string to_html(const spec &s);

} // namespace xenon
