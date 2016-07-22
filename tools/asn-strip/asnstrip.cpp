//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>

#include <ict/command.h>
#include <xenon/xddl_code.h>


using std::cout;
using std::cerr;
using std::endl;

class TextSource
{
    public:
    TextSource(std::string filename) : filename(filename)
    {
        std::ifstream is(filename.c_str());
        std::string line;

        if (!is.good()) IT_PANIC("bad xml filename: \"" << filename << "\"");

        while (!is.eof())
        {
            std::getline(is, line);
            ict::normalize(line);
            lines.push_back(line);
        }
    }

    std::string add_line(std::string const & src)
    {
        if ((src.find("ETSI") == std::string::npos) && (src.find("---------") == std::string::npos)) return src;
        return "";
    }

    std::string asn()
    {
        enum State {
            WaitingForStart,
            Extracting,
        };
        State state = WaitingForStart;
        std::string asn;
        for (auto it=lines.begin(); it!=lines.end(); ++it)
        {
            switch (state)
            {
                case WaitingForStart:
                    if (it->find("DEFINITIONS") != std::string::npos)
                    {
                        asn += add_line(*it);
                        asn += '\n';
                        state = Extracting;
                    }
                    break;
                case Extracting:
                    asn += add_line(*it);
                    asn += '\n';
                    if (it->find("END") !=std::string::npos) state = WaitingForStart;
                    break;
            }
        }
        return asn;
    }

   std::string asn_m()
    {
        enum State {
            WaitingForStart,
            Extracting,
        };

        State state = WaitingForStart;
       std::string asn;

        for (auto it=lines.begin(); it!=lines.end(); ++it)
        {
            // skip page footer text
            if ((it->find("ETSI") ==std::string::npos) && (it->find("---------") == std::string::npos))
            {
                switch (state)
                {
                    case WaitingForStart :
                        if (it->find("-- ASN1START") != std::string::npos) 
                        {
                            //PRINT("start found");
                            state = Extracting;
                        }
                        break;
                    case Extracting :
                        if (it->find("-- ASN1STOP") != std::string::npos) 
                        {
                            //PRINT("end found");
                            state = WaitingForStart;
                        }
                        else 
                        {
                            //PRINT("adding: " << *it);
                            asn += *it;
                            asn += '\n';
                        }
                        break;
                }
            }
        }

        return asn;
    }

    std::string filename;
    std::vector<std::string> lines;
};

int main(int argc, char **argv)
{
    bool markers = false;

    std::string xml_patch;
    try {

        ict::command line("asn-strip", "Strip ASN out of text file", "asn-strip textfile");

        line.add(ict::Option("markers", 'm', "file has ASN1START/ASN1STOP comment markers",
            [&]{ markers = true; }));
        line.parse(argc, argv);

        if (line.targets.size() != 1) 
        {
            line.help();
            IT_PANIC("exactly one text file must be specified");
        }    

        TextSource source(line.targets[0]);

        cout << (markers ? source.asn_m() : source.asn()) << endl;

    } catch (std::exception & e)
    {
        cerr << e.what() << endl;
        return 1;
    }
}
