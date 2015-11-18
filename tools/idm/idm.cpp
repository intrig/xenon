//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/xenon.h>
#include <ict/command.h>
#include <ict/xddl_code.h>

using std::cout;
using std::cerr;
using std::endl;

struct command_flags {
    bool flat_xml = false;
    bool pretty_xml = false;
    bool output_dom = false;

    bool encoding = false;
    bool properties = false;
    bool show_extra = false;
    std::string format = "nlvhs";
};

void processXddlFile(ict::command const & line, command_flags const & flags);

int main(int argc, char **argv) {
    using ict::command;
    using ict::option;
    try {
        command_flags flags;

        command line("idm", "A cli message decoder.", 
            "idm [options] xddl_file [message]...\n");

        line.add(option("encoding", 'e', "Display encoding fields", [&]{ flags.encoding = true; } ));
        line.add(option("properties", 'p', "Display properties", [&]{ flags.properties = true; } ));
        line.add(option("xml", 'x', "Display message(s) in XML", [&]{ flags.pretty_xml = true; } ));
        line.add(option("flat-xml", 'f', "Display message(s) in flat XML", [&]{ flags.flat_xml = true; } ));
        line.add(option("location", 'L', "show xddl source location", [&]{ flags.format += "FL"; } ));
        line.add(option("dom", 'd', "Display xddl dom", [&]{ flags.output_dom = true; } ));
        line.add(option("extra", 'E', "Display extra bits", [&]{ flags.show_extra = true; } ));

        line.add_note("message : an ASCII hex or binary message string: e.g., 010304 or @11011011");

        line.parse(argc, argv);

        if (line.targets.empty()) IT_THROW("no file given");
        auto filename = ict::url(line.targets[0]);

        if (!filename.file.empty()) {
            processXddlFile(line, flags);
        }

        else IT_PANIC("unrecognized file: " << filename);
        

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
        auto u = ict::url(*i);
        auto r = d.add_spec(u.path + u.file);
        ++i;
        
        ict::bitstring bs; // the message to parse
        std::ostringstream inst_dump;

        auto filter = [&](ict::message::const_cursor c) { 
                if (!flags.encoding   && c->is_per()  ) return false;
                if (!flags.properties && c->is_prop() && !c->elem->v->is_visible()) return false;
                if (!flags.show_extra && c->is_extra()) return false;
                return true; };
        for (; i != line.targets.end(); ++i) {
            ict::message inst;
            bs = ict::bitstring(i->c_str());

            if (bs.empty()) {
                cout << endl << "  " << *i << 
                    " is not a valid hex or binary message string." << endl;
                line.help();
                exit(0);
            } 
            ict::spec::cursor start;
            if (!u.anchor.empty()) {
                start = ict::get_record(d, u);
            }
            else start = d.start();
            inst = ict::parse(start, bs);
            if (flags.flat_xml) inst_dump << ict::to_xml(inst.root()) << '\n';
            else if (flags.pretty_xml)  {
                ict::Xml pretty;
                pretty << ict::to_xml(inst.root()) << "\n";
                inst_dump << pretty;
            }
            else inst_dump << ict::to_text(inst, flags.format, filter);

            cout << inst_dump.str(); 
            inst_dump.str("");
        }
        if (flags.output_dom) cout << d;
    } catch (ict::exception & e) {
        if (flags.output_dom) cout << d;
        throw;
    }
}

