//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.

/* URL Parser
 *
 * This doesn't work with gcc 4.8.  Requires 4.9. So its been removed from build.
 * I think its the std::regex::extended that cuases the issues. */

#include <regex>
#include <ict/xenon.h>
#include <ict/command.h>

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char **argv) {
    try {
        std::string url (argv[1]);
        unsigned counter = 0;

        std::regex url_regex (
                R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
                std::regex::extended
                );
        std::smatch url_match_result;

        std::cout << "Checking: " << url << std::endl;

        if (std::regex_match(url, url_match_result, url_regex)) {
            for (const auto& res : url_match_result) {
                std::cout << counter++ << ": " << res << std::endl;
            }
        } else {
            std::cerr << "Malformed url." << std::endl;
        }
    } catch (std::exception & e) {
        cerr << e.what() << "\n";
        return 1;
    }
}

