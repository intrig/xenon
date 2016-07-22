#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <string>
#include <iostream>
#include <exception>
#include <typeinfo>
#include <cctype>
#include <xenon/xddl_code.h>

class SizeConstraint;
class SubtypeSpec;
class SubtypeValueSet;
class Value;
class ExtensionMarker;
class NamedValue;

extern std::map<std::string, Value *> local_constants;

inline std::string & lcfirst(std::string & s)
{
    if (s.empty()) return s;

    s[0] = std::tolower(s[0]);
    return s;
}

inline std::string var_length() 
{
    std::ostringstream os;
    os << 
      "<enc><field name=''b0'' length=''1''/>"
      "<field name=''length'' length=''b0 ? 15 : 7''/></enc>";

    return os.str();
}

#define PANIC() do { \
    throw ict::create_exception("internal error", __FILE__, __LINE__); \
} while (0)

#define WARN(m) do { \
    std::ostringstream os; \
    os << "warning: " m << " [" << __FILE__ << ":" << __LINE__ <<"]"; \
    std::cerr << os.str() << std::endl; \
} while (0)

extern int current_line;

class ast
{
    public:
    ast() : line(current_line) {}

    virtual std::string str() const { return std::string(); }
    virtual std::string name() const { return std::string(typeid(*this).name()); }
    int line;

};

class Name : public ast
{
    public:
    Name(std::string value) : _name(value) {}
    virtual std::string str() const { return _name; }
    virtual std::string name() const { return _name; }
    std::string qname() const
    {
        std::string n = name();
        if (!n.empty() && n.find('-', 1) != std::string::npos)
        {
            n.insert(n.begin(), '{');
            n += '}';
        }
        return n;
    }

    std::string _name;

};

class NamedNumber : public ast
{
    public:
    NamedNumber(Name * id) : id(id), value(0), is_extension_marker(false) {}
    NamedNumber(Name * id, Value * value) : id(id), value(value), is_extension_marker(false) 
    {
    }
    NamedNumber(ExtensionMarker *) : value(0), is_extension_marker(true) 
	{
		id = new Name("ExtensionMarker");
	}
    virtual std::string name() const { return id->str(); }

    Name * id;
    Value * value;
	bool is_extension_marker;
};

class NamedNumberList : public ast
{
    public:
    NamedNumberList(NamedNumber * number) : extension(false)
    {
        add(number);
    }

    void add(NamedNumber * number) 
    {
		if (number->is_extension_marker) 
        {
            extension = true;
        }
        else {
            items.push_back(number);
            if (extension) ext_items.push_back(number);
            else base_items.push_back(number);
        }
    }

    std::vector<NamedNumber *> base_items;
    std::vector<NamedNumber *> ext_items;
    std::vector<NamedNumber *> items;
	bool extension;
};

class Type : public ast
{
    public:
    Type() : ast(), index(-1), optional(false), extension(false), def_value(0), option_index(-1) {}

    virtual std::string instance() const 
    { 
        std::ostringstream os;
        os << "line " << line << ": undefined instance for type " << typeid(*this).name();
        IT_PANIC(os.str());
    }

    virtual std::string instance(Name *) const 
    { 
        std::ostringstream os;
        os << "line " << line << ": undefined name instance for type " << typeid(*this).name();
        IT_PANIC(os.str());
    }

    virtual std::string instance(Name *, SubtypeSpec *) const 
    { 
        std::ostringstream os;
        os << "line " << line << ": undefined subtype instance for type " << typeid(*this).name();
        IT_PANIC(os.str());
    }

    virtual std::string declaration(Name *) const 
    { 
        std::ostringstream os;
        os << "line " << line << ": undefined declaration for type " << typeid(*this).name();
        IT_PANIC(os.str());
    }

    virtual std::string declaration(Name *, SubtypeSpec *) const 
    { 
        std::ostringstream os;
        os << "line " << line << ": undefined subtype declaration for type " << typeid(*this).name();
        IT_PANIC(os.str());
    }

    virtual std::string value_assignment(Name *, Value *) const
    { 
        std::ostringstream os;
        os << "line " << line << ": undefined value assignment for type " << typeid(*this).name();
        IT_PANIC(os.str());
    }

    virtual bool is_inline() const { return false; }

    std::string qname() const
    {
        std::string n = name();
        if (!n.empty() && n.find('-', 1) != std::string::npos)
        {
            n.insert(n.begin(), '{');
            n += '}';
        }
        return n;
    }

    int index;
    bool optional;
    bool extension;
	NamedValue * def_value;
    int option_index;



};

class ElementTypeList : public ast {
    public:
	ElementTypeList() : extension(false), option_index(0), ext_group_no(1) {}

    void add(Type * item);

    virtual std::string instance() const;

    std::string optional_bitmap(std::vector<Type *> const & item_list) const;

    std::string extension_bitmap(std::vector<Type *> const & item_list) const;

	bool extension; // has an extension
    int option_index;
    std::vector<Type *> items;
    int ext_group_no;
    std::vector<Type *> ext_items;
};

class AlternativeTypeList : public ast {
    public:
	AlternativeTypeList() : extension(false) {}

    void add(Type * item) 
    { 
		if (item->extension) extension = true;
		else {
            if (extension) 
            {
                ext_items.push_back(item);
                item->index = ext_items.size();
            } else {
                items.push_back(item);
                item->index = items.size();
            }
		}
    }

    std::string root_choice() const {
        int range = ict::required_bits(0, items.size() - 1);

        std::ostringstream xml;
        xml << "<enc><field name=''choice'' length=''" << range << "''>";
        int i=0;
        std::vector<Type *>::const_iterator it;

        for (i=0, it = items.begin(); it!=items.end(); ++it)
        {
            xml << "<item key=''" << i++ << "'' value=''" << (*it)->name() << "''/>";
        }
        xml << "</field></enc>";
        
        // now create the switch statement
        xml << "<switch expr=''choice''>";
        for (i=0, it=items.begin(); it!=items.end(); ++it, ++i)
        {
            std::string s = (*it)->instance();
            if (!s.empty()) 
            {
                xml << "<case value=''" << i << "''>" << s << "</case>";
            }
        }
        xml << "</switch>";
        return xml.str();
    }

    std::string ext_choice() const
    {
        std::ostringstream xml;
        xml << "<enc><field name=''b0'' length=''1''/>"
        "<field name=''choice'' length=''b0 ? 15 : 6''>";
        int i=0;
        std::vector<Type *>::const_iterator it;

        for (i=0, it = ext_items.begin(); it!=ext_items.end(); ++it)
        {
            xml << "<item key=''" << i++ << "'' value=''" << (*it)->name() << "''/>";
        }
        xml << "</field></enc>";
        
        // length of the extension
        xml << var_length();
        xml << "<record name=''extension'' length=''length * 8''>";
        // now create the switch statement
        xml << "<switch expr=''choice''>";
        for (i=0, it=ext_items.begin(); it!=ext_items.end(); ++it, ++i)
        {
            std::string s = (*it)->instance();
            if (!s.empty()) 
            {
                xml << "<case value=''" << i << "''>" << s << "</case>";
            }
        }
        xml << "</switch>";
        xml <<"</record>";
        return xml.str();
    }

    virtual std::string instance() const
    {
        std::ostringstream xml;

        if (items.size() == 1 && ext_items.size() == 0)
        {
            xml << "<prop name=''choice'' value=''0'' visible=''true''>" <<
                "<item key=''0'' value=''" << items[0]->name() <<
                "''/></prop>";
            std::string s = (*(items.begin()))->instance();
            xml << s;
        } else {

		// add the extension bit if it exists
            if (extension)
            {
                xml << "<enc><field name=''ext'' length=''1''/></enc>"
                "<switch expr=''ext''>"
                  "<case value=''0''>" << 
                     root_choice() <<
                  "</case>"
                  "<case value=''1''>" << 
                     ext_choice() <<
                  "</case>"
                "</switch>";
            } else {
                xml << root_choice();
            }
        }

        return xml.str();
    }

	bool extension;
    std::vector<Type *> items;
    std::vector<Type *> ext_items;
};

class DefinedType : public Type
{
    public:
    DefinedType(Name *type_name) : type_name(type_name) 
    {
    }

    virtual std::string instance() const;
    virtual std::string instance(Name * name) const;

    virtual std::string declaration(Name * id) const
    {
        std::ostringstream os;
        os << "<record id=''" << id->str() << "''>";
        os <<   "<fragment href=''#" << type_name->str() << "''/>";
        os << "</record>";
        return os.str();
    }
    virtual std::string instance(Name * id, SubtypeSpec *) const
    {
        // TODO: Ignore the subtype spec for now.  This only occurs for "WITH COMPONENTS" and it is unclear if 
        // there is an encoding implications.
        return instance(id);
    }

    virtual std::string name() const { return type_name->str(); }
    Name * type_name;
};

class BuiltinType : public Type
{
};

class SubtypeSpec : public ast
{
    public:

    virtual Value * lower() const = 0;
    virtual Value * upper() const = 0;
    virtual std::string size_str() const = 0;
    std::string size_field();
    bool is_range() const;
};

class SizeConstraint : public SubtypeSpec
{
    public:
    SizeConstraint(SubtypeSpec * spec) : spec(spec) {}
    virtual Value * lower() const { return spec->lower(); }
    virtual Value * upper() const { return spec->upper(); }
    virtual std::string size_str() const;

    SubtypeSpec * spec;
};

class SingleValue : public SubtypeSpec
{
    public:
    SingleValue(Value * value) : value(value) {}

    virtual std::string str() const;

    virtual Value * lower() const { return value; }
    virtual Value * upper() const { return value; }
    virtual std::string size_str() const;

    Value * value;
};

class ContainingSubtype : public SubtypeSpec
{
    public:
    ContainingSubtype(Type * type) : type(type) {}
    virtual Value * lower() const { PANIC(); }
    virtual Value * upper() const { PANIC(); }
    virtual std::string size_str() const { return type->name(); }
    Type * type;
};

class ValueRange : public SubtypeSpec
{
    public:
    ValueRange(Value * lower, Value * upper) : _lower(lower), _upper(upper) {}

    virtual Value * lower() const { return _lower; }
    virtual Value * upper() const { return _upper; }

    virtual std::string str() const;
    virtual std::string size_str() const;

    Value * _lower;
    Value * _upper;
};

class InnerTypeConstraints : public SubtypeSpec
{
    public:
    InnerTypeConstraints() {}
    virtual Value * lower() const { PANIC(); }
    virtual Value * upper() const { PANIC(); }
    virtual std::string size_str() const { PANIC(); }
};

class Subtype : public Type
{
    public:
    Subtype(Type * type, SubtypeSpec * spec) : type(type), spec(spec) {}
    virtual std::string instance() const;
    virtual std::string instance(Name *) const;
    virtual std::string instance(Name *, SubtypeSpec *) const;
    virtual std::string declaration(Name *) const;
    virtual bool is_inline() const { return type->is_inline(); }
    virtual std::string name() const { return type->name(); }

    Type * type;
    SubtypeSpec * spec;
};

class SequenceOfSubtype : public Subtype
{
    public:
    SequenceOfSubtype(Type * type, SizeConstraint * spec) : Subtype(type, spec) {}

    virtual std::string declaration(Name *) const;
    virtual std::string instance() const;
    virtual std::string instance(Name *) const;
    virtual std::string instance(Name *, SubtypeSpec *) const;

    private:
    std::string choice_sequence(Name * id) const;
};


class NamedType : public Type
{
    public:
    NamedType(Name * name, Type * type) : _name(name), type(type) { }

    virtual std::string instance() const
    {
        // if there is a type->def_value then call instance once with def_value not set, to treat it like any ordinary
        // optional value.  Then set it to the type->def_value and call instance again to create itself as just a
        // visible property.
        std::ostringstream os;
        std::string rec = type->instance(_name);

        if (rec.empty()) return "";
        if (optional) os << "<if expr=''option." << qname() << "''>";
        os << rec;
        if (optional) os << "</if>";
        if (def_value) {
            type->def_value = def_value;
            os << "<if expr=''!option." << qname() << "''> <!-- this has a default value, add property -->";
            os << type->instance(_name);
            os << "</if>";
            type->def_value = 0;
        }
        return os.str();
    }

    virtual std::string name() const { return _name->str(); }

    Name * _name;
    Type * type;
};

class SequenceType : public BuiltinType
{
    public:
    SequenceType()  // an empty sequence
    {
        items = new ElementTypeList();
    }
    
    SequenceType(ElementTypeList * items) : items(items) {}
    virtual std::string instance() const
    {
        return items->instance();
    }
    virtual std::string instance(Name *) const;

    virtual std::string declaration(Name * id) const
    {
        std::ostringstream os;
            os << "<record id=''" << id->str() << "''>" << 
                    instance() <<
                  "</record>";
        return os.str();
    }

    ElementTypeList * items;
};

class ChoiceType : public BuiltinType
{
    public:
    ChoiceType(AlternativeTypeList * items) : items(items) {}

    virtual std::string instance() const
    {
        return items->instance();
    }

    virtual std::string declaration(Name * id) const {
        std::ostringstream os;
            os << "<record id=''" << id->str() << "''>" << 
                    instance() <<
                  "</record>";
        return os.str();
    }

    virtual std::string instance(Name * name) const {
        std::ostringstream os;
            os << "<record name=''" << name->str() << "''>" << 
                    instance() <<
                  "</record>";
        return os.str();
    }


    AlternativeTypeList * items;
};

class BooleanType : public BuiltinType
{
    public:
    virtual std::string declaration(Name *) const;
    virtual std::string instance(Name *) const;
};

class IntegerType : public BuiltinType
{
    public:
    virtual std::string name() const { return "value"; } // TODO 
    virtual std::string instance() const;
    virtual std::string instance(Name *) const;
    virtual std::string instance(Name *, SubtypeSpec *) const;
    virtual std::string value_assignment(Name *, Value *) const;
    virtual std::string declaration(Name *, SubtypeSpec *) const;
    virtual bool is_inline() const { return true; }
};

class EnumeratedType : public BuiltinType
{
    public:
    EnumeratedType(NamedNumberList * number_list) : number_list(number_list) {}
    virtual std::string declaration(Name *) const;
    virtual std::string instance() const;
    virtual std::string instance(Name *) const;
    virtual bool is_inline() const { return true; }

    std::string base_enum_field(Name * id, std::vector<NamedNumber *> items) const;
    NamedNumberList * number_list;
};

class BitStringType : public BuiltinType
{
    public:
    virtual std::string declaration(Name *, SubtypeSpec *) const;
    virtual std::string instance(Name *, SubtypeSpec *) const;
    virtual std::string instance(Name * name) const 
    {
        std::ostringstream os;
        os << var_length() << "<!-- BitStringType instance(name) --> <field length=''length'' name=''" << name->str() << "''/>";

        return os.str();
    }
};

class NamedBitStringType : public BuiltinType
{
    public:
    NamedBitStringType(NamedNumberList * number_list) :number_list(number_list)
    {
    }

    virtual std::string declaration(Name *, SubtypeSpec *) const;
    virtual std::string instance(Name *, SubtypeSpec *) const;
    virtual std::string instance(Name * id) const;

    NamedNumberList * number_list;
};

class NullType : public Type
{
    public:
    virtual std::string instance() const { return ""; }
    virtual std::string instance(Name *) const { return ""; }
    virtual std::string instance(Name *, SubtypeSpec *) const { return ""; }
};

class OctetString : public Type
{
    public:
    virtual std::string name() const { return "value"; } // TODO 
    virtual std::string declaration(Name *, SubtypeSpec *) const;
    virtual std::string declaration(Name *) const;
    virtual std::string instance() const 
    {
        std::string lc = name();
        lcfirst(lc);
        return instance(new Name("value"));
    }
    virtual std::string instance(Name * name) const 
    {
        std::ostringstream os;
        os << var_length() << "<field length=''length * 8'' name=''" << name->str() << "''/>";

        return os.str();
    }
    virtual std::string instance(Name *, SubtypeSpec *) const;
    virtual bool is_inline() const { return true; }
};


class Assignment : public ast
{
};

class AssignmentList : public ast
{
    public:
    void add(Assignment * item) { items.push_back(item); }
    virtual std::string str() const
    {
        std::ostringstream os;
        std::vector<Assignment *>::const_iterator it;
        for (it = items.begin(); it!=items.end(); ++it)
        {
            os << (*it)->str();
        }
        return os.str();
    }

    std::vector<Assignment *> items;
};

class TypeAssignment : public Assignment
{
    public:
    TypeAssignment(Name * id, Type * type);

    virtual std::string str() const
    {
        return type->declaration(id);
    }

    Name * id;
    Type * type;
};

class ModuleBody : public ast
{
    public:
    ModuleBody(AssignmentList * items) : items(items) {}

    virtual std::string str() const
    {
        return items->str();
    }

    AssignmentList * items;
};

class ModuleDefinition : public ast
{
    public:
    ModuleDefinition(Name * id, ModuleBody * body) : id(id), body(body) 
    {
    }

    virtual std::string str() const
    {
        std::ostringstream os;
        os << "<!-- Module: " << name() << " -->" << body->str();

        return os.str();
    }

    virtual std::string name() const { return id->str(); }

    Name * id;
    ModuleBody * body;
};

class ModuleList : public ast
{
    public:
    ModuleList() : comments(true) { }

    ModuleList(ModuleDefinition * module) : comments(true) 
    { 
        add(module);
    }

    void add(ModuleDefinition * module)
    {
        modules.push_back(module);
    }

    virtual std::string str() const 
    { 
        std::ostringstream os;
        os << "<?xml version=''1.0'' encoding=''UTF-8''?>"
              "<xddl>";

        std::vector<ModuleDefinition *>::const_iterator it;

        if (comments)
        {
            using namespace std::chrono;
            auto tt = system_clock::to_time_t(system_clock::now());
            os << "<!-- Generated by asnx " << ctime(&tt) << " -->";
            for (it = modules.begin(); it!=modules.end(); ++it)
            {
                os << "<!-- " << (*it)->name() << " -->";
            }

            os << "<!-- This line intentionally left blank -->";
        }

        for (it = modules.begin(); it!=modules.end(); ++it)
        {
            os << (*it)->str();
        }
        os << "<type id=''opt''>"
                "<item key=''0'' value=''not included''/>"
                "<item key=''1'' value=''included''/>"
              "</type>";

        os << "</xddl>";

        return os.str();
    }

    std::vector<ModuleDefinition *> modules;
    bool comments;

};

class Value : public ast
{
    public:
    std::string qstr() const;
};

class BuiltinValue : public Value
{
};

class DefinedValue : public Value
{
    public:
    DefinedValue(Name * id) : id(id) {}
    virtual std::string name() const { return id->str(); }
    virtual std::string str() const { return id->str(); }
    Name * id;
};

class BooleanValue : public BuiltinValue
{
    public:
    BooleanValue(bool value) : value(value) {}
    bool value;
};

class NullValue : public BuiltinValue
{
    public:
    NullValue() {}
};

class SpecialRealValue : public BuiltinValue
{
    public:
    SpecialRealValue(bool value) : value(value) {}
    bool value; // true is +infinity, false is -infinity
};

class SignedNumber : public BuiltinValue
{
    public:
    SignedNumber(std::string const & s) 
    {
        //string_to_int(number, s);
        number = ict::to_integer<int64_t>(s);
    }
    virtual std::string str() const 
    {
        std::ostringstream os;
        os << number;
        return os.str();
    }

    void negate() { number = -number; }
    int64_t number;

};

class HexString : public BuiltinValue
{
    public:
    HexString(std::string const & s)
    {
        ict::string_to_int(number, s, 16);
    }

    int number;
};

class BinaryString : public BuiltinValue
{
    public:
    BinaryString(std::string const & bin) : bin(bin) 
    {
    }

    std::string bin;
    int number;
};

class CharString : public BuiltinValue
{
    public:
    CharString(std::string const & s) : char_str(s) {}

    std::string char_str;
};

class NamedValue : public Value
{
    public:
    NamedValue(Name * id, Value * value) : id(id), value(value) {}
    Name * id;
    Value * value;
};

class ValueAssignment : public Assignment
{
    public:
    ValueAssignment(Name * identifier, Type * type, Value * value);

    virtual std::string str() const
    {
        return type->value_assignment(identifier, value);
    }

    Name * identifier;
    Type * type;
    Value * value;

};

class TypeExtension : public Type
{
	public:
	TypeExtension() 
	{
		extension = true;
	}
};

class ExtensionMarker : public ast
{
};

class TypeExtensionGroup : public Type
{
    public:
    virtual std::string instance() const;
    TypeExtensionGroup(ElementTypeList * items) : items(items) {}
    ElementTypeList * items;
    virtual std::string name() const { return _name; }
    void name(std::string value) { _name = value; }

    private:
    std::string _name;
};

class SymbolList : public ast
{
    public:
    SymbolList(Name * symbol)
    {
        add(symbol);
    }

    void add(Name * symbol)
    {
        items.push_back(symbol);
    }

    std::vector<Name *> items;
};

