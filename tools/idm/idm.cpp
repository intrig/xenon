//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/xenon.h>
#include <ict/command.h>

using std::cout;
using std::cerr;
using std::endl;

struct command_flags {
    bool output_xml = false;
    bool pretty_xml = true;
    bool output_dom = false;
    std::string format = "nlvhs";
};

void processXddlFile(ict::command const & line, command_flags const & flags);

int main(int argc, char **argv) {
    try {
        command_flags flags;

        ict::command line("lt", "Linesight from the cli", 
            "lt [options] xddl_file [message]...\n"
            "lt [options] pcap_file");

        line.add(ict::Option("xml", 'x', "Display message(s) in XML", [&]{ flags.output_xml = true; } ));
        line.add(ict::Option("flat-xml", 'f', "Display message(s) in flat XML", 
            [&]{ flags.output_xml = true; flags.pretty_xml = false;} ));
        line.add(ict::Option("format", 'F', "message columns", flags.format, 
            [&](const std::string & v){ flags.format = v; } ));
        line.add(ict::Option("dom", 'd', "Display xddl dom", [&]{ flags.output_dom = true; } ));

        line.add_note("message : an ASCII hex or binary message string: e.g., 010304 or @11011011");

        line.parse(argc, argv);

        if (line.targets.empty()) IT_THROW("no file given");
        auto filename = line.targets[0];

        if (ict::ends_with(filename, ".xddl")) {
            processXddlFile(line, flags);
        }

        else IT_THROW("unrecognized file: " << filename);
        

    } catch (ict::exception & e) {
        cout << e.what() << endl;
        return 1;
    }
}

void processXddlFile(ict::command const & line, command_flags const & flags) {
    auto i = line.targets.begin();
    ict::spec_server d;
    try { 
        d.clear();
        d.add_spec(i->c_str());
        ++i;

        ict::bitstring bs; // the message to parse
        std::ostringstream inst_dump;
        for (; i != line.targets.end(); ++i) {
            ict::message inst;
            bs = ict::bitstring(i->c_str());

            if (bs.empty()) {
                cout << endl << "  " << *i << 
                    " is not a valid hex or binary message string." << endl;
                line.help();
                exit(0);
            } 
            inst = ict::parse(d, bs);
            if (flags.output_xml) inst_dump << ict::to_xml(inst.root());
            else inst_dump << inst.text(flags.format, true);

            cout << inst_dump.str(); 
            inst_dump.str("");
        }
        if (flags.output_dom) cout << d;
    } catch (ict::exception & e) {
        if (flags.output_dom) cout << d;
        throw;
    }
}

