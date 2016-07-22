#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/ict.h>

#include <string>
#include <vector>
#include <iostream>

namespace xenon {
    class cpp_code {
        public:
            cpp_code(int starting_indent = 0) : _indent(starting_indent) { }

            // return a standard string of the code nicely indented
            std::string str() {
                std::string out;

                std::vector<std::string> lines;

                for (auto it= _code.begin(); it <  _code.end(); ++it) {
                    out += *it; // add the character to the string

                    switch (*it) {
                        case ':' :
                            //if out ends with "private:" or "public:" then fall through
                            if ((out.find("public:") != std::string::npos) ||
                                (out.find("private:") != std::string::npos)) {
                                ict::normalize(out);
                                lines.push_back(out);
                                out.clear();
                            }
                            break;
                        case '}' :
                        {
                            std::string::iterator next = it + 1;
                            if (!((next != _code.end()) && *next == ';')) {
                                ict::normalize(out);
                                lines.push_back(out);
                                out.clear();
                            }
                            break;
                        }
                        case '>' :
                            if (out.find("#include") != std::string::npos) {
                                ict::normalize(out);
                                lines.push_back(out);
                                out.clear();
                            }
                            break;
                        case '\"':
                        {
                            size_t first = out.find_first_of("\"");
                            size_t last = out.find_last_of("\"");
                            size_t hashtag = out.find("#");
                            if (first != last && hashtag != std::string::npos) {
                                ict::normalize(out);
                                lines.push_back(out);
                                out.clear();
                            }
                            break;
                        }
                        case ';' :
                        case '{' :
                        {
                            // now add it to our vector of lines.
                            ict::normalize(out);
                            lines.push_back(out);
                            out.clear();
                        }
                        case 'f' :
                            if (out.find("#endif") != std::string::npos) {
                                ict::normalize(out);
                                lines.push_back(out);
                                out.clear();
                            }
                            break;

                    }
                }

                // We have a vector of lines now
                // Iterate throw the vector and print them out.

                std::ostringstream os;

                _indent = 0;
                for (auto l=lines.begin(); l!= lines.end(); ++l)
                {
                    std::string line = *l;
                    std::string::reverse_iterator last = line.rbegin();

                    if (last != line.rend()) // the line isn't empty
                    {
                        switch (*last)
                        {
                            case '{' :
                                os << ict::spaces(_indent);
                                _indent += 4;
                                break;
                            case '}' :
                                _indent -= 4;
                                if (_indent < 0) _indent = 0;
                                os << ict::spaces(_indent);
                                break;
                            case ';' :
                                if (line.find("};") != std::string::npos)
                                {
                                    _indent -= 4;
                                    if (_indent < 0) _indent = 0;
                                }
                                os << ict::spaces(_indent);
                                break;
                            case '\"':
                            case '>' :
                                break;
                            case ':' :
                                os << ict::spaces(_indent);
                                break;
                            case '\n':
                                os << ict::spaces(_indent);
                                break;
                        }
                    }
                    os << line << '\n';
                }


                // lets iterate through the code string 
                return os.str();
            }

            void add(std::string const & s)
            {
                // Add a chunk of code. May contain multiple lines (like the above example) 
                // or partial lines.
                _code += s;

            }

            void clear()
            {
                _code = "";
            }

        private:

        int _indent;
        std::string _code;
    };


    inline cpp_code & operator<<(cpp_code & cs, std::string const & str) {
        cs.add(str);
        return cs;
    }
}
