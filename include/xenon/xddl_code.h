#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <xenon/xml_parser_base.h>

#include <stack>
#include <map>
#include <memory>

namespace xenon {

    class XmlText
    {
        public:
        XmlText(std::string const & s) : content(s) { }
        std::string content;
    };

    inline std::ostream & operator<<(std::ostream & os, XmlText const & cd)
    {
        std::string::const_iterator it;
        for (it=cd.content.begin(); it!= cd.content.end(); ++it)
        {
            switch (*it)
            {
                case '&' : os << "&amp;"; break;
                case '<' : os << "&lt;"; break;
                case '>' : os << "&gt;"; break;
                //case '\"' : os << "&quot;"; break;
                //case '\'' : os << "&apos;"; break;
                //case '\t' : os << "&tab;"; break;
                default : os << *it;
            }
        }
        return os;
    }

    typedef std::pair<std::string, std::string> attpair;

    class XmlAttribute
    {
        public:
        explicit XmlAttribute(std::string const & s) : content(s) { }
        std::string content;
    };

    template <typename S>
    inline S & operator<<(S & os, XmlAttribute const & cd) {
        for (auto it=cd.content.begin(); it!= cd.content.end(); ++it)
        {
            switch (*it)
            {
                case '&' : os << "&amp;"; break;
                case '<' : os << "&lt;"; break;
                case '>' : os << "&gt;"; break;
                case '\"' : os << "&quot;"; break;
                case '\'' : os << "&apos;"; break;
                case '\t' : os << "&tab;"; break;
                default : os << *it;
            }
        }
        return os;
    }

    template <typename Stream>
    class Xml : public xml_parser_base {
        class XmlNode {
            public:
            virtual ~XmlNode() {}
            virtual Stream & str(Stream & ss, int indent) = 0;
        };

        class XmlComment : public XmlNode {
            public:
            Stream & str(Stream & ss, int indent) {
                ss << ict::spaces(indent) << "<!--" << content << "-->\n";
                return ss;
            }
            std::string content;
        };

        class XmlProcInst : public XmlNode {
            public:

            Stream & str(Stream & ss, int indent) {
                //ict::replace(content, "iso-8859-1", "UTF-8");
                ss << ict::spaces(indent) << "<?" << content << "?>\n";
                return ss;
            }

            std::string content;
        };

        class XmlElement : public XmlNode
        {
            public:
            XmlElement() {}

            XmlElement(std::string name) : name(name) {}

            XmlElement(std::string name, std::map<std::string, std::string> & atts) : name(name), atts(atts) {}

            void clear()
            {
                children.clear();
            }

            bool emptyElement() const
            {
                return cdata.empty() && children.empty();
            }

            void my_sort(std::vector<attpair> & attvec)
            {
                std::vector<attpair> dest;
                attpair name;
                attpair length;
                attpair start;
                attpair id;

                std::sort(attvec.begin(), attvec.end());

                std::vector<attpair>::iterator it;
                for (it = attvec.begin(); it !=attvec.end(); ++it)
                {
                    if (it->first == "name") name = *it;
                    else if (it->first == "length") length = *it;
                    else if (it->first == "start") start = *it;
                    else if (it->first == "id") id = *it;
                    else dest.push_back(*it);
                }

                if (!start.first.empty()) dest.insert(dest.begin(), start);
                if (!length.first.empty()) dest.insert(dest.begin(), length);
                if (!name.first.empty()) dest.insert(dest.begin(), name);

                if (!id.first.empty()) dest.push_back(id);

                attvec = dest;
            }




            std::string attributes()
            {
                if (atts.empty()) return "";

                std::vector<attpair> attvec(atts.begin(), atts.end());

                //std::sort(attvec.begin(), attvec.end(), AttCompare());
                my_sort(attvec);

                std::ostringstream os;

                std::vector<attpair>::iterator it;
                for (it=attvec.begin(); it!=attvec.end(); ++it)
                {
                    XmlAttribute att(it->second);
                    os << ' ' << it->first << "=\"" << att << "\"";
                }
                return os.str();
            }

            Stream & str(Stream & ss, int indent)
            {
                if (name == "&filtered") return ss;

                if (cdata.find_first_not_of(" \t\r\n") == std::string::npos) cdata.clear();

                if (emptyElement())
                {
                    ss << ict::spaces(indent) << "<" << name << attributes() << "/>\n";
                } else if (!cdata.empty()) 
                {
                    std::string ret = "";
                    if (cdata.find('\n') == std::string::npos)
                    {
                        ict::normalize(cdata);
                        ret = "";
                    }
                    XmlText cd(cdata);
                    ss << ict::spaces(indent) << "<" << name << attributes() << ">" <<
                        ret << cd << "</" << name << ">\n";
                } else {
                    ss << ict::spaces(indent) << "<" << name << attributes() << ">\n";

                    for (auto it = children.begin(); it!=children.end(); ++it)
                    {
                        (*it)->str(ss, indent + 2);
                    }
                    ss << ict::spaces(indent) << "</" << name << ">\n";
                }

                return ss;
            }

            std::string name;
            std::map<std::string, std::string> atts;
            //std::string cdata;
            std::string cdata;
            std::vector<std::shared_ptr<XmlNode>> children;

        };

        public:

        Xml() : filter(false), show_decl(true) {
            clear();
        }

        Xml(std::string const & s, bool show_decl = true) : filter(false), show_decl(show_decl) {
            add(s);
        }

        std::string str()
        {
            std::ostringstream ss;
            str(ss);
            return ss.str();
        }

        std::string raw()
        {
            return _code;
        }

private:
        template <typename Str>
        Str & str(Str & ss)
        {
            elements.push(std::make_shared<XmlElement>());

            auto & base = elements.top();

            parse(_code.c_str(), _code.size(), true);

            if (base->children.empty()) return ss;

            for (auto it = base->children.begin(); it!=base->children.end(); ++it)
            {
                (*it)->str(ss, _indent);
            }
            elements.pop(); // remove base
            reset();
            return ss;
        }

public:
        Xml & add(std::string const & s)
        {
            enum State
            {
                Normal,
                Q_Found
            };
            State state = Normal;

            for (auto it = s.begin(); it != s.end(); ++it)
            {
                switch (*it)
                {
                    case '\'' :
                        switch (state)
                        {
                            case Normal : 
                                state = Q_Found;
                                break;
                            case Q_Found :
                                _code += '"';
                                state = Normal;
                                break;
                        }
                        break;

                    default:
                        switch (state)
                        {
                            case Normal : 
                                _code += *it;
                                break;
                            case Q_Found :
                                _code += '\'';
                                _code += *it;
                                state = Normal;
                                break;
                        }
                        break;
                }
            }

            return *this;
        }

        void clear()
        {
            _indent = 0;
            _code = "";
        }

        bool filter;

        private:


        void startElement(const char *name, const char **atts) {
            auto & p = elements.top();

            std::map<std::string, std::string> dict;
            for (const char ** ai=atts; (*ai)[0]!='\0'; ai+=2)
            {
                dict[*ai] = *(ai + 1);
            }
            _cdata.clear();

            auto elem = std::make_shared<XmlElement>();

            std::string n = name;
            if (filter && (n == "prop" || n == "setprop")) n = "&filtered";

            elem->name = n;
            elem->atts = dict;
            elements.push(elem);
            p->children.push_back(elem);
        }

        void endElement(const char *)
        {
            auto & last = elements.top();
            last->cdata = _cdata;
            _cdata.clear();
            elements.pop();
        }
            
        void characterData(const char * data) 
        { 
            //PRINT("_cdata adding [" << data << "]");
            _cdata += data;
        }

        void comment(const char * content) 
        { 
            auto & p = elements.top();
            auto node = std::make_shared<XmlComment>();
            node->content = content;
            p->children.push_back(node);
        }

        void proc_inst(const char * pi) {
            if (show_decl) {
                auto & p = elements.top();
                auto node = std::make_shared<XmlProcInst>();
                node->content = pi;
                p->children.push_back(node);
            }
        }

        bool show_decl;

        int _indent;
        std::string _code;
        std::string _cdata;
        std::stack<std::shared_ptr<XmlElement>> elements;
    };

    using xml_type = Xml<std::ostream>;
}
