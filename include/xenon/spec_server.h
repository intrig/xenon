#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <xenon/xddl.h>
#include <xenon/recref.h>
#include <list>
#include <vector>

namespace xenon {

class spec_server {
public:
    spec_server() {
        std::string xddlroot = ict::get_env_var("XDDLPATH");
        xddl_path = ict::split(xddlroot, ';');
        xddl_path.push_back(".");
    }

    // TODO get these to work, possibly making doms a shared ptr
    spec_server(const spec_server & b) = delete;
    spec_server& operator=(const spec_server & b) = delete;

    /*!
	 Create a spec and call add_spec() with path parameter.  
     */
    spec_server(const std::string & path) : spec_server() {
        add_spec(path);
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
        doms.emplace_back();
        doms.back().owner = this;
        doms.back().open(first, last, name);
        auto root = doms.back().ast.root();
        if (root.empty()) IT_PANIC("invalid root node in " << name);
        return root.begin();
    }

    /*!
	 Add another spec.  If it is a directory, then add it to the xddl_path list of directories to search.  If it is a
	 file, then load it.  It may be relative to the xddl_path.  Otherwise, throw exception.
     */
    spec::cursor add_spec(const std::string & path) {
        auto p = path;
		ict::tilde_expand(p);
        if (ict::is_directory(p)) xddl_path.insert(xddl_path.begin(), p);
		else {
			if (!locate(p)) IT_PANIC("cannot access \"" << path << "\"");
			auto i = std::find_if(doms.begin(), doms.end(), [&](const spec & dom){ 
				return dom.file == p;} );
			if (i !=doms.end()) return i->ast.root();
			auto contents = ict::read_file(p);
			return add_spec(contents.begin(), contents.end(), p);
		}
        return spec::cursor();
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
        auto xddl = root.begin(); // the first specs <xddl> element
        auto st = std::find_if(xddl.begin(), xddl.end(), [&](const element & e) {
            return e.tag() == "start"; });
        if (st == root.end()) IT_PANIC("no <start> element in " << root->parser->file);
        return st;
    }

    spec & base() const { return *(doms.begin()); }
    std::vector<std::string> xddl_path;

    mutable std::list<spec> doms;
private:

    // given a file name, locate it relative to the xddl_path.  The filename is then mutated to reflect the 
    // true path.
    inline bool locate(std::string & fname) {
        auto ext = ict::extension(fname);
        if (ict::is_file(fname)) return true;
        if (!ict::is_absolute_path(fname)) {
            for (auto const & path : xddl_path) {
                std::string new_path = path + "/" + fname;
                if (ict::is_file(new_path)) {
                    fname = new_path;
                    return true;
                }
            }
        }
        return false;
    }
};

} // namespace
