#include "to_adoc.h"
#include "xspx_parser.h"
#include <iostream>
#include <xenon/ict/command.h>

int main(int argc, char **argv) {
    try {
        std::string hfile;
        std::string sfile;
        bool dispatch = false;
        bool adoc = false;
        std::string dispatch_name = "dispatch";
        xsp_parser p;

        ict::command line("xspx", "Xspec Processor",
                          "xspx [options] xspec-file");
        line.add(ict::option("header file", 'H', "Output header file", "",
                             [&](std::string s) { hfile = s; }));
        line.add(ict::option("source file", 'S', "Output cpp file", "",
                             [&](std::string s) { sfile = s; }));
        line.add(ict::option("adoc", 'a',
                             "Output adoc documentation file",
                             [&]() { adoc = true; }));
        line.add(ict::option("dispatcher", 'd', "Generate displatch function",
                             dispatch_name, [&](std::string s) {
                                 dispatch = true;
                                 dispatch_name = s;
                             }));

        line.parse(argc, argv);

        if (line.targets.size() != 1)
            IT_FATAL("exactly one xspec-file must be specified");
        if (hfile.empty() && !sfile.empty())
            IT_FATAL("header file must also be specified");

        p.open(line.targets[0]);

        if (adoc) {
            xspx::to_adoc(std::cout, p);
            return 0;
        }
        if (dispatch)
            xspx::to_dispatch(std::cout, p, dispatch_name);
        else if (!hfile.empty() && !sfile.empty()) {
            std::ofstream h(hfile, std::ios::out | std::ios::binary);
            std::ofstream s(sfile, std::ios::out | std::ios::binary);
            p.to_stream(h, s);

        } else if (!hfile.empty()) {
            std::ofstream h(hfile, std::ios::out | std::ios::binary);
            p.to_stream(h);
        } else {
            p.to_stream(std::cout);
        }

    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
