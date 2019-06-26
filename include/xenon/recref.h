#pragma once

#include <ict/ict.h>
#include <string>

namespace ict {

template <typename Stream, typename I, typename Del>
inline void join(Stream & os, I first, I last, const Del & del) {
    if (first == last) return;
    os << *first;
    ++first;
    while (first != last) {
        os << del << *first;
        ++first;
    }
}

#include <sys/stat.h>
#if defined(WIN32) || defined(WIN64)
    #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
    #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
inline bool exists(std::string const & path) {
    struct stat state;
    return (!stat(path.c_str(), &state));
}

inline bool is_file(std::string const & path) {
    struct stat state; 
    if (stat(path.c_str(), &state)) return false;
    return (S_ISREG(state.st_mode));
}

inline bool is_directory(std::string const & path) {
    struct stat state;
    if (stat(path.c_str(), &state)) return false;
    return S_ISDIR(state.st_mode);
}

inline bool tilde_expand(std::string & path) {
    if (!path.empty()) {
        if (path[0] == '~') {
            auto home = ict::get_env_var("HOME");
            path.replace(0, 1, home);
            return true;
        }
    }
    return false;
}

}
namespace xenon {
template <typename Rec>
void parse_recref(std::string const & ref, Rec & rec) {
    if (ref[0] == '#') {
        if (ref.size() == 1) return;
        rec.anchor = ref;
    } else if (ref.find(".xddl") != std::string::npos) { // old style
        // path/to/file/file.xddl#anchor
        size_t i = ref.find_last_of('/');
        if (i != std::string::npos) {
            ++i;
            rec.path = ref.substr(0, i);
        } else i = 0;

        // the rest is filename and anchor
        size_t j = ref.find('#', i);

        if (j != std::string::npos) rec.file = ref.substr(i, j - i);
        else rec.file = ref.substr(i);

        if (j != std::string::npos) rec.anchor = ref.substr(j);
    } else { // new style, convert it to old style for now
        // path/to/file/file/anchor
        auto v = ict::split(ref, '/');
        switch (v.size()) {
            case 0 :
                break;
            case 1 : 
                rec.path = ref + '/';
                break;
            default :
                rec.anchor += '#';
                rec.anchor += v.back();
                v.pop_back();
                rec.file = v.back() + ".xddl";
                v.pop_back();
                if (v.size()) {
                    ict::osstream os;
                    ict::join(os, v.begin(), v.end(), '/');
                    os << '/';
                    rec.path = os.take();
                }
                break;
        }
    }
}

struct recref {
    recref() {};

    recref(const std::string & x) {
        parse_recref(x, *this);
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

    bool friend operator==(const recref & a, const recref & b) {
        return (a.path == b.path) && (a.file == b.file) && (a.anchor == b.anchor);
    }

    bool friend operator!=(const recref& a, const recref& b) {
        return !(a == b);
    }

    bool friend operator<(const recref& a, const recref& b) {
        return a.str() < b.str();
    }

    std::string path; // the path, including the trailing '/', e.g., "3GPP/"
    std::string file; // the filename, e.g. "TS-23.040.xddl"
    std::string anchor;  // the anchor, e.g., "#10.5.3.8"
};

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
