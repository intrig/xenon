#include "recref_unit.h"
#include <string>
#include <vector>
#include <xenon/recref.h>

struct ref_type {
    std::string path;
    std::string file;
    std::string anchor;
};

inline bool operator==(const ref_type &x, const ref_type &y) {
    return (x.path == y.path) && (x.file == y.file) && (x.anchor == y.anchor);
}

template <typename Stream>
inline Stream &operator<<(Stream &os, const ref_type &x) {
    os << x.path << x.file << x.anchor;
    return os;
}

void unit::recref_regex() {
    auto v = std::vector<std::pair<std::string, ref_type>>{
        {"", {"", "", ""}},
        {"3GPP/TS-36.331.xddl#DL-DCCH-Message",
         {"3GPP/", "TS-36.331.xddl", "#DL-DCCH-Message"}},
        {"icd.xddl", {"", "icd.xddl", ""}},
        {"xddl/index.xddl", {"xddl/", "index.xddl", ""}},
        {"empty", {"empty/", "", ""}},
        {"#anchor", {"", "", "#anchor"}},
        {"file/anchor", {"", "file.xddl", "#anchor"}},
        {"path/file/anchor", {"path/", "file.xddl", "#anchor"}},
        {"path/to/file/file/anchor", {"path/to/file/", "file.xddl", "#anchor"}},
        {"3GPP/TS-36.331/DL-DCCH-Message",
         {"3GPP/", "TS-36.331.xddl", "#DL-DCCH-Message"}},
    };

    IT_ASSERT(ict::is_directory("/"));
    for (auto &x : v) {
        ref_type y;
        xenon::parse_recref(x.first, y);
        IT_ASSERT_MSG("'" << x.second << "' == '" << y << "'", x.second == y);
    }
}

struct relative_url_type {
    xenon::recref base;
    xenon::recref offset;
    xenon::recref result;
};

void unit::relative_url() {
    std::vector<relative_url_type> url_tests = {
        {"", "", ""},
        {"", "#3.1.1", "#3.1.1"},
        {"", "C.R1001-G.xddl", "C.R1001-G.xddl"},
        {"", "C.R1001-G.xddl#3.1.1", "C.R1001-G.xddl#3.1.1"},

        {"C.S0005.xddl", "", "C.S0005.xddl"},
        {"C.S0005.xddl", "#3.1.1", "C.S0005.xddl#3.1.1"},
        {"C.S0005.xddl", "C.R1001-G.xddl", "C.R1001-G.xddl"},
        {"C.S0005.xddl", "C.R1001-G.xddl#3.1.1", "C.R1001-G.xddl#3.1.1"},

        {"3GPP2/C.S0005.xddl", "", "3GPP2/C.S0005.xddl"},
        {"3GPP2/C.S0005.xddl", "#3.1.1", "3GPP2/C.S0005.xddl#3.1.1"},
        {"3GPP2/C.S0005.xddl", "C.R1001-G.xddl", "3GPP2/C.R1001-G.xddl"},
        {"3GPP2/C.S0005.xddl", "C.R1001-G.xddl#3.1.1",
         "3GPP2/C.R1001-G.xddl#3.1.1"},

        {"3GPP2/C.S0005.xddl", "foo/b.xddl", "3GPP2/foo/b.xddl"},

        {"icd.xddl", "#zero", "icd.xddl#zero"},
        {"3GPP/TS-24.008.xddl", "TS-24.007.xddl#11.2",
         "3GPP/TS-24.007.xddl#11.2"},
        {"3GPP/TS-24.008.xddl", "TS-24.007.xddl#11.2.2.1c",
         "3GPP/TS-24.007.xddl#11.2.2.1c"},
    };

    for (const auto &i : url_tests) {
        auto result = xenon::relative_url(i.base, i.offset);
        IT_ASSERT_MSG(result.str() << " == " << i.result.str(),
                      result == i.result);
    }
}

int main(int, char **) {
    unit test;
    return ict::unit_test<unit>(&test).run();
}
