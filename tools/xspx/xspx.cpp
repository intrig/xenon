//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "xspx_parser.h"

#include <ict/command.h>

#include <iostream>

int main(int argc, char **argv) {

    try {
        bool dispatch = false;
        xsp_parser p;

        ict::command line("xsp", "Xspec Processor", "xsp [options] xspec-file");
        line.add(ict::option("dispatcher", 'd', "Generate displatch function", [&]{ dispatch = true; } ));

        line.parse(argc, argv);

        if (line.targets.size() != 1) IT_THROW("exactly one xspec-file must be specified");

        p.open(line.targets[0]);

        if (dispatch) xspx::to_dispatch(std::cout, p);
        else std::cout << p.header();

    } catch (ict::exception & e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
