//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <iostream>
#include <vector>
#include <xenon/xenon.h>

using std::cout;
using std::cerr;

struct nv_pairs {
    nv_pairs(const std::string & name, int64_t value) : name(name), value(value) {}
    std::string name;
    int64_t value;
};

int main(int, char**) {
    try {
        auto fields = std::vector<nv_pairs>();

        cout << "loading\n";
        xenon::spec_server doc("icd.xddl");
        cout << "assigning\n";
        auto msg = xenon::parse(doc, "0101046B102C000114E03003603800203801C03801E03801F030037030002030"
              "00903000603000E0300120300130380000300110300030380080300200300210"
              "3001903001603000F03000703000403000C03000503000D1A03C9E16C18070DE"
              "2C7CFF3C7CC1001E00E01C000389");

        cout << "iterating over top level nodes only\n";
        for (auto c = msg.begin(); c != msg.end(); ++c) {
            // and now we can process each node
            cout << c->name() << '\n';
        }

        // using a linear cursor, we can iterate through the entire message in a depth first way
        cout << "putting fields and their values into a vector: " << xenon::to_text(msg);
        for (auto c = ict::to_linear(msg.root()); c != msg.end(); ++c) {
            // consumes() means it consumes bits from the message, usually a field 
            if (c->consumes()) fields.emplace_back(c->name(), c->value());
        }
        cout << "done, processed " << fields.size() << " fields\n\n";
        fields.clear();

        // We an also do the same thing using the ict::recurse() algorithm that takes a root node and
        // a lambda expression.  This is currently more efficient and recommended over the linear_cursor 
        // method above.
        cout << "again, using recurse algorithm\n";
        // The first cursor parameter in the lambda expression is a cursor to the current node, the second
        // cursor parameter is its parent, which is unused in this case.
        ict::recurse(msg.root(), [&](xenon::message::cursor c, xenon::message::cursor) {
            if (c->consumes()) fields.emplace_back(c->name(), c->value());
        });
        cout << "done, processed " << fields.size() << " fields\n\n";

        cout << "now find a field: DATA/MSG_TYPE\n";
        auto c = xenon::find_first(msg, "DATA/MSG_TYPE");

        if (c != msg.end()) cout << "found it! " << xenon::description(c) << "\n\n";

        cout << "now find another\n";
        c = xenon::find_first(msg.root(), "RLP_CAP_INFO_BLOCK/MAX_MS_NAK_ROUNDS_FWD");
        if (c != msg.end()) {
            cout << "found it! " << c->value() << "\n";
            cout << "full path is: " << xenon::path_string(c) << "\n\n";
        }
        

    } catch (std::exception & e) { 
        cerr << e.what() << '\n';
        return 1;
    }
}
