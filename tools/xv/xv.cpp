//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <ict/xenon.h>
#include <ict/command.h>

using std::cout;
using std::cerr;
using std::endl;

// globally used flags
bool verbose=false;
std::string xddl_path; 

template <typename T>
std::string location(T t) {
    std::ostringstream os;
    os << " [" << t.xv_file << ":" << t.xv_line << "]";
    return os.str();
}

class Mask {
    public:
    Mask() : path(true), length(true), value(true),  show_desc(true) {}
    bool path;
    bool length;
    bool value;
    bool show_desc;
};

Mask mask;

inline std::string filter_path(const std::string & p) {
    int v;
    auto tokens = ict::split(p, '/');
    for (auto & i : tokens) {
        if (!i.empty() && ict::string_to_int(v, i)) i = "record";
    }

    auto new_p = ict::join(tokens, "/");
    return new_p;
}

class XvField {
    public:
    XvField(ict::message::cursor c) {
        path = ict::path_string(c);
        //path = filter_path(path);
        if (c->length() <= 32) value = c->value();
        else value = 0;
        length = c->length();
        sbs = ict::to_string(c->bits);
        desc = ict::description(c);
        ict::normalize(desc);
        file = c->file();
        xddl_line = c->line();
    }

    XvField(std::string const & str) {
        std::vector<std::string> v = ict::split(str, ':');
        path = v[1];
        ict::normalize(path);
        //path = filter_path(path);
        std::stringstream is(v[2]);
        is >> value;
        std::stringstream ls(v[3]);
        ls >> length;
        sbs = ict::normalize(v[4]);

        // desc may contain ':' characters
        size_t index = str.find(':'); 
        index = str.find(':', index + 1);
        index = str.find(':', index + 1);
        index = str.find(':', index + 1);
        index = str.find(':', index + 1);

        desc = str.substr(index + 1);
        ict::normalize(desc);
    }

    bool operator==(XvField const & rhs) const {
        if (mask.path && path != rhs.path) return false; 
        if (mask.value && value != rhs.value) return false; 
        if (mask.length && length != rhs.length ) return false;
        if (sbs != rhs.sbs) return false;
        if (mask.show_desc && desc != rhs.desc) return false;
        return true;
    }

    bool operator!=(XvField const & rhs) const {
        return !operator==(rhs);
    }

    void to_stream(std::ostream & os) const {
        os << "f: " << path << ": " << value << ": " << length << ": " << sbs << ": " << desc;
    }

    std::string path;
    int64_t value;
    int length;
    std::string sbs;
    std::string desc;

    std::string file;
    int xddl_line;
    size_t xv_line = 0;
    std::string xv_file;
};

class XvMessage {
    public:
    XvMessage() {}

    XvMessage(std::string file, ict::message & m) {
        xddl_file = file;
        bs = ict::serialize(m);
        for (ict::message::linear_cursor n = m.begin(); n!=m.end(); ++n) {
            if (n->is_field()) fields.push_back(XvField(n));
        }
    }

    bool operator==(XvMessage const & rhs) const {
        return ((xddl_file == rhs.xddl_file) && (bs == rhs.bs) && (fields == rhs.fields));
    }

    bool operator!=(XvMessage const & rhs) const {
        return !operator==(rhs);
    }

    bool compare(XvMessage const & rhs) const {
        if (xddl_file != rhs.xddl_file) IT_PANIC("xddl_file doesn't match");

        if (bs != rhs.bs) {
            std::ostringstream os;
            os << "bitstrings don't match: " << ict::to_string(bs) << " != " << ict::to_string(rhs.bs) << location(*this) << "\n";
            to_stream(os);
            auto t = os.str();
            IT_PANIC(t.c_str());
        }

        if (fields != rhs.fields) {
            std::ostringstream os;
            os << "field vectors don't match" << endl;
            os << xddl_file << " " << ict::to_hex_string(bs.begin(), bs.end()) << endl;

            auto li = fields.begin();
            auto ri = rhs.fields.begin();
            for (; li!= fields.end() && ri!=rhs.fields.end(); ++li, ++ri) {
                if (*li != *ri) {
                    os << "+";
                    li->to_stream(os); 
                    os << endl;
                    os << "-";
                    ri->to_stream(os); 
                    os << endl;
                }
            }
            auto t = os.str();
            IT_PANIC(t.c_str());
        }
        return true;
    }

    bool empty() { 
        return xddl_file.empty() && bs.empty() && fields.empty();
    }
    void to_stream(std::ostream & os) const {
        os << "m: " << xddl_file << ": " << ict::to_string(bs) << endl;
        os << "count: " << fields.size() << endl;
        for (auto i=fields.begin(); i!= fields.end(); ++i) {
            i->to_stream(os);
            os << endl;
        }
    }

    size_t xv_line;
    std::string xv_file;
    std::string xddl_file;
    ict::bitstring bs;
    std::vector<XvField> fields;
};

class XvFile {
    public:
    XvFile(std::string const & filename) : name_only(false) {
        std::string line;
        XvMessage msg;
        int line_no = 0;
        std::ifstream file(filename.c_str());
        if (file.is_open()) {
            while (file.good()) {
                std::string s;
                getline(file, s);
                line_no++;
                line = s;

                ict::normalize(line);

                if (line.find("m:") == 0) {
                    if (!msg.empty()) {
                        data.push_back(msg);
                        msg.fields.clear();
                    }

                    auto l = ict::split(line, ':');

                    msg.xddl_file = ict::normalize(l[1]);
                    msg.bs = ict::bitstring(ict::normalize(l[2]).c_str());
                    msg.xv_line = line_no;
                    msg.xv_file = filename;
                }
                else if (line.find("f:") == 0) {
                    XvField f(line);
                    f.xv_file = filename;
                    f.xv_line = line_no;
                    msg.fields.push_back(f);
                }
            }

            if (!msg.empty()) {
                data.push_back(msg);
            }
        }
    }

    bool validate() {
        ict::spec_server doc;
        if (!xddl_path.empty()) doc.xddl_path.push_back(xddl_path);
        ict::message m;
        std::string xddl_file;

        auto i = data.begin();
        for (; i!=data.end(); ++i) {
            if (xddl_file != i->xddl_file) {
                xddl_file = i->xddl_file;
                if (verbose) std::cout << "processing xddl file " << xddl_file << "\n";
                doc.clear();
                doc.add_spec(xddl_file);
            }

            if (verbose) std::cout << "    " << ict::to_string(i->bs) << "\n";
            m = ict::parse(doc, i->bs);

            XvMessage xvm(xddl_file, m);
            if (!i->compare(xvm)) return false;
        }
        return true;
    }

    std::string reprint() {
        std::ostringstream os;
        ict::spec_server doc;
        if (!xddl_path.empty()) doc.xddl_path.push_back(xddl_path);
        ict::message m;
        std::string xddl_file;
        auto i = data.begin();
        for (; i!=data.end(); ++i) {
            if (xddl_file != i->xddl_file) {
                xddl_file = i->xddl_file;
                doc.clear();
                doc.add_spec(xddl_file);
            }

            m = ict::parse(doc, i->bs);
            XvMessage xvm(xddl_file, m);

            xvm.to_stream(os);
            os << endl;
        }
        std::string s = os.str();
        return s;
    }

    std::vector<XvMessage> data;
    bool name_only;

};

void validate_xv_file(const std::string & name, bool name_only, bool force_print) {
    XvFile file(name.c_str());
    file.name_only = name_only;

    if (force_print) cout << file.reprint() << endl;
    else if (!file.validate()) exit(1);
}

void print_xv_message(ict::spec_server & spec, const std::string xddl_file, const std::string ascii_msg) {
    auto m = ict::parse(spec, ict::bitstring(ascii_msg.c_str()));
    XvMessage xm(xddl_file, m);
    xm.to_stream(cout);
    cout << endl;
}


int main(int argc, char **argv) {
    try {
        bool force_print = false;
        std::string xddl_file;
        bool name_only = false;
        bool show_time = false;

        ict::command line("xv", "Validate messages", "xv [options] [xddl-file | xv-file | message]...");
        line.add(ict::option("force-xv", 'f', "force printing in xv format", [&]{ force_print = true; }));
        line.add(ict::option("no-paths", 'p', "don't check paths", [&]{ mask.path = false;} ));
        line.add(ict::option("no-lengths", 'L', "don't check lengths", [&]{mask.length = false;} ));
        line.add(ict::option("no-values", 'v', "don't check values", [&]{mask.value = false;} ));
        line.add(ict::option("no-desc", 'd', "don't check descriptions", [&]{ mask.show_desc = false;} ));
        line.add(ict::option("verbose", 'V', "show progress", [&]{ verbose = true;} ));
        line.add(ict::option("timing", 't', "show timing", [&]{ show_time = true;} ));
        line.add(ict::option("xddl_path", 'X', "XDDL path", "", [&](const std::string & v){ xddl_path = v;} ));

        line.add_note("Arguments (processed in order):");
        line.add_note("  xddl file : XDDL file will be loaded and used for subsequent decoding");
        line.add_note("  xv file : xv file will be validated (or redecoded and printed if -p)");

        line.parse(argc, argv);

        if (line.targets.empty()) IT_PANIC("no arguments given");

        ict::timer timer;
        timer.start();

        ict::spec_server spec2;
        if (!xddl_path.empty()) spec2.xddl_path.push_back(xddl_path);

        for (auto const & arg : line.targets) {
            if (verbose) std::cout << "processing target " << arg << "\n";
            if (ict::ends_with(arg, ".xddl")) {
                // load xddl file, set as spec for future operations
                xddl_file = arg.c_str();
                spec2.clear();
                spec2.add_spec(xddl_file);
            }
            else if (ict::ends_with(arg, ".xv")) {
                // validate an xv file
                validate_xv_file(arg, name_only, force_print);
            } else {
                // print message to stdout
                print_xv_message(spec2, xddl_file, arg);
            }
        }
        timer.stop();
        if (show_time) std::cout << "total time: " << ict::to_string(timer) << "\n";

    } catch (ict::exception & e) {
        cerr << e.what() << endl;
        return 1;
    }
}

