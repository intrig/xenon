//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "loadunit.h"

#include <ict/xenon.h>
#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;
using std::cout;

void load_unit::sanity() {
}

void get_all_xddl(const bf::path & dir, std::vector<bf::path> & paths) {
    if (!exists(dir)) return;
    bf::directory_iterator end_itr;
    for (auto i = bf::directory_iterator(dir); i != end_itr; ++i) {
        if (bf::is_directory(i->path())) get_all_xddl(i->path(), paths);
        else if (bf::extension(i->path()) == ".xddl") {
            paths.push_back(i->path());
            //cout << i->path().string() << '\n';
        }
    }

}
void load_unit::load_all_specs() {
    auto root = ict::get_env_var("XDDLPATH");
    if (root.empty()) return;

    auto paths = std::vector<bf::path>();

    auto p = bf::path(root);
    try {
        get_all_xddl(p, paths);
        for (auto & p : paths) {
            cout << "adding spec: " << p << '\n';
            specs.add_spec(p.string());

        }
#if 0
        if (exists(p)) {
          if (bf::is_regular_file(p)) cout << p << " size is " << bf::file_size(p) << '\n';
          else if (bf::is_directory(p)) {
            cout << p << " is a directory containing:\n";
            for (bf::directory_entry& x : bf::directory_iterator(p)) cout << "    " << x.path() << '\n'; 
          }
          else
            cout << p << " exists, but is not a regular file or directory\n";
        }
        else
          cout << p << " does not exist\n";
#endif
      } catch (const bf::filesystem_error& ex) {
        cout << ex.what() << '\n';
      }
}


int main (int, char **) {
    load_unit test;
    ict::unit_test<load_unit> ut(&test);
    return ut.run();
}
