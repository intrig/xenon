#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <unistd.h>
#include <xenon/ict/ict.h>

namespace fs = std::experimental::filesystem;

// used for quiet mode
struct null_buffer : public std::streambuf {
    int overflow(int c) { return c; }
};

struct section {
    std::string name;
    std::vector<std::string> lines;
};

using section_list = std::vector<section>;

void parse_line(std::ostream &os, const section_list &sects,
                const std::string &line);

inline std::ostream &operator<<(std::ostream &os,
                                const std::vector<std::string> &l) {
    std::for_each(l.begin(), l.end(), [&](auto &l) { os << l << '\n'; });
    return os;
}

inline std::ostream &operator<<(std::ostream &os,
                                const std::vector<char> &buf) {
    std::for_each(buf.begin(), buf.end(), [&](auto &ch) { os << ch; });
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const section &b) {
    os << b.name << '\n' << b.lines;
    return os;
}

// This should go somewhere else.
inline std::string exec_command(const char *cmd) {
    char buffer[128];
    std::ostringstream result;
    result << "# " << cmd << "\n\n";

    auto ret = 0;
    {
        std::shared_ptr<FILE> fd(popen(cmd, "r"),
                                 [&](auto p) { ret = pclose(p); });
        if (!fd)
            throw std::runtime_error("popen() failed!");
        while (!feof(fd.get())) {
            if (fgets(buffer, 128, fd.get()) != NULL)
                result << buffer;
        }
    }
    if (ret != 0)
        IT_PANIC(result.str());
    return result.str();
}

std::string match_one(const std::string &line, const std::regex & ex) {
    std::smatch matches;
    if (std::regex_search(line, matches, ex))
        return matches[1].str();
    return std::string();
}

bool parse_insert(std::ostream &os, auto &sects, auto &line) {
    //std::cout << "checking insert: " << line << '\n';
    const std::regex insert(":insert[[:space:]]+([[:alnum:]-]+)");
    auto m = match_one(line, insert);
    if (m.empty())
        return false;
    //std::cout << "insert found: " << m << '\n';
    auto i = std::find_if(sects.begin(), sects.end(),
                          [&](auto &s) { return s.name == m; });
    if (i == sects.end())
        return true;
        //IT_PANIC("section not found for insert " << m);
    for (auto &l : i->lines) {
        //std::cout << "inserting: " << l << '\n';
        parse_line(os, sects, l);
    }
    return true;
}

bool parse_run(auto &os, auto &sects, auto &line) {
    const std::regex run_ex(":run[[:space:]]+([[:print:]]+)");
    auto m = match_one(line, run_ex);
    if (m.empty())
        return false;
    auto res = exec_command(m.c_str());
    os << "----\n" << res << "----\n";
    return true;
}

bool parse_read(auto &os, auto &sects, auto &line) {
    const std::regex read_ex(":read[[:space:]]+([[:print:]]+)");
    auto m = match_one(line, read_ex);
    if (m.empty())
        return false;
    os << "[source,xml]\n"
          "----\n" << ict::read_file(m) << "----\n";
    return true;
}

void parse_line(std::ostream &os, const section_list &sects,
                const std::string &line) {
    // %bit -> <bit>
    const std::regex xddl_tag("\%([[:alnum:]]+)");

    // ^bit -> <bit>
    const std::regex tag_link("\\^([[:alnum:]]+)");

    if (parse_insert(os, sects, line))
        return;
    if (parse_run(os, sects, line))
        return;
    if (parse_read(os, sects, line))
        return;

    // just a regular line
    auto sub = line;
    sub = regex_replace(sub, xddl_tag, "link:#$1[<$1>]");
    sub = regex_replace(sub, tag_link, "link:#$1[<$1>]");
    os << sub << '\n';
}

auto parse_section_file(auto & file) {
    auto f = ict::read_file(file);
    auto lines = ict::line_split(f.begin(), f.end());
    auto sects = section_list();
    section s;

    std::regex tag(":tag ([[:alnum:]-]+)");

    for (auto &line: lines) {
        auto t = match_one(line, tag);
        if (!t.empty()) {
            if (!s.name.empty()) {
                //std::cout << "adding " << s << '\n';
                sects.push_back(s);
                s.name.clear();
                s.lines.clear();
                line.clear();
            }
            s.name = t;
        }
        if (!s.name.empty())
            s.lines.push_back(line);
    }

    if (!s.name.empty())
        sects.push_back(s);

    return sects;
}

void usage() {
    std::cerr << "process an xspx flavored adoc\n\n"
                 "Usage: procadoc -s sect-file [-q] [file]\n"
                 "  -q : quiet, don't output anything\n"
                 "  -s file : subsection definition file\n"
                 "  file : file to process, or stdin if not provided\n";
}

int main(int argc, char **argv) {
    try {
        std::string src_file;
        std::string sect_file;
        int opt;
        bool quiet = false;
        std::istream *in = &std::cin;
        std::ifstream ifile;

        while ((opt = getopt(argc, argv, "hqs:")) != -1) {
            switch (opt) {
            case 'q':
                quiet = true;
                break;
            case 's':
                sect_file = optarg;
                break;

            case 'h':
            default:
                usage();
                exit(EXIT_FAILURE);
            }
        }

        if (sect_file.empty())
            throw std::invalid_argument("no subsection file");

        auto sects = parse_section_file(sect_file);

        if (optind < argc)
            src_file = argv[optind];
        optind++;

        if (optind < argc)
            throw std::invalid_argument("");

        if (!src_file.empty()) {
            ifile.open(src_file);
            if (!ifile)
                IT_PANIC("cannot open " << src_file);
            in = &ifile;
        }

        std::ostream *out = &std::cout;
        null_buffer nb;
        std::ostream null_stream(&nb);
        if (quiet)
            out = &null_stream;

        std::string line;
        while (std::getline(*in, line))
            parse_line(*out, sects, line);

    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
