//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "specunit.h"

#include <ict/xenon.h>

#include <vector>
#include <string>

doc_unit::doc_unit()
{
}

void doc_unit::sanity()
{
    ict::spec_server doc;
    IT_ASSERT(doc.empty());
}

void doc_unit::constructor_file()
{
    {
        try {
            ict::spec_server doc("xddl/index.xddl");
            IT_ASSERT(!doc.empty());
        }
        catch (ict::exception & e) {
            IT_FORCE_ASSERT(e.what());
        }
    }

    // let's try an empty directory
    {
        std::string error;
        try {
            ict::spec_server doc("empty");
        }
        catch (ict::exception & e) {
            error = e.what();
        }

        IT_ASSERT_MSG("[" << error << "]", 
            error.find("File extension must be") != std::string::npos);
    } 

    // let's try a non-existent directory
    {
        std::string error;
        try {
            ict::spec_server doc("foo");
        } catch (ict::exception & e) {
            error = e.what();
        }
        IT_ASSERT_MSG("[" << error << "]", 
            error.find("File extension must be") != std::string::npos);
    }

    // let's try a non-existent file
    {
        std::string error;
        try {
            ict::spec_server doc("goo.xddl");
        } catch (ict::exception & e) {
            error = e.what();
        }

        IT_ASSERT_MSG("[" << error << "]", 
            error.find("cannot open \"goo.xddl\"") != std::string::npos);
    }

    
    // parse a file that is just a string of characters
    { 
        std::string error;
        try {
            ict::spec_server doc("garbage.xddl");
        } catch (ict::exception & e) {
            error = e.what();
        }
        IT_ASSERT(!error.empty());
    }
    {
        ict::spec_server doc;
        std::string error;
        try {
            IT_ASSERT(doc.empty());

            doc.add_spec("garbage.xddl");
        } catch (ict::exception & e) {
            error = e.what();
        }
        IT_ASSERT(!error.empty());
    }

#if defined(_WIN32)
    {
        std::string error;
        try {
        // The dos way */
            ict::spec_server doc(".\\xddl\\index.xddl");
        } catch (ict::exception & e) {
            error = e.what();
        }
        IT_ASSERT(error.empty());
    }
#endif

}

void doc_unit::parse_file()
{
    ict::spec_server doc;
    std::string error;
    try {
        IT_ASSERT(doc.empty() == true);
        doc.add_spec("nonexexistentfile.xddl");
    } catch (ict::exception & e) {
        error = e.what();
    }
    IT_ASSERT(!error.empty());
}

void doc_unit::absolute()
{
    std::ostringstream abspath;
    abspath << ict::get_env_var("XDDLABS") << "/icd.xddl";

    ict::spec_server doc(abspath.str().c_str());
    IT_ASSERT(!doc.empty());
}

void doc_unit::index2() {
    ict::spec_server doc("xddl/index2.xddl");
    IT_ASSERT(!doc.empty());
}

void doc_unit::index3() {
    ict::spec_server doc("xddl/index3.xddl");
    IT_ASSERT(!doc.empty());
}

void doc_unit::ip_protocol() {
    std::string xddlroot = ict::get_env_var("XDDLROOT");
    std::string ip = xddlroot;
    ip += "/IP/index.xddl";
    ict::spec_server doc(ip.c_str());
    IT_ASSERT(!doc.empty());
}

void doc_unit::icd() {
#if 0 //
    ict::spec doc("icd.xddl");
    IT_ASSERT(!doc.empty());

    std::vector<ict::spec> specs;
    
    for (int i=0; i<100; ++i) specs.push_back(doc);

    for (auto & s : specs) IT_ASSERT(!s.empty());
#endif
}

void doc_unit::fail1() {
    std::string error;
    ict::spec_server doc;
    try {
        ict::spec_server doc3("exprfail.xddl");
    } catch (ict::exception & e)
    {
        error = e.what();
    }
    IT_ASSERT(!error.empty());
}

void doc_unit::fail2() {
    ict::spec_server doc3("exprpass.xddl");
    IT_ASSERT(!doc3.empty());
}


void doc_unit::fail3() {
    try {
        std::string xddl =
            "<xddl>"
              "<start>"
                "<field name=''A'' length=''8''>"
                  "<field name=''doh'' length=''8''/>"
                "</field>"
                "<field name=''B'' length=''8''/>"
                "<field name=''C'' length=''8''/>"
              "</start>"
            "</xddl>";

        ict::spec_server doc(xddl.begin(), xddl.end());

        IT_FORCE_ASSERT("Shouldn't be here");
    } catch (ict::exception & e) {
    }
}


// path should override environment
void doc_unit::search_paths() {
    std::string xddlroot = ict::get_env_var("XDDLROOT");
    ict::spec_server d;

    auto paths = d.xddl_path;
    
    // verify search path is correct
    IT_ASSERT_MSG(paths.size(), paths.size() == 3);
    IT_ASSERT(paths[0] == "xddl");
    IT_ASSERT(paths[1] == xddlroot);
    IT_ASSERT(paths[2] == ".");

    // add one and verify 
    d.xddl_path.push_back("another/path");
    paths = d.xddl_path;
    IT_ASSERT(paths.size() == 4);
    IT_ASSERT(paths[3] == "another/path");

    // now clear and verify empty
    d.xddl_path.clear();
    d.xddl_path.push_back(".");

    paths = d.xddl_path;
    IT_ASSERT(paths.size() == 1); // should be set to "."

    // add one
    d.xddl_path.push_back("xddl2");
    paths = d.xddl_path;
    IT_ASSERT(paths.size() == 2);
    IT_ASSERT(paths[1] == "xddl2");

    d.add_spec("other/index.xddl");
    IT_ASSERT(!d.empty());
}

int main (int, char **) {
    doc_unit test2;
    ict::unit_test<doc_unit> ut_modern(&test2);
    return ut_modern.run();
}
