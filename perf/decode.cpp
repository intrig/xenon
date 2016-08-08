//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <iostream>
#include <vector>
#include <xenon/xenon.h>
#include <xenon/xddl_code.h>
#include <ict/command.h>
#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;
using std::cout;
using std::cerr;

template <typename Op>
void iterate(int n, Op op) {
    ict::timer time;
    time.start();
    for (int i = 0; i < n; ++i) op();
    time.stop();
    cerr << time << '\n';
};

void load_all_xddl(const bf::path & dir, xenon::spec_server & specs) {
    bf::directory_iterator end_itr;
    for (auto i = bf::directory_iterator(dir); i != end_itr; ++i) {
        if (bf::is_directory(i->path())) load_all_xddl(i->path(), specs);
        else if (bf::extension(i->path()) == ".xddl") {
            specs.add_spec(i->path().string());
        }
    }
}

void load_test(const std::string & root, int n) {
    iterate(n, [&](){ 
        xenon::spec_server specs;
        auto p = bf::path(root);
        load_all_xddl(p, specs);
    });
}

void decode(xenon::spec_server & specs, const ict::bitstring & bits, int n) {
    xenon::message m;
    iterate(n, [&](){ m = xenon::parse(specs, bits); });
}

void decode_xml(xenon::spec_server & specs, const ict::bitstring & bits, int n) {
    auto m = xenon::parse(specs, bits);
    iterate(n, [&](){ 
        auto s = xenon::to_xml(m);
    });
}

void decode_pretty(xenon::spec_server & specs, const ict::bitstring & bits, int n) {
    auto m = xenon::parse(specs, bits);
    iterate(n, [&](){ 
        xenon::xml_type xml(xenon::to_xml(m));
        auto x = xml.str();
    });
}

int main(int argc, char** argv) {
    try {
        int iterations = 3000;
        bool pretty = false;
        bool xml = false;
        bool load = false;
        ict::command line("xmlperf", "Decode xenon::message to xml", "xmlperf xddl-dir");
        line.add(ict::option("load", 'l', "Load all xddl files", [&]{ load = true; }));
        line.add(ict::option("xml", 'x', "Convert to flat xml", [&]{ xml = true; }));
        line.add(ict::option("pretty", 'p', "Convert to pretty xml", [&]{ pretty = true; }));
        line.add(ict::option("iterations", 'i', "Number of iterations", std::to_string(iterations), 
            [&](const std::string & i) {
                iterations = std::stoi(i);
            }));

        line.parse(argc, argv);
        if (line.targets.size() != 1) IT_FATAL("Exactly one argument is required, the path to the xddl directory.");


        auto icd = line.targets[0] + "/icd.xddl";
        xenon::spec_server specs(icd);
        auto bits = ict::bitstring("0101046B102C000114E03003603800203801C03801E03801F030037030002030"
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

#if 0
        auto msg = xenon::parse(doc, "0101046B102C000114E03003603800203801C03801E03801F030037030002030"
              "00903000603000E0300120300130380000300110300030380080300200300210"
              "3001903001603000F03000703000403000C03000503000D1A03C9E16C18070DE"
              "2C7CFF3C7CC1001E00E01C000389");

        if (pretty) {
            time.start();
            for (auto i=0; i < iterations; ++i) {
                xenon::xml_type xml(xenon::to_xml(msg));
                auto x = xml.str();
            }
            time.stop();
        } else {
            time.start();
            for (auto i=0; i < iterations; ++i) {
                auto s = xenon::to_xml(msg);
            }
            time.stop();
        }
        cerr << time << '\n';
#endif
        

    } catch (std::exception & e) { 
        cerr << e.what() << '\n';
        return 1;
    }
}
