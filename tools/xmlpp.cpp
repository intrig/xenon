#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <xenon/ict/command.h>
#include <xenon/xddl_code.h>

int main(int argc, char **argv) {
    bool decl = true;
    std::string filename;

    try {
        ict::command line("xml-pp", "Pretty print xml to stdout",
                          "xml-pp [options] xmlfile...");
        line.add(ict::Option("nodecl", 'n', "don't print xml declaration",
                             [&] { decl = false; }));
        line.parse(argc, argv);

        if (line.targets.empty())
            IT_FATAL("no xml files specified");

        for (auto const &f : line.targets) {
            filename = f;
            auto v = ict::read_file(filename);
            auto s = std::string(v.begin(), v.end());
            xenon::xml_type xml(s, decl);

            std::cout << xml.str();
        }

    } catch (std::exception &e) {
        if (!filename.empty())
            std::cerr << "file: " << filename << " ";
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
