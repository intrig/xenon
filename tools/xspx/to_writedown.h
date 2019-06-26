#pragma once
#include "xspx_parser.h"

namespace xspx {

template <typename Elem>
std::string attributes(const Elem & elem) {
    std::ostringstream os;
    if (elem.attributes.empty()) {
        return "\nattributes: none\n\n";
    }
    os << "\nattributes | name  | [type](#AttributeTypes) | required\n";
    os <<   "-----------|-------|-------------------------|---------\n";
    for (auto & a : elem.attributes) {
        if (a.fixed.empty()) {
            os << " | " << a.name << " | " << a.type_name << "|";
            if (a.required) os << " &#10004; ";
            os << " | ";
            os << "\n";
        }
    }
    return os.str();
}

std::string attribute_types(const custom_type_list & types) {
    std::ostringstream os;
    os << "\nType | Default | Description\n";
    os <<   "-----|---------|------------\n";
    for (auto & t : types) {
        os << t.name << " | " << t.def << " | " << t.desc << "\n";
    }
    return os.str();
}

std::string child_elements(const group_list & groups) {
    // just first group for now
    auto dest = groups[0].children;
    std::ostringstream os;
    for (auto & s : dest) {
        s.insert(0, "^");
    }
    return ict::join(dest, ", ");
}

template <typename Elem>
std::string children(xsp_parser const & parser, const Elem & elem) {
    auto & v1 = elem.children;
    std::vector<std::string> dest;
    if (!elem.group_hrefs.empty()) {
        auto i = xenon::find_by_name(parser.groups.begin(), parser.groups.end(), elem.group_hrefs.back());
        auto & v2 = i->children;
        std::set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(), std::inserter(dest, dest.begin()));
    } else {
        dest = v1;
    }

    for (auto & s : dest) {
        s.insert(0, "^");
    }
    if (!elem.group_hrefs.empty()) {
        dest.push_back("[Common Children](#Common-Children)");
    }
    return dest.empty() ? "none" : ict::join(dest, ", ");
}

inline std::string anchor(const std::string & x) {
    std::string y = x;
    std::replace(y.begin(), y.end(), ' ', '-');
    return y;
}

template <typename OS, typename Cursor>
void disp_element(OS & os, xsp_parser const & parser, Cursor c) {
    std::string n = (c->display.empty()) ? c->tag.c_str() : c->display; 
    os << "# " << n << " {\n\n";
    
    os << ":include ex.wd#" << anchor(n) << "/summary?\n";

    if (!c.empty()) {
        for (auto choice = c.begin(); choice!=c.end(); ++choice) {
            disp_element(os, parser, choice);
        }
    } else {
        os << attributes(*c) << "\n\n";

        os << "children: " << children(parser, *c) << "\n\n";
    }

    os << ":include ex.wd#" << anchor(n) << "/detail?\n";

    os << "}\n\n";
}

namespace ict {
template <typename Cursor, typename Pred>
void for_each_cursor(Cursor & parent, Pred op) {
    for (auto c = parent.begin(); c!=parent.end(); ++c) {
        op(c);
    }

}
}

void to_writedown(std::ostream & os, const xsp_parser & xspx) {
    os << ":title XDDL Element Reference\n\n";
    os << ":toc(\"auto\")\n\n";

    auto tree = xspx::elem_tree(xspx);
    os << "# Elements {\n";
    for (auto c = tree.begin(); c != tree.end(); ++c) {
        disp_element(os, xspx, c);
    }    
    os << "} // Elements\n";

    os << "# Attribute Types {\n";
    os << attribute_types(xspx.custom_types);
    os << "} // Attributes\n";

    os << "# Common Children {\n";
    os << child_elements(xspx.groups) << '\n';
    os << "}\n";

}

}
