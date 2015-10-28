//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/xenon.h>
#include <ict/command.h>

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char **argv) {
    try {
        bool verbose = false;
        std::string xddl_file;
        ict::spec_server ss;

        ict::command line("recombobulator", 
            "Transform a message into one of the same type with random values.", 
            "recombobulator [options] recref message...");
        line.add(ict::option("verbose", 'V', "show progress", [&]{ verbose = true;} ));
        line.add(ict::option("xddl_path", 'X', "XDDL path", "", [&](const std::string & v){ 
            ss.xddl_path.push_back(v);
        }));

        line.add_note("\"For when you didn’t combobulate quite right the first time around.\"");

        line.parse(argc, argv);

        if (line.targets.empty()) IT_PANIC("no arguments given");

        auto u = ict::url(line.targets[0]);

        if (u.file.empty()) IT_THROW("invalid recref: " << line.targets[0]);
        auto start = ss.add_spec(u.path + u.file);
        if (!u.anchor.empty()) {
            start = ict::get_record(ss, u);
        }
        
        for (auto i = line.targets.begin() + 1; i != line.targets.end(); ++i) {
            if (verbose) std::cout << "processing target " << *i << "\n";
            auto m = ict::parse(start, *i);
            cout << ict::to_text(m) << "\n\n";
            ict::recombobulate(m);
            cout << ict::to_text(m) << '\n';
            cout << ict::to_hex_string(ict::serialize(m)) << '\n';
        }

    } catch (ict::exception & e) {
        cerr << e.what() << endl;
        return 1;
    }
}

