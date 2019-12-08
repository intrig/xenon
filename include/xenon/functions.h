#pragma once
#include "ict/ict.h"
#include <string>
#include <vector>

namespace xenon {

// find_first
struct path {
    typedef std::vector<std::string>::iterator iterator;
    typedef std::vector<std::string>::const_iterator const_iterator;

    path(const std::vector<std::string> &path, bool abs = false)
        : p(path), abs(abs) {}

    path(const std::string &path_string) {
        auto ps = path_string;
        if (ps.size() == 0)
            std::runtime_error("invalid empty path");
        if (ps[0] == '/') {
            abs = true;
            ps.erase(0, 1);
            if (ps.size() == 0)
                std::runtime_error("a lone '/' is an invalid path");
            if (ps[0] == '/') {
                abs = false;
                ps.erase(0, 1);
                if (ps.size() == 0)
                    std::runtime_error("a lone '//' is an invalid path");
            }
        }
        p = ict::escape_split(ps, '/');
    }

    path(const char *path_string) : path(std::string(path_string)) {}

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

inline ict::osstream &operator<<(ict::osstream &os, const path &p) {
    if (p.absolute())
        os << "/";
    ict::join(os, p.cbegin(), p.cend(), "/");
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const path &p) {
    if (p.absolute())
        os << "/";
    ict::join(os, p.cbegin(), p.cend(), "/");
    return os;
}

template <typename Cursor, typename Path, typename ForwardIterator,
          typename Action>
// Cursor - SequenceContainer and ForwardIterator
// Path - Container with ForwardIterator
// Action - Lambda Expression that takes a Cursor parameter
//
// parent - parent node in tree
// path - a vector of strings made up from something like "a/b/c"
// curr - current iterator into path
// action - lambda function to be called upon match
inline void for_each_path(Cursor parent, const Path &path, ForwardIterator curr,
                          Action action) {
    for (auto child = parent.begin(); child != parent.end(); ++child) {
        if (*curr == child->name()) {
            if ((curr + 1) == path.end()) {
                action(child);
                for_each_path(child, path, path.begin(), action);
            } else
                for_each_path(child, path, curr + 1, action);
        } else
            for_each_path(child, path, path.begin(), action);
    }
}

template <typename Cursor, typename Action>
void for_each_path(Cursor parent, const path &path, Action action) {
    for_each_path(parent, path, path.begin(), action);
}

// similar to for_each_path, but we return after the first time found
template <typename Cursor, typename ForwardIterator>
inline Cursor find_first(Cursor parent, ForwardIterator first,
                         ForwardIterator last, ForwardIterator curr) {
    for (auto child = parent.begin(); child != parent.end(); ++child) {
        IT_WARN(*curr);
        IT_WARN(child->name());
        IT_WARN("checking " << *curr << " == " << child->name());
        if (*curr == child->name()) {
            if ((curr + 1) == last) {
                return child;
            } else {
                auto c = find_first(child, first, last, curr + 1);
                if (c != child.end())
                    return c;
            }
        } else {
            auto c = find_first(child, first, last, first);
            if (c != child.end())
                return c;
        }
    }
    return parent.end();
}

// absolute version of above
template <typename Cursor, typename ForwardIterator>
inline Cursor find_first_abs(Cursor parent, ForwardIterator first,
                             ForwardIterator last, ForwardIterator curr) {
    for (auto child = parent.begin(); child != parent.end(); ++child) {
        if (*curr == child->name()) {
            if ((curr + 1) == last) {
                return child;
            } else {
                auto c = find_first_abs(child, first, last, curr + 1);
                if (c != child.end())
                    return c;
            }
        }
    }
    return parent.end();
}

template <typename Cursor>
inline Cursor find_first(Cursor parent, const path &path) {
    IT_WARN("\nfind_first " << path);
    return path.absolute()
               ? find_first_abs(parent, path.begin(), path.end(), path.begin())
               : find_first(parent, path.begin(), path.end(), path.begin());
}

template <typename Cursor>
inline Cursor rfind_first(Cursor first, const path &path) {
    typedef typename Cursor::ascending_cursor_type ascending_cursor;
    auto rfirst = ascending_cursor(first);
    while (!rfirst.is_root()) {
        if (rfirst->name() == *path.begin()) {
            auto parent = Cursor(rfirst);
            if (path.size() == 1)
                return parent;
            auto x = find_first_abs(parent, path.begin(), path.end(),
                                    path.begin() + 1);
            if (x != parent.end())
                return x;
        }
        ++rfirst;
    }
    return rfirst;
}

template <typename T> inline bool leaf_test(const T &) { return 1; }

template <typename T> inline std::string name_of(const T &a) { return a.name; }

template <> inline std::string name_of(const std::string &value) {
    return value;
}

template <> inline std::string name_of(const int &value) {
    return ict::to_string(value);
}

template <typename S, typename C> inline void path_string(S &ss, C c) {
    if (!c.is_root()) {
        path_string(ss, c.parent());
        ss << '/';
        ss << name_of(*c);
    }
}

// return the path of a cursor
template <typename T, typename C = typename T::is_cursor>
inline std::string path_string(T c) {
    ict::osstream ss;
    path_string(ss, c);
    return ss.take();
}

} // namespace xenon
