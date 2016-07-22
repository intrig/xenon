//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <ict/command.h>
#include <xenon/xddl_code.h>

int main(int argc, char **argv)
{
    bool replace = false;
    bool decl = true;
    std::string filename;

    try {
        ict::command line("xml-pp", "Pretty print xml to stdout", "xml-pp [options] xmlfile...");
        line.add(ict::Option("nodecl", 'n', "don't print xml declaration", [&]{ decl = false; }));
        line.add(ict::Option("replace", 'r', "replace file instead", [&]{ replace = true; }));
        line.parse(argc, argv);

        if (line.targets.empty()) IT_FATAL("no xml files specified");

        for (auto const & f : line.targets)
        {
            filename = f;
            xenon::Xml xml(filename, decl);

            if (replace)
            {
                std::ostringstream oss;
                xml.str(oss);
                std::ofstream ofs(filename.c_str());
                ofs << oss.str();

            } else {
                xml.str(std::cout);
            }
        }

    } catch (std::exception & e)
    {
        if (!filename.empty()) std::cerr << "file: " << filename << " ";
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
