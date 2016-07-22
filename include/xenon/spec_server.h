#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <xenon/xddl.h>
#include <list>
#include <vector>
namespace xenon {

class spec_server {
public:
    /*!!!
     Constructors
     */

    /*!
     Create an empty document.  
     */
    spec_server() {
        std::string xddlroot = ict::get_env_var("XDDLPATH");
        xddl_path = ict::split(xddlroot, ';');
        xddl_path.push_back(".");
    }

    // TODO get these to work, possibly making doms a shared ptr
    spec_server(const spec_server & b) = delete;
    spec_server& operator=(const spec_server & b) = delete;

    /*!
     Create a spec from a xddl file.  
     */
    spec_server(const std::string & filename) 
        : spec_server() {
        add_spec(filename);
    }

    template <typename InputIterator>
    /*!
     Create a spec from input iterators.  
     */
    spec_server(InputIterator first, InputIterator last) 
        : spec_server() {
        add_spec(first, last, "<buffer>");
    }

    template <typename InputIterator>
    spec::cursor add_spec(InputIterator first, InputIterator last, const std::string & name) {
        // IT_WARN("adding spec: " << name);
        doms.emplace_back();
        doms.back().owner = this;
        doms.back().open(first, last, name);
        auto root = doms.back().ast.root();
        if (root.empty()) IT_PANIC("invalid root node in " << name);
        return root.begin();
    }

    /*!
     Add another spec.
     */
    spec::cursor add_spec(const std::string & filename) {
        auto x = filename;
        if (!locate(x)) IT_PANIC("cannot open \"" << filename << "\"");
        auto i = std::find_if(doms.begin(), doms.end(), [&](const spec & dom){ 
            // IT_WARN(dom.file << " == " << x);
            return dom.file == x;} );
        if (i !=doms.end()) return i->ast.root();
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

    
    friend std::ostream& operator<<(std::ostream &os, const spec_server & s) {
        for (const auto & d : s.doms) os << ict::to_text(d.ast);
        return os;
    }

    /*!
     Return the default start record or throw exception.
     */
    spec::cursor start() const {
        if (empty()) IT_PANIC("empty spec list");
        auto root = base().ast.root();
        auto st = find(root, "xddl/start", tag_of);
        if (st == root.end()) IT_PANIC("no <start> element in " << root->parser->file);
        return st;
    }

    spec & base() const { return *(doms.begin()); }
    std::vector<std::string> xddl_path;

    mutable std::list<spec> doms;
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
