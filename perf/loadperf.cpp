#include "loadperf.h"

#include <experimental/filesystem>
#include <xenon/ict/command.h>
#include <xenon/xenon.h>

namespace fs = std::experimental::filesystem;
using std::cout;

void load_all_xddl(const fs::path &dir, xenon::spec_server &specs) {
    if (!exists(dir))
        return;
    fs::directory_iterator end_itr;
    for (auto i = fs::directory_iterator(dir); i != end_itr; ++i) {
        if (fs::is_directory(i->path()))
            load_all_xddl(i->path(), specs);
        else if (i->path().extension() == ".xddl") {
            specs.add_spec(i->path().string());
        }
    }
}

void load_unit::load_all_specs() {
    for (auto i = 0; i < iterations; ++i) {
        auto p = fs::path(root);
        xenon::spec_server specs;
        try {
            load_all_xddl(p, specs);
        } catch (const fs::filesystem_error &ex) {
            IT_FORCE_ASSERT("filesystem error: " << ex.what());
        } catch (const std::exception &ex) {
            IT_FORCE_ASSERT(ex.what());
        }
    }
}

int main(int argc, char **argv) {
    try {
        load_unit test;

        ict::command line("xmlperf", "Loading XDDL performance",
                          "xmlperf xddl-dir");
        line.add(ict::option(
            "iterations", 'i', "Number of iterations",
            std::to_string(test.iterations),
            [&](const std::string &i) { test.iterations = std::stoi(i); }));
        line.add_note("load the entire XDDL tree");

        line.parse(argc, argv);
        if (line.targets.size() != 1)
            IT_FATAL("Exactly one argument is required, the path to the xddl "
                     "directory.");
        test.root = line.targets[0];

        ict::unit_test<load_unit> ut(&test);
        return ut.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
