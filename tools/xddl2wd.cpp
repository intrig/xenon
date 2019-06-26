//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/command.h>
#include <xenon/xenon.h>

using std::cout;
using std::cerr;
using std::endl;


void convert_to_wd(std::ostream & os, xenon::recref const & rr);

int main(int argc, char **argv) {
    using ict::command;
    using ict::option;
    try {
        std::string xddl_file;
        
        command line("xddl2wd", "Convert xddl to writedown.", 
            "xddl [options] recref\n");
#if 0
        line.add(option("encoding", 'e', "Display encoding fields", [&]{ flags.encoding = true; } ));
        line.add(option("properties", 'p', "Display properties", [&]{ flags.properties = true; } ));
        line.add(option("xml", 'x', "Display message(s) in XML", [&]{ flags.pretty_xml = true; } ));
        line.add(option("flat-xml", 'f', "Display message(s) in flat XML", [&]{ flags.flat_xml = true; } ));
        line.add(option("location", 'L', "show xddl source location", [&]{ flags.format += "FL"; } ));
        line.add(option("dom", 'd', "Display xddl dom", [&]{ flags.output_dom = true; } ));
        line.add(option("html", 'H', "Display in html", [&]{ flags.output_html = true; } ));
        line.add(option("debug", 'D', "Display message(s) in debug gibberish", [&]{ flags.debug_print = true; } ));
        line.add(option("extra", 'E', "Display extra bits", [&]{ flags.show_extra = true; } ));
#endif

        line.parse(argc, argv);

        if (line.targets.empty()) IT_FATAL("no file given");

        convert_to_wd(cout, line.targets[0]);
    } catch (std::exception & e) {
        cout << e.what() << endl;
        return 1;
    }
}

void convert_to_wd(std::ostream & os, xenon::recref const & rr) {
    xenon::spec_server d;
    auto r = d.add_spec(rr.path + rr.file);
    os << "done!\n";
}

