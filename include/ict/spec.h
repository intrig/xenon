#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license
#include <ict/xddl.h>
#include <list>
#include <vector>
namespace ict {

class spec {
public:
    /*!!!
     Constructors
     */

    /*!
     Create an empty document.  
     */
    spec() 
    {
        std::string xddlroot = ict::get_env_var("XDDLPATH");
        xddl_path = ict::split(xddlroot, ';');
        xddl_path.push_back(".");
    }

    // TODO get these to work, possibly making doms a shared ptr
    spec(const spec & b) = delete;
    spec& operator=(const spec & b) = delete;

    /*!
     Create a spec from a xddl file.  
     */
    spec(const std::string & filename) 
        : spec() {
        open(filename);
    }

    template <typename InputIterator>
    /*!
     Create a spec from input iterators.  
     */
    spec(InputIterator first, InputIterator last) 
        : spec() {
        add_spec(first, last, "<buffer>");
    }

    /*!
     Clear current spec and open another.
     */
    void open(const std::string & filename) 
    {
        clear();
        add_spec(filename);
    }

    template <typename InputIterator>
    /*!
     Clear current spec and open another.
     */
    void open(InputIterator first, InputIterator last) 
    {
        clear();
        add_spec(first, last, "<buffer>");
    }

    template <typename InputIterator>
    xddl::cursor add_spec(InputIterator first, InputIterator last, const std::string & name) 
    {
        //IT_WARN("adding spec: " << name);
        doms.emplace_back();
        doms.back().owner = this;
        doms.back().open(first, last, name);
        auto root = doms.back().ast.root();
        if (root.empty()) IT_THROW("invalid root node in " << name);
        return root;
    }

    /*!
     Add another spec.
     */
    xddl::cursor add_spec(const std::string & filename) 
    {
        auto i = std::find_if(doms.begin(), doms.end(), [&](const xddl & dom){ return dom.file == filename;} );
        if (i !=doms.end()) return i->ast.root();
        auto x = filename;
        if (!locate(x)) IT_PANIC("cannot open \"" << filename << "\"");
        auto contents = ict::read_file(x);
        return add_spec(contents.begin(), contents.end(), x);
    }

    /*!
     Clear all specs.
     */
    void clear() 
    { doms.resize(0); }

    /*!
     Check for empty.
     */
    bool empty() const 
    { return doms.empty(); }

    friend std::ostream& operator<<(std::ostream &os, const spec & s) {
        for (const auto & d : s.doms) os << d.ast;
        return os;
    }

    xddl & base() { return *(doms.begin()); }
    std::vector<std::string> xddl_path;

    std::list<xddl> doms;
private:

    inline bool locate(std::string & fname) {
        auto ext = ict::extension(fname);
        if (ext != ".xddl") IT_PANIC("File extension must be .xddl: " << fname);
        if (ict::file_exists(fname)) return true;
        if (!ict::is_absolute_path(fname)) {
            for (auto const & path : xddl_path) {
                std::string new_path = path + "/" + fname;
                if (ict::file_exists(new_path)) {
                    fname = new_path;
                    return true;
                }
            }
        }
        return false;
    }
};

} // namespace
