#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/ict.h>
#include <vector>
#include <string>

namespace xenon {
// find and rfind algorithms
struct path {
    typedef std::vector<std::string>::iterator iterator;
    typedef std::vector<std::string>::const_iterator const_iterator;

    path(const std::vector<std::string> & path, bool abs = true) : p(path), abs(abs) { }

    path(const std::string & path_string) {
        auto ps = path_string;
        if (ps.size() == 0) std::runtime_error("invalid empty path");
        abs = true;
        if (ps[0] == '/') { 
            abs = true;
            ps.erase(0, 1);
            if (ps.size() == 0) std::runtime_error("a lone '/' is an invalid path");
            if (ps[0] == '/') { 
                abs = false;
                ps.erase(0, 1);
                if (ps.size() == 0) std::runtime_error("a lone '//' is an invalid path");
            }
        }
        p = ict::escape_split(ps, '/');
    }

    path(const char * path_string) : path(std::string(path_string)) {}

    iterator begin() { return p.begin(); }
    const_iterator begin() const { return p.begin(); }
    const_iterator cbegin() const { return p.begin(); }
    iterator end() { return p.end(); }
    const_iterator end() const { return p.end(); }
    const_iterator cend() const { return p.end(); }
    bool absolute() const { return abs; }
    void absolute(bool abs) { this->abs = abs; }
    bool empty() const { return p.empty(); }
    size_t size() const { return p.size(); }
    private:
    std::vector<std::string> p;
    bool abs = false;
};

inline std::ostream & operator<<(std::ostream & os, const path & p) {
    if (p.absolute()) os << "/";
    ict::join(os, p.cbegin(), p.cend(), "/");
    return os;
}

template <typename T>  
inline bool leaf_test(const T &) { return 1; }

template <typename T> 
inline std::string name_of(const T & a) { return a.name; }

template <>
inline std::string name_of(const std::string & value) { return value; }

template <>
inline std::string name_of(const int & value) { return ict::to_string(value); }

namespace util {
    template <typename Cursor, typename Op, typename Test>
    inline Cursor rfind_x(Cursor first, const std::string & name, Op op, Test test) {
        typedef typename Cursor::ascending_cursor_type ascending_cursor;
        auto c = ascending_cursor(first);
        while (!c.is_root()) {
            if (op(*c) == name && test(*c)) return c;
            ++c;
        }
        return c;
    }
    template <typename Cursor, typename PathIter, typename Op, typename Test>
    inline Cursor find_x(Cursor parent, PathIter first, PathIter last, Op op, Test test) {
        if (parent.empty()) return parent.end();
        for (auto i = parent.begin(); i!=parent.end(); ++i) {
            if (op(*i) == *first) {
                if (first + 1 == last) {
                    if (test(*i)) return i;
                } else {
                    auto n = find_x(i, first + 1, last, op, test);
                    if (n != i.end()) return n;
                }
            }
        }
        return parent.end();
    }
}

// find given a path
template <typename Cursor, typename Op, typename Test>
inline Cursor find(Cursor parent, const path & path, Op op, Test test) {
    typedef typename Cursor::linear_type iterator;
    if (path.absolute()) return util::find_x(parent, path.begin(), path.end(), op, test);
    else {
        for (iterator i = parent.begin(); i!= parent.end(); ++i) {
            if (op(*i) == *path.begin()) {
                if (path.begin() + 1 == path.end()) {
                    if (test(*i)) return i;
                } else {
                    auto c = Cursor(i);
                    auto x = util::find_x(c, path.begin() + 1, path.end(), op, test);
                    if (x != c.end()) return x;
                }
            }
        }
        return parent.end();
    }
}

#if 0
void match_path(Cursor parent, path & curr_path, const path & path) {
    for (iterator i = parent.begin(); i!= parent.end(); ++i) {
        curr_path += name_of(i);
        if (curr_path.has_tail(path)) f(
    }
}

template <typename Cursor, const path & path, typename Function>
inline Function for_each_path(cursor parent, const path & path, Op f) {
    typedef typename Cursor::linear_type iterator;
    path curr;
    for (iterator i = parent.begin(); i!= parent.end(); ++i) {
        curr = name_of(i);
    }

    if (path.absolute()) return util::find_x(parent, path.begin(), path.end(), op, test);
    else {
        for (iterator i = parent.begin(); i!= parent.end(); ++i) {
            if (op(*i) == *path.begin()) {
                if (path.begin() + 1 == path.end()) {
                    if (test(*i)) return i;
                } else {
                    auto c = Cursor(i);
                    auto x = util::find_x(c, path.begin() + 1, path.end(), op, test);
                    if (x != c.end()) return x;
                }
            }
        }
        return parent.end();
    }
}
#endif
template <typename Cursor, typename Op>
inline Cursor find(Cursor parent, const path & path, Op op) {
    typedef typename Cursor::value_type value_type;
    return find(parent, path, op, [](const value_type &){ return true; });
}

template <typename Cursor>
inline Cursor find(Cursor parent, const path & path) {
    return find(parent, path, name_of<typename Cursor::value_type>);
}

template <typename Cursor, typename Op, typename Test>
inline Cursor rfind(Cursor first, const path & path, Op op, Test test) {
    typedef typename Cursor::ascending_cursor_type ascending_cursor;
    if (!path.absolute()) std::runtime_error("path must be absolute for rfind()");
    if (path.size() == 1) return util::rfind_x(first, *path.begin(), op, test);
    
    auto rfirst = ascending_cursor(first);

    while (!rfirst.is_root()) {
        if (op(*rfirst) == *path.begin()) {
            auto parent = Cursor(rfirst);
            auto x = util::find_x(parent, path.begin() + 1, path.end(), op, test);
            if (x != parent.end()) return x;
        }
        ++rfirst;
    }
    return rfirst;
}

// rfind given a path
template <typename Cursor, typename Op>
inline Cursor rfind(Cursor first, const path & path, Op op) {
    typedef typename Cursor::value_type value_type;
    return rfind(first, path, op, [](const value_type &){ return true; });
}

template <typename Cursor>
inline Cursor rfind(Cursor first, const path & path) {
    return rfind(first, path, name_of<typename Cursor::value_type>);
}

template <typename S, typename C> 
inline void path_string(S & ss, C c) {
    if (!c.is_root()) {
        path_string(ss, c.parent());
        ss << '/';
        ss << name_of(*c);
    }
}

// return the path of a cursor
template <typename T, typename C = typename T::is_cursor> 
inline std::string path_string(T c) {
    std::ostringstream ss;
    path_string(ss, c);
    return ss.str();
}
}
