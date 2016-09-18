#pragma once
#include "xspx_parser.h"

namespace xspx {

void to_writedown(std::ostream & os, const xsp_parser & xspx) {
    for (auto & i : xspx.elems) {
        for (auto & j : i) {
            if (!j.is_base) os << j.tag << ": " << j.name << '\n';
        }
        os << "done\n";
    }    

    os << "choices:\n";
    for (auto & choice : xspx.choices) {
        for (auto & i : choice.elems) os << i.tag << ": " << i.name << ", ";
        os << '\n';
    }

}

}
