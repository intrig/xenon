#include <iostream>
#include <ict/xenon.h>
#include <ict/command.h>

using std::cout;
using std::cerr;

int main(int argc, char ** argv) {
    try {
        ict::spec_server ss;

        ict::command line("sibs", 
            "Example to parse a sib message containing another.", 
            "sibs [options]");
        line.add(ict::option("xddl_path", 'X', "XDDL path", "", [&](const std::string & v){ 
            ss.xddl_path.push_back(v);
        }));

        line.parse(argc, argv);

        auto start = ict::get_record(ss, "icd.xddl");
        auto msg = ict::parse(start, "220008342E1F7F61BA04C697D176821A7A9F4EA20663F3");

        // display sib message
        cout << ict::to_text(msg) << '\n';

        // grab the inner sib. The '//' means it doesn't have to be a direct child of msg.root()
        auto c = ict::find(msg.root(), "//SIB-Data-variable");
        if (c==msg.end()) IT_THROW("can't find SIB-Data-variable");

        // display the bits of the inner sib
        cout << c->name() << " " << c->bits << "\n";

        // now parse the inner sib as a SysInfoType19
        // We first get the record we are interested from the spec, and then use it to parse.
        start = ict::get_record(ss, "3GPP/TS-25.331.xddl#SysInfoType19");
        auto inner = ict::parse(start, c->bits);

        // display inner sib
        cout << ict::to_text(inner, "nlvsFL");
    }

    catch (std::exception & e) {
        cerr << e.what() << "\n";
    }
}
