//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <xenon/message.h>
#include <xenon/xddl.h>
#include <xenon/spec_server.h>

struct attr {
    attr(const char * key, const char * value) : key(key), value(std::string(value)) {}
    attr(const char * key, const std::string & value) : key(key), value(value) {}
    attr(const char * key, int64_t value) : key(key), value(ict::to_string(value).c_str()) {}
    attr(const char * key, const ict::bitstring &  value) : key(key), value(to_string(value)) {}

    const char * key;
    std::string value;

};

ict::osstream& operator<<(ict::osstream & strm, const attr & att) {
    if (!att.value.empty()) strm << " " << att.key << "=\"" << att.value << "\"";
    return strm;
}

namespace xenon {

const node_info_list node_info = {
    { "nil", node_info_type::is_nil },

    { "root", node_info_type::is_nil }, // not sure about this
    
    { "extra", node_info_type::is_terminal, [](ict::osstream& os, message::const_cursor n) { 
        os << "<extra" << attr("length", n->bits.bit_size()) << attr("data",to_string(n->bits)) << ">"; } },

    { "field", node_info_type::is_terminal, [](ict::osstream& os, message::const_cursor n) { 
        if (auto f = get_ptr<field>(n->elem->v)) {
            os << "<field" << attr("name", n->name()) << attr("length", n->bits.bit_size()) <<  
                attr("data", to_string(n->bits));
            if (f->bias) {
                // TODO why is this check here?
                if (n->bits.bit_size() <= 32) 
					os << attr("value", n->value()); 
            }
            os << ">";
            description_xml(os, n);
            return;
        }
        if (get_ptr<pad>(n->elem->v)) {
            os << "<field" << attr("name", n->name()) << attr("length", n->bits.bit_size()) <<  
                attr("data", to_string(n->bits)) << ">";
            return;
        }
        IT_PANIC("conversion to xml panic error");
    }},

    { "float", node_info_type::is_terminal, [](ict::osstream& os, message::const_cursor n) { 
        os << "<float" << attr("name", n->name()) << attr("data", n->bits) << ">";
        description_xml(os, n);
    }},

    { "incomplete", node_info_type::is_terminal, [](ict::osstream& os, message::const_cursor n) { 
        os << "<incomplete" << attr("name", n->name()) << attr("value", n->value()) << ">";
    }},

    { "message", node_info_type::is_parent, [](ict::osstream& os, message::const_cursor n) { 
        os << "<message" << attr("docref", n->file()) << ">";
    }},
    
    { "record", node_info_type::is_parent, [](ict::osstream& os, message::const_cursor n) { 
        os << "<record" << attr("name", n->name()) << ">";
    }},
    
    { "repeat", node_info_type::is_parent, [](ict::osstream& os, message::const_cursor n) { 
        os << "<repeat" << attr("name", n->name()) << ">";
    }},

    // repeat record
    { "record", node_info_type::is_parent, [](ict::osstream& os, message::const_cursor n) { 
        os << "<record" << attr("name", n->name()) << ">";
    }},

    { "prop", node_info_type::is_property, [](ict::osstream& os, message::const_cursor n) { 
        os << "<prop" << attr("name", n->name()) << attr("value", n->value()) << ">";
        description_xml(os, n);
    }},
    
    { "setprop", node_info_type::is_property, [](ict::osstream& os, message::const_cursor n) { 
        os << "<setprop" << attr("name", n->name()) << attr("value", n->value()) << ">";
        description_xml(os, n);
    }},

    { "peek", node_info_type::is_property, [](ict::osstream& os, message::const_cursor n) { 
        os << "<peek" << attr("name", n->name()) << attr("value", n->value()) << ">";
    }},

    { "error", node_info_type::is_property, [](ict::osstream& os, message::const_cursor n) { 
        os << "<error" << attr("desc", n->desc) << ">";
    }},
};

// convert a message cursor to xml
namespace util {
    template <typename Cursor, typename Filter>
    void to_xml(ict::osstream &os, Cursor c, Filter filter) {
        node_info[c->type].start_tag(os, c);
        if (!c.empty() && filter(c)) for (auto n = c.begin(); n!= c.end(); ++n) to_xml(os, n, filter);
        os << "</" << node_info[c->type].name << ">";
    }

    template <typename Cursor, typename Filter>
    std::string to_xml(Cursor c, Filter filter) {
        ict::osstream os;
        os << "<message>";
        if (!c.empty()) for (auto n = c.begin(); n!= c.end(); ++n) util::to_xml(os, n, filter);
        os << "</message>";
        return os.take();
    }
}

std::string to_xml(const message & m, 
    std::function<bool(message::const_cursor c)> filter) {
    return util::to_xml(m.root(), filter);
}

// get the file name of a message cursor

template <typename Filter>
void node_text(ht::text_rows & rows, message::const_cursor parent, std::vector<ht::heading> & vh, Filter & filter, int level = 0) {
    ht::text_row row;

    auto first = parent.begin();  // for calculating row
    for (auto n = parent.begin(); n != parent.end(); ++n) {
        if (filter(n)) {
            for (auto & h : vh) {
                std::string curr = "";
                switch (h.type) {
                    case ht::mnemonic : curr = n->mnemonic(); break;
                    case ht::name : 
                    {
                        curr = ict::spaces(level * 2);
                        curr = curr + n->name();

                        break;
                    }
                    case ht::length:
                        if (n->consumes()) curr = ict::to_string(n->length());
                        break;
                    case ht::value:
                        if (n->is_terminal()) {
                            if (n->length() > 64) curr="***";
                            else curr = ict::to_string(n->value());
                        }
                        break;
                    case ht::hex: 
                        if (n->consumes()) {
                            curr = ict::to_string(n->bits);
                            if (curr.size() > 16) {
                                curr.resize(16);
                                curr += "...";
                            }
                        }
                        break;
                    case ht::line: curr = ict::to_string(n->line()); break;
                    case ht::file: curr = n->file(); break;
                    case ht::row: curr = ict::to_string(n - first); break;
                    case ht::description: 
                    {
                        curr = description(n); 
                        break;
                    }
                    default: {}
                }
                h.width = std::max(h.width, curr.size());
                row.push_back(curr);
            }
            rows.push_back(row);
            row.clear();
            if (!n.empty()) node_text(rows, n, vh, filter, level + 1);
        }
    }
}

std::string to_text(const message & m, const std::string & format, 
    std::function<bool(message::const_cursor c)> filter) {
    auto fsv = format;
    std::vector<ht::heading> vh;

    ht::text_row header;
    for (auto v : fsv) {
        ht::heading h;
        switch (v) {
            case 'h' : h.type = ht::hex; break;
            case 'l' : h.type = ht::length; break;
            case 'L' : h.type = ht::line; break;
            case 'F' : h.type = ht::file; break;
            case 'm' : h.type = ht::mnemonic; break;
            case 'n' : h.type = ht::name; break;
            case 's' : h.type = ht::description; break;
            case 'v' : h.type = ht::value; break;
            case 'r' : h.type = ht::row; break;

            default:
                IT_PANIC("unrecognized format: " << v << " in format string " << fsv);
        }
        std::string heading_name{ht::to_name(h.type)};
        h.width = heading_name.size() + 1;
        vh.push_back(h);
        header.push_back(heading_name);
    }

    ht::text_rows rows;

    rows.push_back(header);

    node_text(rows, m.root(), vh, filter);

    ict::osstream os;
    for (auto i = vh.begin(); i != vh.end()-1; ++i) ++i->width;
    for (auto const & row : rows) {
        for (unsigned i =0; i< vh.size(); ++i) {
            os << row[i];
            if (i != vh.size() - 1) os << ict::spaces(vh[i].width - row[i].size());
        }
        os << "\n";
    }

    return os.take();
}

} // namespace

