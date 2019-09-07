#include "loadunit.h"
#include <experimental/filesystem>
#include <xenon/spec_server.h>

namespace fs = std::experimental::filesystem;
using std::cout;

void load_unit::sanity() {}

void load_all_xddl(const fs::path &dir, xenon::spec_server &specs) {
    if (!exists(dir))
        return;
    fs::directory_iterator end_itr;
    for (auto i = fs::directory_iterator(dir); i != end_itr; ++i) {
        if (fs::is_directory(i->path()))
            load_all_xddl(i->path(), specs);
        else if (i->path().extension() == ".xddl")
            specs.add_spec(i->path().string());
    }
}

void load_unit::load_all_specs() {
    xenon::spec_server specs;
    auto p = fs::path("xddl");
    try {
        load_all_xddl(p, specs);
    } catch (const fs::filesystem_error &ex) {
        IT_FORCE_ASSERT("filesystem error: " << ex.what());
    } catch (const std::exception &ex) {
        IT_FORCE_ASSERT(ex.what());
    }
}

int main(int, char **) {
    load_unit test;
    ict::unit_test<load_unit> ut(&test);
    return ut.run();
}
