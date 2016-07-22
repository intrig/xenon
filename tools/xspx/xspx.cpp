//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "xspx_parser.h"

#include <ict/command.h>

#include <iostream>

int main(int argc, char **argv) {

    try {
        bool dispatch = false;
        std::string dispatch_name = "dispatch";
        xsp_parser p;

        ict::command line("xsp", "Xspec Processor", "xsp [options] xspec-file");
        line.add(ict::option("dispatcher", 'd', "Generate displatch function", dispatch_name, [&](std::string s){ 
            dispatch = true;
            dispatch_name = s; } ));

        line.parse(argc, argv);

        if (line.targets.size() != 1) IT_FATAL("exactly one xspec-file must be specified");

        p.open(line.targets[0]);

        if (dispatch) xspx::to_dispatch(std::cout, p, dispatch_name);
        else std::cout << p.header();

    } catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
