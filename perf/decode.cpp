#include <experimental/filesystem>
#include <iostream>
#include <vector>
#include <xenon/ict/command.h>
#include <xenon/xddl_code.h>
#include <xenon/xenon.h>

namespace fs = std::experimental::filesystem;
using std::cerr;
using std::cout;

template <typename Op> void iterate(int n, Op op) {
    ict::timer time;
    time.start();
    for (int i = 0; i < n; ++i)
        op();
    time.stop();
    cerr << time << '\n';
}

void load_all_xddl(const fs::path &dir, xenon::spec_server &specs) {
    fs::directory_iterator end_itr;
    for (auto i = fs::directory_iterator(dir); i != end_itr; ++i) {
        if (fs::is_directory(i->path()))
            load_all_xddl(i->path(), specs);
        else if (i->path().extension() == ".xddl") {
            specs.add_spec(i->path().string());
        }
    }
}

void load_test(const std::string &root, int n) {
    iterate(n, [&]() {
        xenon::spec_server specs;
        auto p = fs::path(root);
        load_all_xddl(p, specs);
    });
}

void decode(xenon::spec_server &specs, const ict::bitstring &bits, int n) {
    xenon::message m;
    iterate(n, [&]() { m = xenon::parse(specs, bits); });
}

void decode_xml(xenon::spec_server &specs, const ict::bitstring &bits, int n) {
    auto m = xenon::parse(specs, bits);
    iterate(n, [&]() { auto s = xenon::to_xml(m); });
}

void decode_pretty(xenon::spec_server &specs, const ict::bitstring &bits,
                   int n) {
    auto m = xenon::parse(specs, bits);
    iterate(n, [&]() {
        xenon::xml_type xml(xenon::to_xml(m));
        auto x = xml.str();
    });
}

int main(int argc, char **argv) {
    try {
        int iterations = 3000;
        bool pretty = false;
        bool xml = false;
        bool load = false;
        ict::command line("xmlperf", "Decode xenon::message to xml",
                          "xmlperf xddl-dir");
        line.add(ict::option("load", 'l', "Load all xddl files",
                             [&] { load = true; }));
        line.add(ict::option("xml", 'x', "Convert to flat xml",
                             [&] { xml = true; }));
        line.add(ict::option("pretty", 'p', "Convert to pretty xml",
                             [&] { pretty = true; }));
        line.add(ict::option(
            "iterations", 'i', "Number of iterations",
            std::to_string(iterations),
            [&](const std::string &i) { iterations = std::stoi(i); }));

        line.parse(argc, argv);
        if (line.targets.size() != 1)
            IT_FATAL("Exactly one argument is required, the path to the xddl "
                     "directory.");

        auto icd = line.targets[0] + "/icd.xddl";
        xenon::spec_server specs(icd);
        auto bits = ict::bitstring(
            "0101046B102C000114E03003603800203801C03801E03801F030037030002030"
            "00903000603000E0300120300130380000300110300030380080300200300210"
            "3001903001603000F03000703000403000C03000503000D1A03C9E16C18070DE"
            "2C7CFF3C7CC1001E00E01C000389");

        if (load) {
            load_test(line.targets[0], iterations);
        } else if (xml) {
            decode_xml(specs, bits, iterations);
        } else if (pretty) {
            decode_pretty(specs, bits, iterations);
        } else {
            decode(specs, bits, iterations);
        }
    } catch (std::exception &e) {
        cerr << e.what() << '\n';
        return 1;
    }
}
