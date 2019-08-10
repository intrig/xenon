//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/command.h>
#include <xenon/xddl_code.h>
#include <xenon/xenon.h>

using std::cout;
using std::cerr;
using std::endl;

struct command_flags {
    bool flat_xml = false;
    bool pretty_xml = false;
    bool debug_print = false;
    bool output_dom = false;
    bool output_html = false;

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
        line.add(option("html", 'H', "Display in html", [&]{ flags.output_html = true; } ));
        line.add(option("debug", 'D', "Display message(s) in debug gibberish", [&]{ flags.debug_print = true; } ));
        line.add(option("extra", 'E', "Display extra bits", [&]{ flags.show_extra = true; } ));

        line.add_note("message : an ASCII hex or binary message string: e.g., A10304 or @11011011");

        line.parse(argc, argv);

        if (line.targets.empty()) IT_FATAL("no file given");
        auto filename = xenon::recref(line.targets[0]);

        if (!filename.file.empty()) {
            processXddlFile(line, flags);
        }

        else IT_FATAL("unrecognized file: " << filename);
        

    } catch (std::exception & e) {
        cout << e.what() << endl;
        return 1;
    }
}

void processXddlFile(ict::command const & line, command_flags const & flags) {
    auto i = line.targets.begin();
    xenon::spec_server d;
    try { 
        d.clear();
        auto u = xenon::recref(*i);
        d.add_spec(u.path + u.file);
        ++i;
        
        ict::bitstring bs; // the message to parse
        std::ostringstream inst_dump;

        auto filter = [&](xenon::message::const_cursor c) { 
            if (!flags.encoding   && c->is_encoding()  ) return false;
            if (!flags.properties && c->is_prop() && !c->is_visible()) return false;
            if (!flags.show_extra && c->is_extra()) return false;
            return true; };
        for (; i != line.targets.end(); ++i) {
            xenon::message inst;
            bs = ict::bitstring(i->c_str());

            if (bs.empty()) {
                cout << endl << "  " << *i << 
                    " is not a valid hex or binary message string." << endl;
                line.help();
                exit(0);
            } 
            xenon::spec::cursor start;
            if (!u.anchor.empty()) {
                start = xenon::get_record(d, u);
            }
            else start = d.start();
            inst = xenon::parse(start, bs);
            if (flags.flat_xml) inst_dump << xenon::to_xml(inst, filter) << '\n';
            else if (flags.pretty_xml) {
                xenon::xml_type pretty(xenon::to_xml(inst, filter));
                inst_dump << pretty.str();
            }
            else if (flags.debug_print) {
                inst_dump << xenon::to_debug_text(inst, filter) << '\n';
            }
            else inst_dump << xenon::to_text(inst, flags.format, filter);

            cout << inst_dump.str(); 
            inst_dump.str("");
        }
        if (flags.output_dom) {
            if (flags.output_html) cout << xenon::to_html(d.base());
            else cout << d;
        }
    } catch (std::exception &) {
        if (flags.output_dom) cout << d;
        throw;
    }
}

/* 
idm icd.xddl C308D1472EEFC299D40000000000060003018C1FAD90001801520040028200005354AAD3DA18D73B9830140A4800
*/
