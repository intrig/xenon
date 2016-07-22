//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "asnast.h"
#include <ict/ict.h>

using namespace std;

map<string, Type *> defined_types;
map<string, Value *> local_constants;

extern bool ignore_warnings;

TypeAssignment::TypeAssignment(Name * id, Type * type) : id(id), type(type) 
{ 
    defined_types[id->str()] = type;
}

string SingleValue::str() const 
{
    return lower()->qstr();
}

string small_uint(const string & name)
{
    ostringstream os;
    os << "<field name=''b0'' length=''1''/>"
      "<field name=''" << name << "'' length=''b0 ? 15 : 6''/>";
    return os.str();
}

string xddl_property(const string & name, const string & value) {
    ostringstream os;
    if (name.empty()) {
        os << "<!-- TODO trying to create a proprerty with empty name -->";
    } else if (value.empty()) {
        os << "<!-- TODO trying to create a proprerty with empty value -->";
    } else {
        os << "<prop name=''" << name << "'' visible=''true''  value=''0''>" <<
        "<item key=''0'' value=''" << value << 
        "''/></prop>";
    }
    return os.str();

}
void ElementTypeList::add(Type * item) 
{ 
    if (item->extension) extension = true;
    else {
        if (item->optional) item->option_index = option_index++;
        if (extension) 
        {
            ext_items.push_back(item);
            item->index = ext_items.size();
            if (TypeExtensionGroup * group = dynamic_cast<TypeExtensionGroup *>(item))
            {
                ostringstream os;
                os << "ext-group-" << item->index;
                group->name(os.str());
            }
        } else {
            items.push_back(item);
            item->index = items.size();
        }
    }
}

string ElementTypeList::optional_bitmap(std::vector<Type *> const & item_list) const
{
    std::vector<Type *> optional;
    std::vector<Type *>::const_iterator it;
    for (it = item_list.begin(); it!=item_list.end(); ++it) {
        if ((*it)->optional) optional.push_back(*it);
    }
    if (optional.empty()) return std::string();

    std::ostringstream xml;
    xml << "<enc>";
    xml << "<record name=''option''>";
    int i = 0;
    for (it = optional.begin(); it!=optional.end(); ++it, ++i) {
        xml << "<field name=''" << (*it)->name() << "'' length=''1'' type=''#opt''/>";
    }

    xml << "</record>";
    xml << "</enc>";
    return xml.str();
    
}

string ElementTypeList::extension_bitmap(std::vector<Type *> const & item_list) const
{
    std::ostringstream xml;
    xml << "<enc>";
    xml << "<field name=''ext-cnt'' length=''7'' bias=''1''/>";
    xml << "<record name=''option'' length=''{ext-cnt}''>";
    int i = 0;
    std::vector<Type *>::const_iterator it;
    for (it = item_list.begin(); it!=item_list.end(); ++it, ++i) {
        xml << "<field name=''" << (*it)->name() << "'' length=''1'' type=''#opt''/>";
    }
    xml << "</record>";
    xml << "</enc>";
    return xml.str();
    
}

string ElementTypeList::instance() const
{
    ostringstream xml;

    if (extension) xml << "<enc><field name=''ext'' length=''1''/></enc>";

    xml << optional_bitmap(items);

    for (vector<Type *>::const_iterator it = items.begin(); it!=items.end(); ++it)
    {
        xml << (*it)->instance();
    }

    if (!ext_items.empty())
    {
        xml << "<if expr=''ext''>";
    
        xml << extension_bitmap(ext_items);

        for (vector<Type *>::const_iterator it = ext_items.begin(); it!=ext_items.end(); ++it)
        {
            xml << (*it)->instance();
        }

        xml << "</if>";
    }

    return xml.str();
}

string SequenceOfSubtype::declaration(Name * id) const
{
    ostringstream os;
    os << "<record id=''" << id->str() << "''>";

    string lc = type->name();
    lcfirst(lc);

    if (spec->is_range())
    {
        os << spec->size_field() << "<repeat num=''count''>";
    } else {
        os << "<repeat num=''" << spec->lower()->qstr() << "''>";
    }
    os <<type->instance() << "</repeat>";

    os << "</record>";
    return os.str();
}

string SequenceOfSubtype::choice_sequence(Name * id) const
{
    ostringstream os;
    os << spec->size_field() <<
          "<repeat name=''" << id->str() << "'' num=''count''>" << type->instance() << "</repeat>";
    return os.str();
}

string SequenceOfSubtype::instance() const
{
    ostringstream os;
    return instance(new Name("value"));
    return os.str();
}

string SequenceOfSubtype::instance(Name * id) const
{
    ostringstream os;
    if (dynamic_cast<ChoiceType *>(type))
    {
        os << choice_sequence(id);
    } else {
        os << spec->size_field();
        os << "<repeat name=''" << id->str() << "'' num=''count''>" << type->instance() << "</repeat>";
    }
    return os.str();
}

string SequenceOfSubtype::instance(Name *, SubtypeSpec *) const
{
    ostringstream os;
    os << "<error/>";
    return os.str();
}

string ValueRange::str() const
{
    ostringstream os;
    os << "(" << (_lower ? _lower->str() : "null") << ", " << (_upper ? _upper->str() : "null") << ")";
    return os.str();
}
string size_string(Value * lower, Value * upper) {
    typedef long long int64_t;

    if (lower == upper) return lower->qstr();

    ostringstream os;
    if (ict::is_integer(lower->qstr()) && ict::is_integer(upper->qstr())) {
        int64_t lb = ict::to_integer<int64_t>(lower->qstr());
        int64_t ub = ict::to_integer<int64_t>(upper->qstr());
        int bits = ict::required_bits(lb, ub);
        // IT_WARN("(" << lower->qstr() << ", " << upper->qstr() << ") = " << bits << " bits");
        os << bits;
    } else {
        os << "size(" << lower->qstr() << ", " << upper->qstr() << ")";
    }
    return os.str();
}

string SizeConstraint::size_str() const
{
    return size_string(lower(), upper());
}

string ValueRange::size_str() const
{
    return size_string(lower(), upper());
}

string BooleanType::declaration(Name * id) const
{
    ostringstream os;
    os << "<record id=''" << id->str() << "''>" << instance(id) << "</record>";
           
    return os.str();
}
string BooleanType::instance(Name * id) const 
{
    ostringstream os;
    os << "<field name=''" << id->str() << "'' length=''1''/>";
    return os.str();
}
string IntegerType::instance() const
{
    ostringstream os;
    os << "<field name=''value'' length=''32''/>";
    return os.str();
}


string IntegerType::instance(Name * id) const
{
    ostringstream os;
    os << "<field name=''" << id->str() << "'' length=''32''/>";
    return os.str();
}

string IntegerType::instance(Name * id, SubtypeSpec * spec) const
{
    ostringstream os;

    if (def_value)
    {
        //os << " IntegerType default=''value(" << def_value->id->str() << ")''";
        //os << xddl_property(id->name(), def_value->id->str()); 
        os << xddl_property(id->str(), def_value->value->name());
    } else {
        os << "<field name=''" << id->str() << "'' length=''" << 
            spec->size_str() << "''";

        string l = spec->lower()->qstr();
        if (l != "0" && spec->is_range()) os << " bias=''" << l << "''";
        os << "/>";
    }

    return os.str();
}

string IntegerType::declaration(Name * id, SubtypeSpec * spec) const
{
    ostringstream os;
    os << "<record id=''" << id->str() << "''>" << instance(id, spec) << "</record>";
    return os.str();
}


string IntegerType::value_assignment(Name * id, Value * value) const
{
    ostringstream os;
    os << "<!-- <prop name=''" << id->str() << "'' value=''" << value->str() << "''/> -->";
    return os.str();
}

ValueAssignment::ValueAssignment(Name * id, Type * type, Value * value) : identifier(id), type(type), 
    value(value) 
{
    local_constants[id->str()] = value;
}

string EnumeratedType::instance() const
{
    return instance(new Name("value"));
}

string EnumeratedType::declaration(Name * id) const
{
    ostringstream os;
    os << "<record id=''" << id->str() << "''>";

    os << instance(id) << "</record>";

    return os.str();
}

string EnumeratedType::base_enum_field(Name * id, vector<NamedNumber *> items) const
{
    ostringstream os;

    if (def_value) {
        os << xddl_property(id->str(), def_value->value->name());
    } else {
        os << "<field name=''" << id->str() << "'' length=''" << ict::required_bits(1, items.size()) << 
            "''>";

        vector<NamedNumber *>::iterator it;
        int i;
        
        for (i = 0, it = items.begin(); it!=items.end(); ++it, ++i)
        {
            os << "<item key=''" << i << "'' value=''" << (*it)->name() << "''/>";
        }
        os << "</field>";
    }
    return os.str();
}

string ext_enum_field(Name * id, vector<NamedNumber *> items)
{
    ostringstream os;
    os << "<enc><field name=''b0'' length=''1''/></enc>"
      "<field name=''" << id->name() << "'' length=''b0 ? 15 : 6''>";

    vector<NamedNumber *>::iterator it;
    int i;
    for (i = 0, it = items.begin(); it!=items.end(); ++it, ++i)
    {
        os << "<item key=''" << i << "'' value=''" << (*it)->name() << "''/>";
    }
    os << "</field>";
    return os.str();
}

string EnumeratedType::instance(Name * id) const 
{ 
    ostringstream os;

    if (number_list->extension)
    {
        os << "<enc><field name=''ext'' length=''1''/></enc>";
        if (number_list->items.size() > 1)
        {
            os << "<switch expr=''ext''>"
              "<case value=''0''>" << 
                 base_enum_field(id, number_list->base_items) << 
              "</case>"
              "<case value=''1''>" << 
                ext_enum_field(id, number_list->ext_items) <<
              "</case>"
            "</switch>";
        }
    } else {
        if (number_list->items.size() > 1)
        {
            os << base_enum_field(id, number_list->base_items);
        } else {
            os << "<prop name=''" << id->name() << "'' visible=''true''  value=''0''>" <<
                "<item key=''0'' value=''" << number_list->items[0]->name() << 
                "''/></prop>";
        }
    }

    return os.str();
}

string BitStringType::declaration(Name * id, SubtypeSpec * spec) const
{
    ostringstream os;
    os << "<record id=''" << id->str() << "''>" << instance(id, spec) << "</record>";
    return os.str();
}

string BitStringType::instance(Name * id, SubtypeSpec * spec) const
{
    ostringstream os;
    if (def_value) {
        os << "<!-- TODO default value for BitStringType -->";
    } else { 
        if (ContainingSubtype * s = dynamic_cast<ContainingSubtype *>(spec))
        {
            os << var_length() << "<record name=''" << id->str() << "'' length=''length'' href=''#" << s->size_str() << "''/>";

        } else if (auto * sc = dynamic_cast<SizeConstraint *>(spec)) {
            if (sc->is_range()) {
                os << sc->size_field();
                os << "<field name=''" << id->str() << "'' length=''count''/>";
            } else {
                os << "<field name=''" << id->str() << "'' length=''" << spec->size_str() << "''/>";
            }
        } else PANIC();
    }
    return os.str();
}

string NamedBitStringType::declaration(Name * name, SubtypeSpec * spec) const
{
    ostringstream os;
    os << "<record id=''" << name->str() << "''>" << instance(name, spec) << "</record>";
    return os.str();
}

string NamedBitStringType::instance(Name *) const
{
    ostringstream os;
    os << "<error/>";
    return os.str();
}

string NamedBitStringType::instance(Name * id, SubtypeSpec * spec) const
{
    ostringstream os;
    os << "<record name=''" <<  id->str() << "'' length=''" << spec->size_str() << "''>";

    vector<NamedNumber *>::iterator it;
    for (it = number_list->items.begin(); it!= number_list->items.end(); ++it)
    {
        //os <<"<field name=''" << (*it)->name() << "'' length=''" << (*it)->value->str() << "''/>";
        os <<"<field name=''" << (*it)->name() << "'' length=''1''/>";
    }

    os << "</record>";

    return os.str();
}

string OctetString::declaration(Name * id) const
{
    ostringstream os;
    os << "<record id=''" << id->str() << "''>" << instance(id) << "</record>";
           
    return os.str();
}
string OctetString::declaration(Name *id, SubtypeSpec *spec) const
{
    ostringstream os;
    os << "<record id=''" << id->str() << "''>" << instance(new Name("value"), spec) << "</record>";
    return os.str();
}

string OctetString::instance(Name * id, SubtypeSpec * spec) const
{
    // TODO: unfortuneate to use dynamic_cast here.  Perhaps something like
    // a virtual spec->octetString(id, this) would be better.
    ostringstream os;
    if (ContainingSubtype * s = dynamic_cast<ContainingSubtype *>(spec))
    {
#if 1
        os << var_length() << 
            "<field name=''octet size'' length=''8''/>" 
            "<record name=''" << id->str() << "'' length=''{octet size} * 8'' href=''#" << s->size_str() << "''/>";
#else
        os << var_length() << "<record name=''" << id->str() << "'' length=''length * 8'' href=''#" << s->size_str() << "''/>";
#endif
    } else if (SizeConstraint * s = dynamic_cast<SizeConstraint *>(spec)) {
        if (s->is_range())
        {
            os << spec->size_field() << 
                "<field name=''" << id->str() << "'' length=''count * 8''/>";
        } else {
            os << "<field name=''" << id->str() << "'' length=''" << s->size_str() << " * 8''/>";
        }
    } else PANIC();
    return os.str();
}


string SingleValue::size_str() const
{
    return value->qstr();
}

string DefinedType::instance(Name * name) const
{
    ostringstream os;
    //os << "<!-- DefinedType::instance(name) -->";

    if (def_value)
    {
        //os << "<!-- DefinedType(name) default=''value(" << def_value->value->name() << ")'' -->";
        os << xddl_property(name->str(), def_value->value->name());
    } else {
        map<string, Type *>::iterator it  = defined_types.find(type_name->str());
        if (it == defined_types.end()) 
        {
            if (!ignore_warnings) WARN("[line " << type_name->line << "] unresolved type reference: " << type_name->str());
        }

        if (it != defined_types.end() && (it->second->is_inline()))
        {
            os << it->second->instance(name);
        } else {

            os << "<record name=''" << name->str() << "'' href=''#" << type_name->str() << "''/>";
        }
    }
    return os.str();
}

string DefinedType::instance() const {
    ostringstream os;
    // os << "<!-- DefinedType::instance() -->";

    map<string, Type *>::iterator it  = defined_types.find(type_name->str());
    if (it == defined_types.end()) {
        if (!ignore_warnings)WARN("[line " << type_name->line << "] unresolved type reference: " << type_name->str());
    }

    if (it != defined_types.end() && (it->second->is_inline())) {
        os << it->second->instance();
    } else {
        os << "<fragment href=''#" << type_name->str() << "''/>";
    }
    if (def_value) {
        os << "<!-- DefinedType default=''value(" << def_value->value->name() << ")'' -->";
    }
    return os.str();
}

string TypeExtensionGroup::instance() const {
    ostringstream os;

    os << 
        "<if expr=''option." << qname() << "''>" << 
          "<field name=''ext-len'' length=''8''/>"
          "<record name=''extension'' length=''{ext-len} * 8''>";

    os << items->instance();

    os <<"</record></if>";
    return os.str();
}       

string SequenceType::instance(Name * n) const { 
    string s = instance();
    if (s.empty()) return "";
    ostringstream os;
    if (n->str().empty()) os << s;
    else os << "<record name=''" << n->str() << "''>" << s << "</record>";
    return os.str();
}

string Subtype::instance() const {
    string lc = type->name();
    lcfirst(lc);
    return type->instance(new Name("value"), spec);
    //return type->instance(new Name(lc), spec);
}

string Subtype::instance(Name * id) const {
    ostringstream os;
    // os << "<!-- Subtype::instance " << id->str() << " -->";
    type->def_value = def_value;
    os << type->instance(id, spec);
    type->def_value = 0;
    return os.str();
}

string Subtype::instance(Name * id, SubtypeSpec *) const {
    // this->type is Integer
    // this->spec is ValueRange (0..2047)
    
    // localspec is ValueRnage (1, 4)
    abort();

    ostringstream os;
    IntegerType * int_type = dynamic_cast<IntegerType *>(type);

    if (int_type)
    {
        os << int_type->instance(id, spec);
    } else { os << "</error>"; }

    return os.str();
}

string Subtype::declaration(Name * id) const {
    return type->declaration(id, spec);
}

string SubtypeSpec::size_field() {
    ostringstream os;
    os << "<enc><field name=''count'' length=''" << size_str() << "''";
    string l = lower()->qstr();
    if (l != "0" && is_range()) os << " bias=''" << l << "''";
    os << "/></enc>";
    return os.str();
}

bool SubtypeSpec::is_range() const { 
    return lower()->str() != upper()->str(); 
}

string Value::qstr() const {
    string n = str();
    
    map<string,Value*>::iterator it;
    it = local_constants.find(n);
    if (it!=local_constants.end())
    {
        return it->second->str();
    }

    if (!n.empty() && n.find('-', 1) != std::string::npos)
    {
        n.insert(n.begin(), '{');
        n += '}';
    }
    return n;
}


