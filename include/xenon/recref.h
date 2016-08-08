#pragma once

#include <ict/ict.h>
#include <string>

namespace xenon {
struct recref {
    recref() {};

    recref(const std::string & x) {
        size_t i = x.find_last_of('/');
        if (i != std::string::npos) {
            ++i;
            path = x.substr(0, i);
        } else i = 0;

        // the rest is filename and anchor
        size_t j = x.find('#', i);

        if (j != std::string::npos) file = x.substr(i, j - i);
        else file = x.substr(i);

        if (j != std::string::npos) anchor = x.substr(j);
    }

    recref(const char * x) : recref(std::string(x)) {}

    void str(ict::osstream &os) const { 
        os << path << file << anchor;
    }
    std::string str() const { 
        ict::osstream os;
        str(os);
        return os.take();
    }

    bool empty() const { return path.empty() && file.empty() && anchor.empty(); };

    bool is_local() const { return path.empty() && file.empty() && !anchor.empty(); }

    bool friend operator<(const recref& a, const recref& b) {
        return a.str() < b.str();
    }

    std::string path; // the path, including the trailing '/', e.g., "3GPP/"
    std::string file; // the filename, e.g. "TS-23.040.xddl"
    std::string anchor;  // the anchor, e.g., "#10.5.3.8"
};

inline bool operator==(const recref & x, const recref & y) {
    return (x.path == y.path) && (x.file == y.file) && (x.anchor == y.anchor);
}

inline std::ostream & operator<<(std::ostream &os, const recref & x) {
    os << x.str();
    return os;
}

// create an absolute recref from a base file and a relative one
inline recref relative_url(recref const & base, recref const & x) {
    ict::osstream os;
    os << base.path << x.path;
    recref result;
    result.path = os.take();
    result.file = x.file.empty() ? base.file : x.file;
    result.anchor = x.anchor;
    return result;
}

}
