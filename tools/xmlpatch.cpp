#include <xenon/ict/command.h>
#include <xenon/xddl_code.h>

using std::cerr;
using std::cout;
using std::endl;

class XmlPatch {
  public:
    XmlPatch(std::string filename) : filename(filename), state(New) {
        std::ifstream is(filename.c_str());
        std::string line;

        if (!is.good())
            IT_PANIC("bad patch filename: \"" << filename << "\"");

        while (!is.eof()) {
            std::getline(is, line);
            ict::normalize(line);

            if (line == "patch:") {
                std::pair<std::vector<std::string>, std::vector<std::string>> p;
                patches.push_back(p);
                state = PatchState;
            } else if (line == "old:")
                state = Old;
            else if (line == "new:")
                state = New;
            else if (!line.empty()) {
                switch (state) {
                case Old: {
                    auto &p = patches.back();
                    p.first.push_back(line);
                    break;
                }

                case New: {
                    auto &p = patches.back();
                    p.second.push_back(line);
                    break;
                }

                default:
                    IT_PANIC("unknown state");
                }
            }
        }
    }

    enum State { PatchState, Old, New };

    std::string filename;
    std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>>
        patches;
    State state, last_state;
};

typedef std::vector<
    std::pair<std::vector<std::string>, std::vector<std::string>>>
    Patches;
typedef std::pair<std::vector<std::string>, std::vector<std::string>> Patch;

class XmlSource {
  public:
    XmlSource(std::string filename) : filename(filename) {
        std::ifstream is(filename.c_str());
        std::string line;

        if (!is.good())
            IT_PANIC("bad xml filename: \"" << filename << "\"");

        while (!is.eof()) {
            std::getline(is, line);
            ict::normalize(line);
            lines.push_back(line);
        }
    }

    std::string patch(XmlPatch &patch) {
        Patches::iterator patches_it;
        for (patches_it = patch.patches.begin();
             patches_it != patch.patches.end(); ++patches_it) {
            bool done = false;
            auto it = lines.end();
            do {
                for (auto old_it = patches_it->first.begin();
                     old_it != patches_it->first.end(); ++old_it) {
                    it = find(lines.begin(), lines.end(), *old_it);
                }
                if (it == lines.end())
                    done = true;
                else {
                    *it = "";
                    lines.insert(it, patches_it->second.begin(),
                                 patches_it->second.end());
                }
            } while (!done);
        }
        xenon::xml_type xml(ict::join(lines));
        return xml.str();
    }

    std::string filename;
    std::vector<std::string> lines;
};

int main(int argc, char **argv) {
    std::streambuf *coutbuf = std::cout.rdbuf();
    try {
        std::string ofile;
        ict::command line("xml-patch", "Patch up an xml file",
                          "xml-patch [options] xml-file patch-file");
        line.add(ict::Option("output", 'o', "Output file", "asn.xddl",
                             [&](auto o) { ofile = o; }));
        line.parse(argc, argv);

        if (line.targets.size() != 2)
            IT_PANIC("xml-file and patch-file must be specified");

        std::string xml_file = line.targets[0];
        std::string xml_patch = line.targets[1];

        std::ofstream fs;
        if (!ofile.empty()) {
            fs.open(ofile);
            cout.rdbuf(fs.rdbuf());
        }

        // read the xml
        XmlSource source(xml_file);

        XmlPatch patch(xml_patch);

        cout << source.patch(patch);

    } catch (std::exception &e) {
        cerr << e.what() << endl;
        return 1;
    }
    std::cout.rdbuf(coutbuf);
}
