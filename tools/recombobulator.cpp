#include <xenon/ict/command.h>
#include <xenon/xenon.h>

using std::cerr;
using std::cout;
using std::endl;

int main(int argc, char **argv) {
    try {
        bool verbose = false;
        std::string xddl_file;
        xenon::spec_server ss;

        ict::command line(
            "recombobulator",
            "Transform a message into one of the same type with random values.",
            "recombobulator [options] recref message...");
        line.add(ict::option("verbose", 'V', "show progress",
                             [&] { verbose = true; }));
        line.add(ict::option(
            "xddl_path", 'X', "XDDL path", "",
            [&](const std::string &v) { ss.xddl_path.push_back(v); }));

        line.parse(argc, argv);

        if (line.targets.empty())
            IT_FATAL("no arguments given");

        auto u = xenon::recref(line.targets[0]);

        if (u.file.empty())
            IT_FATAL("invalid recref: " << line.targets[0]);
        auto start = ss.add_spec(u.path + u.file);
        if (!u.anchor.empty()) {
            start = xenon::get_record(ss, u);
        }

        for (auto i = line.targets.begin() + 1; i != line.targets.end(); ++i) {
            if (verbose)
                std::cout << "processing target " << *i << "\n";
            auto m = xenon::parse(start, *i);
            cout << xenon::to_text(m) << "\n\n";
            xenon::recombobulate(m);
            cout << xenon::to_text(m) << '\n';
            cout << xenon::to_hex_string(xenon::serialize(m)) << '\n';
        }

    } catch (std::exception &e) {
        cerr << e.what() << endl;
        return 1;
    }
}
