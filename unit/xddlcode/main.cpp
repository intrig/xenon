//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "main.h"
#include <IT/XddlCode.h>
#include <IT/it.h>

using namespace IT;
using namespace std;


void CodeUnit::sanity()
{
    Xml xml;

    IT_ASSERT(xml.str() == "");

    xml << "<xxml/>";
    IT_ASSERT_MSG("[" << xml.str() << "]", xml.str() == "<xxml/>\n");
    xml.clear();

    xml << "<xxml a=\"1\"/>";
    IT_ASSERT_MSG(xml.str(), xml.str() == "<xxml a=\"1\"/>\n");
    xml.clear();

    xml << "<xxml a=\"1\" hello=\"goodbye\"/>";
    IT_ASSERT_MSG(xml.str(), xml.str() == "<xxml a=\"1\" hello=\"goodbye\"/>\n");
    xml.clear();

    xml << "<xxml><field/></xxml>";
    xml.str();
    IT_ASSERT_MSG(xml.str(), xml.str() == 
        "<xxml>\n"
        "  <field/>\n"
        "</xxml>\n");
    xml.clear();

    xml << "<hello>hello</hello>";
    IT_ASSERT_MSG(xml.str(), xml.str() == 
        "<hello>hello</hello>\n");

    xml.clear();

    xml << 
    "<hello>"
    "   hello\n"
    "</hello>";

    IT_ASSERT_MSG("[" << xml.str() << "]", xml.str() == 
    "<hello>\n"
    "   hello\n"
    "</hello>\n"
    );

    xml.clear();

    xml << "<?xml version=''1.0'' encoding=''iso-8859-1'' ?>"
              "<xddl version=''2.0000''>"
              "<!-- Module: A  -->" 
              "<xxml><field/></xxml>"
              "</xddl>";

    IT_ASSERT_MSG("[" << xml.str() << "]", xml.str() == 
"<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\n"
"<xddl version=\"2.0000\">\n"
"  <!-- Module: A  -->\n"
"  <xxml>\n"
"    <field/>\n"
"  </xxml>\n"
"</xddl>\n");

    //cout << xml.str();

#if 0
    xml << 
        "<xmml>"
          "<record>"
            "<field/>"
          "</record>"
        "</xmml>";

#endif
    //cout << xml.str();

}

int main (int, char **)
{
    CodeUnit test;
    UnitTest<CodeUnit> ut(&test);
    return ut.run();
}
