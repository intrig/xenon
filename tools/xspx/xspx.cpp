//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "xspx_parser.h"

#include <ict/command.h>

#include <iostream>

int main(int argc, char **argv) {

    try {
        xsp_parser p;

        ict::command line("xsp", "Xspec Processor", "xsp [options] xspec-file");

        line.parse(argc, argv);

        if (line.targets.size() != 1) IT_THROW("exactly one xspec-file must be specified");

        p.open(line.targets[0]);

        std::cout << p.header();

    } catch (ict::exception & e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
