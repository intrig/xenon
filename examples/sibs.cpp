//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <iostream>
#include <ict/command.h>
#include <xenon/xenon.h>

using std::cout;
using std::cerr;

int main(int argc, char ** argv) {
    try {
        xenon::spec_server ss;

        ict::command line("sibs", 
            "Example to parse a sib message containing another.", 
            "sibs [options]");
        line.add(ict::option("xddl_path", 'X', "XDDL path", "", [&](const std::string & v){ 
            ss.xddl_path.push_back(v);
        }));

        line.parse(argc, argv);

        auto start = xenon::get_record(ss, "icd.xddl");
        auto msg = xenon::parse(start, "220008342E1F7F61BA04C697D176821A7A9F4EA20663F3");

        // display sib message
        cout << xenon::to_text(msg) << '\n';

        // grab the inner sib. 
        auto c = xenon::find_first(msg.root(), "SIB-Data-variable");
        if (c==msg.end()) IT_PANIC("can't find SIB-Data-variable");

        // display the bits of the inner sib
        cout << c->name() << " " << c->bits << "\n";

        // now parse the inner sib as a SysInfoType19
        // We first get the record we are interested from the spec, and then use it to parse.
        start = xenon::get_record(ss, "3GPP/TS-25.331/SysInfoType19");
        auto inner = xenon::parse(start, c->bits);

        // display inner sib
        cout << xenon::to_text(inner, "nlvs");
    }

    catch (std::exception & e) {
        cerr << e.what() << "\n";
    }
}
