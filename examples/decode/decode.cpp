//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <iostream>
#include <vector>
#include <ict/xenon.h>

using std::cout;
using std::cerr;
using ict::message;

struct nv_pairs {
    nv_pairs(const std::string & name, int64_t value) : name(name), value(value) {}
    std::string name;
    int64_t value;
};

int main(int, char**) {
    try {
        auto fields = std::vector<nv_pairs>();

        cout << "loading\n";
        ict::spec_server doc("icd.xddl");
        cout << "assigning\n";
        auto msg = ict::parse(doc, "0101046B102C000114E03003603800203801C03801E03801F030037030002030"
              "00903000603000E0300120300130380000300110300030380080300200300210"
              "3001903001603000F03000703000403000C03000503000D1A03C9E16C18070DE"
              "2C7CFF3C7CC1001E00E01C000389");

        cout << "putting fields and their values into a vector: " << ict::to_text(msg);
        for (auto c = ict::linear_begin(msg.root()); c != msg.end(); ++c) {
            if (c->consumes()) fields.emplace_back(c->name(), c->value());
        }
        cout << "done, processed " << fields.size() << " fields\n\n";

        fields.clear();
        cout << "again, using recurse algorithm\n";
        ict::recurse(msg.root(), [&](message::cursor c, message::cursor) {
            if (c->consumes()) fields.emplace_back(c->name(), c->value());
        });
        cout << "done, processed " << fields.size() << " fields\n\n";

        cout << "now find a field: DATA/MSG_TYPE\n";
        auto c = ict::find(msg.root(), "DATA/MSG_TYPE");

        if (c != msg.end()) cout << "found it! " << ict::description(c) << "\n\n";

        cout << "now find using a non-anchored path: //RLP_CAP_INFO_BLOCK/MAX_MS_NAK_ROUNDS_FWD\n";
        c = ict::find(msg.root(), "//RLP_CAP_INFO_BLOCK/MAX_MS_NAK_ROUNDS_FWD");
        if (c != msg.end()) cout << "found it! " << c->value() << "\n\n";
        

    } catch (ict::exception & e) { cerr << e.what() << '\n';
        return 1;
    }
}
