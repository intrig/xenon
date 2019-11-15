#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <xenon/ict/command.h>
#include <xenon/xddl_code.h>

using std::cerr;
using std::cout;
using std::endl;
namespace fs = std::experimental::filesystem;

template <typename T>
inline auto read_text_file(const std::string &filename, T op) {
    if (!fs::exists(filename))
        IT_PANIC(filename << " doesn't exist");

    if (!fs::is_regular_file(filename))
        IT_PANIC(filename << " is not a regular file");

    std::ifstream file(filename.c_str());
    if (!file.good())
        IT_PANIC("cannot open " << filename);

    std::vector<std::string> contents;
    for (std::string line; std::getline(file, line);) {
        if (line.back() == '\r')
            line.pop_back();
        op(line);
    }
}
void convert(const std::string &txt_file) {
    bool asn = false;
    bool skip_next = false;

    read_text_file(txt_file, [&](auto line) {
        if (line.find("-- ASN1START") != std::string::npos)
            asn = true;
        else if (line.find("-- ASN1STOP") != std::string::npos)
            asn = false;
        else if (asn) {
            if (line.find("      ETSI") != std::string::npos)
                skip_next = true;
            else if (skip_next)
                skip_next = false;
            else
                std::cout << line << '\n';
        }
    });
}

int main(int argc, char **argv) {
    std::string xml_patch;
    try {
        ict::command line("asn-strip", "Strip ASN out of text file",
                          "asn-strip textfile");
        line.parse(argc, argv);

        if (line.targets.size() != 1) {
            line.help();
            IT_PANIC("exactly one text file must be specified");
        }
        convert(line.targets[0]);

    } catch (std::exception &e) {
        cerr << e.what() << endl;
        return 1;
    }
}
