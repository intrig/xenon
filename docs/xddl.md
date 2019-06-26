#  XDDL Element Reference
* 1 [Elements](#Elements)
    * 1.1 [bit](#bit)
    * 1.2 [case](#case)
    * 1.3 [cstr](#cstr)
    * 1.4 [default](#default)
    * 1.5 [enc](#enc)
    * 1.6 [export](#export)
    * 1.7 [field](#field)
        * 1.7.1 [bias Attribute](#bias-Attribute)
        * 1.7.2 [type Attribute](#type-Attribute)
    * 1.8 [fragment](#fragment)
    * 1.9 [if](#if)
    * 1.10 [item](#item)
    * 1.11 [jump](#jump)
    * 1.12 [oob](#oob)
    * 1.13 [pad](#pad)
        * 1.13.1 [mod](#mod)
    * 1.14 [peek](#peek)
    * 1.15 [prop](#prop)
    * 1.16 [range](#range)
    * 1.17 [record](#record)
        * 1.17.1 [Record Definition](#Record-Definition)
        * 1.17.2 [Record Link](#Record-Link)
    * 1.18 [repeat](#repeat)
        * 1.18.1 [Repeat Indefinitely](#Repeat-Indefinitely)
        * 1.18.2 [Numbered Repeat](#Numbered-Repeat)
        * 1.18.3 [Bound Repeat](#Bound-Repeat)
    * 1.19 [script](#script)
        * 1.19.1 [The description Variable](#The-description-Variable)
        * 1.19.2 [XddlScript Functions](#XddlScript-Functions)
    * 1.20 [setprop](#setprop)
    * 1.21 [start](#start)
    * 1.22 [switch](#switch)
    * 1.23 [type](#type)
        * 1.23.1 [Anonymous Types](#Anonymous-Types)
    * 1.24 [uint16](#uint16)
    * 1.25 [uint32](#uint32)
    * 1.26 [uint64](#uint64)
    * 1.27 [uint8](#uint8)
    * 1.28 [while](#while)
    * 1.29 [xddl](#xddl)
* 2 [Attribute Types](#Attribute-Types)
* 3 [Common Children](#Common-Children)

<h2 id="Elements">1 Elements</h2>
<h2 id="bit">1.1 bit</h2>

A [&lt;bit&gt;](#bit) is a bit.



attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | name | string| &#10004;  | 
 | type | url| | 
 | bias | integer| | 
 | default | expression| | 


children: [comment](#comment), [item](#item), [range](#range), [script](#script)


The following example is a one bit message:

    <xddl>
      <bit name="x"/>
    </xddl>
And decoding a bit with [idm](#idm):

    # idm obp.xddl @1
    
    Name  Length  Value  Hex  Description
    x     1       1      @1
<h2 id="case">1.2 case</h2>

The [&lt;case&gt;](#case) element only appears as a child of the [&lt;switch&gt;](#switch) element.  It is similar to the *case* keyword in 
*C++*.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | value | integer| &#10004;  | 


children: [Common Children](#Common-Children)


See the [&lt;switch&gt;](#switch) element for example usage.
<h2 id="cstr">1.3 cstr</h2>

The [&lt;cstr&gt;](#cstr) element represents a null-terminated C string.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | max | expression| | 
 | name | string| &#10004;  | 


children: none



Example:

    <xddl>
      <cstr name="greeting"/>
    </xddl>
Decoding the ASCII hex for "Hello" yields:

    # idm cstr.xddl 48656C6C6F00
    
    Name     Length  Value          Hex           Description
    greeting 48      79600447942400 #48656C6C6F00 Hello
<h2 id="default">1.4 default</h2>

The [&lt;default&gt;](#default) element only appears as a child of the [&lt;switch&gt;](#switch) element.  It is similar to the *default* keyword in 
*C/C++*.

attributes: none



children: [Common Children](#Common-Children)



See the [&lt;switch&gt;](#switch) element for example usage.
<h2 id="enc">1.5 enc</h2>

The [&lt;enc&gt;](#enc) element is used to encapsulate encoding fields. Encoding fields are by default not displayed in the [idm](#idm). 

attributes: none



children: [fragment](#fragment), [start](#start), [type](#type), [Common Children](#Common-Children)



Here is an example that reads a `size` field and then a field of that length.  
In this case, we consider `size` to be an encoding field, and not an important part of the message for display 
purposes.  We surround it with [&lt;enc&gt;](#enc) to indicate so.

    <xddl>
      <enc>
        <uint8 name="size"/>
      </enc>
      <field name="value" length="size"/>
    </xddl>
And decoding `080F` with the [idm](#idm) skips over the `length` field:

    # idm enc.xddl 080F
    
    Name  Length  Value  Hex  Description
    value 8       15     #0F
But running [idm](#idm) with the `--encoding` flag will display it:

    # idm --encoding enc.xddl 080F
    
    Name  Length  Value  Hex  Description
    size  8       8      #08
    value 8       15     #0F
However, records inserted within an encoding range are themselves not considered to be encoding.
<h2 id="export">1.6 export</h2>

[&lt;export&gt;](#export) provides a way to create global properties in a message.  These properties can be used and set by different 
records as a message is being parsed.  

attributes: none



children: [prop](#prop)



The following file creates a global property, *size*.  The *A* record references this size and creates a field
based on its length.  The *B* record also creates a field based on the value of *size*, but since there is a local
*size* property defined in its scope (with the value of 16), it is used instead.

    <xddl>
      <export>
        <prop name="size" value="8"/>
      </export>
      <record id="A">
        <field name="b" length="size"/>
      </record>
      <record id="B">
        <prop name="size" value="16"/>
        <field name="b" length="size"/>
      </record>
      <start>
        <record name="A" href="#A"/>
        <record name="B" href="#B"/>
      </start>
    </xddl>
Decoding three bytes yields:

    # idm export.xddl 010203
    
    Name  Length  Value  Hex   Description
    A
      b   8       1      #01
    B
      b   16      515    #0203
These properties are also visible to records that are included from different files.  There are no "file local" 
properties.

<h2 id="field">1.7 field</h2>

The [&lt;field&gt;](#field) element identifies an integer unit of information specific to the message being represented.  

It must have a `name` and `length` attribute.  The `length` is specified in bits, and may be any nonnegative integer
value.  It does not have to be byte aligned within the record it appears.

The optional `bias` attribute is added to the value by a fixed amount when displayed in the [idm](#idm).  See the `bias`
example in the description below.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | name | string| &#10004;  | 
 | type | url| | 
 | bias | integer| | 
 | length | expression| &#10004;  | 
 | default | expression| | 


children: [comment](#comment), [item](#item), [range](#range), [script](#script)



This is a simple example that defines a field named "foo" and is 4 bits long.  The `name` and `length` are required
attributes, and typically they are the only ones used.  Here is an example describing a simple message consisting of one
4 bit field.

    <xddl>
      <start>
        <field name="sequence" length="4"/>
      </start>
    </xddl>
Parsing the four bit message "@1111" results in:

    # idm simple_field.xddl @1111
    
    Name     Length  Value  Hex   Description
    sequence 4       15     @1111
<h2 id="bias-Attribute">1.7.1 bias Attribute</h2>


The optional `bias` attribute is used to offset the value of field by a
fixed amount.  Here's an example:

    <xddl>
      <field name="a" length="1" bias="-10"/>
      <field name="b" length="1" bias="-9"/>
      <field name="c" length="1" bias="-8"/>
      <field name="d" length="1" bias="-7"/>
      <field name="e" length="1" bias="1"/>
      <field name="f" length="1" bias="2"/>
      <field name="g" length="1" bias="3"/>
      <field name="h" length="1" bias="4"/>
    </xddl>
Each field is just 1 bit long, but we are biasing them by varying amounts.
The bias is applied after the fields are parsed.  If we parse a message of
all zeroes, here is what we get:

    # idm bias.xddl @00000000
    
    Name  Length  Value  Hex  Description
    a     1       -10    @0
    b     1       -9     @0
    c     1       -8     @0
    d     1       -7     @0
    e     1       1      @0
    f     1       2      @0
    g     1       3      @0
    h     1       4      @0
As you can see, the *Value* column is offset by the `bias`.  The *Hex*
column still reflects the original bit pattern.

<h2 id="type-Attribute">1.7.2 type Attribute</h2>


The optional `type` attribute references a [&lt;type&gt;](#type) element's `id`.  See the [&lt;type&gt;](#type) element
reference for examples.

This example references a locally defined [&lt;type&gt;](#type).

    <xddl>
     <type id="HelloType">
        <item key="0" value="Goodbye World!"/>
        <item key="1" value="Hello World!"/>
     </type>
     <bit name="A" type="#HelloType"/>
     <bit name="B" type="#HelloType"/>
    </xddl>
And decoding the bits `10` yields:

    # idm hello.xddl @10
    
    Name  Length  Value  Hex  Description
    A     1       1      @1   Hello World!
    B     1       0      @0   Goodbye World!
<h2 id="fragment">1.8 fragment</h2>

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | href | url| &#10004;  | 


children: none



The following example parses the same record twice, once as a fragment, and then once as a record.

    <xddl>
      <record id="A">
        <field name="b" length="8"/>
      </record>
      <start>
        <fragment href="#A"/>
        <record name="A" href="#A"/>
      </start>
    </xddl>
The result:

    # idm fragment.xddl 0102
    
    Name  Length  Value  Hex  Description
    b     8       1      #01
    A
      b   8       2      #02
Fragments are useful sometimes when many messages contain the same handfull of fields.

<h2 id="if">1.9 if</h2>


The [&lt;if&gt;](#if) element provides a way to conditionally include other elements based
on an *expression*.


attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | expr | expression| &#10004;  | 


children: [Common Children](#Common-Children)



The following example illustrates the conditional inclususion of a field:

    <xddl>
      <start>
        <field name="Included" length="8"/>
        <if expr="Included">
         <field name="More" length="8"/>
        </if>
      </start>
    </xddl>
Now we parse two messages with the above file.  The first one will
include the `More` field and the second one will not:

    # idm if.xddl 0105 00
    
    Name     Length  Value  Hex  Description
    Included 8       1      #01
    More     8       5      #05
    Name     Length  Value  Hex  Description
    Included 8       0      #00
The `expr` attribute may be any XDDL expression.  As long as it does not
evaluate to zero, the conditional elements will be included.

<h2 id="item">1.10 item</h2>


The [&lt;item&gt;](#item) element only appears as a child of the [&lt;type&gt;](#type) element.  It is
used to specify an item of an enumerated list.



attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | key | integer| &#10004;  | 
 | href | url| | 
 | value | string| &#10004;  | 


children: none


The option *href* attribute can be specified and is used in conjuntion with the [&lt;jump&gt;](#jump) element.

See [&lt;type&gt;](#type) for example usage.
<h2 id="jump">1.11 jump</h2>

A [&lt;jump&gt;](#jump) element provides an easy way to choose a record to parse based on a value.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | base | jump_name| &#10004;  | 


children: none


A common pattern among parsing messages is to choose one of many records to parse based on a single field's value, a
message type, for example.  This can easy enough be done with a [&lt;switch&gt;](#switch) element:

    <uint8 name="msg-id"/>
    <switch expr="msg-id">
        <case value="1">
            <record href="#A"/>
        </case>
        <case value="2">
            <record href="#B"/>
        </case>
        <case value="3">
            <record href="#C"/>
        </case>
          .
          .
          .
    </switch>

Using [&lt;jump&gt;](#jump) along with [&lt;type&gt;](#type) can greatly simplify this trivial case:

    <uint8 name="msg-id" type="#msg-id"/>
    <type id="msg-type">
      <item key="1" value="A" href="#A"/>
      <item key="2" value="B" href="#B"/>
      <item key="3" value="C" href="#C"/>
         .
         .
         .
    </type>
    <jump base="msg-id"/>

The above two listings are functionally equivalent.
<h2 id="oob">1.12 oob</h2>

[&lt;oob&gt;](#oob) is used to indicate out-of-band data.  It is functionally equivalent to [&lt;enc&gt;](#enc).

attributes: none



children: [export](#export), [start](#start), [type](#type), [Common Children](#Common-Children)

<h2 id="pad">1.13 pad</h2>


The [&lt;pad&gt;](#pad) element is used to align a record to a boundary.  Typically, this
will be a byte boundary, but can be changed by using the attributes.

It's length is not determined by a fixed value or expression, rather it is
determined by the current bit number of the message or record it appears in.


attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | mod | pos_integer| | 
 | name | string| | 
 | offset | size| | 


children: none


Without attributes specified, the [&lt;pad&gt;](#pad) element will consume bits of the
record until the record is byte aligned.  For example, the [&lt;pad&gt;](#pad) element in following
document will consume 3 bits in order to make the message byte aligned.

    <xddl>
      <field name="A" length="5"/>
      <pad/>
      <field name="B" length="8"/>
    </xddl>
And parsing:

    # idm pad.xddl A014
    
    Name  Length  Value  Hex    Description
    A     5       20     @10100
    pad   3       0      @000
    B     8       20     #14
As we can see, the length of the pad is 3.

If we change the length of the *A* field to 2, we get a pad of 6.

    <xddl>
      <field name="A" length="2"/>
      <pad/>
      <field name="B" length="8"/>
    </xddl>
    # idm pad1.xddl A014
    
    Name  Length  Value  Hex     Description
    A     2       2      @10
    pad   6       32     @100000
    B     8       20     #14
<h2 id="mod">1.13.1 mod</h2>


The *mod* attribute defaults to 8, but can be modified.  For example,
it may be desireable to pad to the nearest 2-byte boundary, in which case
we would specify a *mod* of 16.  

<h2 id="peek">1.14 peek</h2>

The [&lt;peek&gt;](#peek) element provides access to data ahead in the message.  This 
information can then be referenced in expressions.


attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | name | string| &#10004;  | 
 | length | expression| &#10004;  | 
 | offset | size| &#10004;  | 


children: none


In some protocols a field cannot be decoded correctly until a subsequent
field is known.  The [&lt;peek&gt;](#peek) element provides a solution for this situation.

    <xddl>
      <peek name="pd" offset="4" length="4"/>
      <switch expr="pd">
        <case value="0">
          <field length="4" name="security header"/>
          <field length="4" name="protocol descriminator"/>
        </case>
        <case value="1">
          <field length="4" name="bearer identity"/>
          <field length="4" name="protocol descriminator"/>
        </case>
       </switch>
    </xddl>
The above example illustrates a typical use of the [&lt;peek&gt;](#peek) element. Notice the [&lt;peek&gt;](#peek) "looks ahead" to the "protocol
discriminator" in each of the [&lt;case&gt;](#case) elements to determine what its value should be.  Then the [&lt;switch&gt;](#switch) can be properly
evaluated.

<h2 id="prop">1.15 prop</h2>

The [&lt;prop&gt;](#prop) element declares and initializes a property.  Properties can
be referenced in expressions just like fields.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | name | string| &#10004;  | 
 | type | url| | 
 | value | expression| | 
 | visible | bool| | 


children: [item](#item), [range](#range), [script](#script)


Properties provide a way to create a data member in the current scope.
This property can later be referenced in expressions.  It is similar to a
field, but does not consume data from the message, and it can later be
changed using the [&lt;setprop&gt;](#setprop) element. 

Also similar to fields, a property can reference a [&lt;type&gt;](#type) using the type
attribute.  This too can later be changed with the [&lt;setprop&gt;](#setprop) element.
<h2 id="range">1.16 range</h2>

The [&lt;range&gt;](#range) element is used to specify a range of values for a [&lt;type&gt;](#type).


attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | end | integer| &#10004;  | 
 | href | url| | 
 | value | string| | 
 | start | integer| &#10004;  | 


children: none


[&lt;range&gt;](#range) elements can exist along side [&lt;item&gt;](#item) elements. The [&lt;item&gt;](#item) values are
evaluated first, and the [&lt;range&gt;](#range) second.  This means a [&lt;range&gt;](#range) can overlap
existing items.  Using these two mechanics, we can use a [&lt;range&gt;](#range) as a
default if no items match a particular value.

The following example illustrates this.  The first part of the enumerated type lists
several colors with their RGB Hex Triplet.  The [&lt;range&gt;](#range) at the bottom will
be used if no [&lt;item&gt;](#item) matches.

    <xddl>
     <type id="colors">
        <item key="#F0F8FF" value="Alice blue"/>
        <item key="#E32636" value="Alizarin"/>
        <item key="#E52B50" value="Amaranth"/>
        <item key="#FFBF00" value="Amber"/>
        <item key="#9966CC" value="Amethyst"/>
        <item key="#FBCEB1" value="Apricot"/>
        <item key="#00FFFF" value="Aqua"/>
        <item key="#7FFFD4" value="Aquamarine"/>
        <item key="#4B5320" value="Army green"/>
        <item key="#7BA05B" value="Asparagus"/>
        <item key="#FF9966" value="Atomic tangerine"/>
        <item key="#6D351A" value="Auburn"/>
        <item key="#007FFF" value="Azure (color wheel)"/>
        <item key="#F0FFFF" value="Azure (web)"/>
    
        <range start="0" end="#FFFFFF" value="Unknown Color"/>
     </type>
      <start>
        <field length="24" name="first" type="#colors"/>
        <field length="24" name="second" type="#colors"/>
        <field length="24" name="third" type="#colors"/>
        <field length="24" name="fourth" type="#colors"/>
        <field length="24" name="fifth" type="#colors"/>
        <field length="24" name="sixth" type="#colors"/>
        <field length="24" name="seventh" type="#colors"/>
        <field length="24" name="eighth" type="#colors"/>
        <field length="24" name="ninth" type="#colors"/>
      </start>
    </xddl>
Parsing a message with this file yields:

    # idm range.xddl E3263600FFFF0000FFF0FFFF66FF00ACE1AF4B5320FF9966F19CBB
    
    Name    Length  Value    Hex     Description
    first   24      14886454 #E32636 Alizarin
    second  24      65535    #00FFFF Aqua
    third   24      255      #0000FF Unknown Color
    fourth  24      15794175 #F0FFFF Azure (web)
    fifth   24      6749952  #66FF00 Unknown Color
    sixth   24      11329967 #ACE1AF Unknown Color
    seventh 24      4936480  #4B5320 Army green
    eighth  24      16750950 #FF9966 Atomic tangerine
    ninth   24      15834299 #F19CBB Unknown Color
See the [&lt;type&gt;](#type) element reference for more usage of types.

<h2 id="record">1.17 record</h2>


A [&lt;record&gt;](#record) is a way to group elements together, including other records.  If given an *id*, records can then be
referenced from other places in the document, or from a different document, using URL notation.

Hence, [&lt;record&gt;](#record) can be used in two different ways:
<h2 id="Record-Definition">1.17.1 Record Definition</h2>

Define a [&lt;record&gt;](#record).

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | id | id_url| | 
 | name | string| | 
 | length | expression| | 


children: [Common Children](#Common-Children)



Example:

    <record id="ack">
        <uint8 name="sequence number"/>
        <uint8 name="error"/>
    </record>

<h2 id="Record-Link">1.17.2 Record Link</h2>

Link to a record defined someplace else.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | name | string| | 
 | href | url| | 
 | length | expression| | 


children: none



The record definition in the example above can be referenced with:

    <record href="#ack"/>



Example:

    <record id="ack">
        <uint8 name="sequence number"/>
        <uint8 name="error"/>
    </record>

<h2 id="repeat">1.18 repeat</h2>


The [&lt;repeat&gt;](#repeat) element repeats its child elements a certain number of times, creating a record for each iteration.  
There are three different ways to use [&lt;repeat&gt;](#repeat), based on the attribute signature, described below.
<h2 id="Repeat-Indefinitely">1.18.1 Repeat Indefinitely</h2>


This form will repeat until all the available bits are consumed.  


attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | name | string| | 
 | minlen | size| | 


children: [Common Children](#Common-Children)



A common pattern for this usage is to combine it with a fixed size record, for example:

    <xddl>
      <record length="8">
        <repeat>
          <bit name="a"/>
          <bit name="b"/>
        </repeat>
        <uint8 name="crc"/>
      </record>
    </xddl>
Example decode:

    # idm repeat1.xddl A3FF
    
    Name       Length  Value  Hex  Description
    record
      repeat
        record
          a    1       1      @1
          b    1       0      @0
        record
          a    1       1      @1
          b    1       0      @0
        record
          a    1       0      @0
          b    1       0      @0
        record
          a    1       1      @1
          b    1       1      @1
      crc      0       0
<h2 id="Numbered-Repeat">1.18.2 Numbered Repeat</h2>

This version repeats based an *expression*.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | num | expression| &#10004;  | 
 | name | string| | 


children: [Common Children](#Common-Children)

<h2 id="Bound-Repeat">1.18.3 Bound Repeat</h2>

This version will repeat its contents at least *min* times and no more than *max*.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | min | expression| | 
 | max | expression| | 
 | name | string| | 
 | minlen | integer| | 


children: [Common Children](#Common-Children)

<h2 id="script">1.19 script</h2>

The [&lt;script&gt;](#script) element contains XddlScript.  It appears as a child of the [&lt;type&gt;](#type) element and is used to specify or refine a
field's description.

attributes: none



children: none



The language is [Lua](http://www.lua.org) based.  Documentation on Lua can be found at
[www.lua.org](http://www.lua.org).

<h2 id="The-description-Variable">1.19.1 The description Variable</h2>


The purpose of the [&lt;script&gt;](#script) element is to set a field's (or
property's) description.  This is done by setting a variable named
*description* to a string.  Here's a simple example that uses a [&lt;script&gt;](#script) to
treat a value as an ASCII string.

    <type id="string">
      <script>
        description = string.format("%s", ascii());
      </script>
    </type>

The *ascii()* function is an XddlScript function that interprets the current
value as an ASCII string.  

<h2 id="XddlScript-Functions">1.19.2 XddlScript Functions</h2>


The following table lists all the currently supported XddlScript functions
and is subject to change.  The function availability when used used by [&lt;field&gt;](#field) or [&lt;prop&gt;](#prop) 
elements is also noted.  

Function              | fields | props | Description
----------------------|--------|-------|---------------------------------------------------------
ascii                 | &#10004;    |       | Return the current value as an ASCII string
ascii7                | &#10004;    |       | Return the current value as a 7 bit ASCII string
Description(name)     | &#10004;    | &#10004;   | Return the description of a previous field
EnumValue             | &#10004;    | &#10004;   | Return the &lt;enum&gt; description of the current value if it has one
Value(name)           | &#10004;    | &#10004;   | Return the value of another field
slice(offset, length) | &#10004;    |       | Slice a field into pieces, see description below
TwosComplement        | &#10004;    |       | Return the current value as a two's complement integer
search(name)          | &#10004;    | &#10004;   | Return the description of a node in the message by name


The *ascii()* string does not have to be null terminated.  However, if 
it is null terminated, the characters after the termination character will
be ignored.  Any non-printable characters will be printed as periods.  

The *Description()* function will return the description of a node that is in scope.
The *search()* function will do a depth-first search for a field from the 
top of the message.

The *slice()* function can take the current value and return a value of just a
bit range, a subset of the entire bitstring that makes up the value.  A
good example is taking a 32-bit IP address type and representing it in the
familiar dot notation:

    <xddl>
    <type id="ip_address">
      <script>
        description = string.format("%d.%d.%d.%d", slice(0, 8), slice(8, 8), slice(16, 8), slice(24, 8))
      </script>
    </type>
      <start>
        <uint32 name="address" type="#ip_address"/>
      </start>
    </xddl>
And parsing some data:

    # idm ipscript.xddl AF38B1E6
    
    Name    Length  Value      Hex       Description
    address 32      2939728358 #AF38B1E6 175.56.177.230
<h2 id="setprop">1.20 setprop</h2>

The [&lt;setprop&gt;](#setprop) element provides a way to change the value or type of a property.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | name | setprop_name| &#10004;  | 
 | type | url| | 
 | value | expression| &#10004;  | 


children: [item](#item), [range](#range), [script](#script), [Common Children](#Common-Children)


The *name* is the name of a property that was previously created using the [&lt;prop&gt;](#prop) element.  It must exist and be in
scope.  The *type* will set a new [&lt;type&gt;](#type) reference of the property.  This must be specified even if the type hasn't
changed, otherwise the type will be removed.  The *value* is the new value of the property.
<h2 id="start">1.21 start</h2>

The [&lt;start&gt;](#start) element is optional and specifies the starting record of a document.
If the [&lt;start&gt;](#start) is not specified, then parsing will begin at the beginning of the document. 

attributes: none



children: [Common Children](#Common-Children)


A typical XDDL specification contains many records, one for each message type to be parsed.  It is convenient to 
have an explicit starting point for parsing, and that is what [&lt;start&gt;](#start) is for.  It is analogous to the *main()* function
in C/C++.
<h2 id="switch">1.22 switch</h2>

The [&lt;switch&gt;](#switch) element is similar in function to the *switch* statement in
popular general purpose programming languages.  Based on the evaluation of
the *expr* attribute, a particular [&lt;case&gt;](#case) element's contents will be parsed.


attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | expr | expression| &#10004;  | 


children: [case](#case), [default](#default)


In order for it to be parsed, the [&lt;switch&gt;](#switch) element's *expr* attribute must
evaluate to the [&lt;case&gt;](#case) element's *value* attribute.

The *value* of each [&lt;case&gt;](#case) child must be unique.

There is no need for a corresponding *break*.  Execution will only
"fall-through" if the [&lt;case&gt;](#case) being executed is empty.

If no matches are found, and a [&lt;default&gt;](#default) element exists as a child of the
[&lt;switch&gt;](#switch), then its contents will be parsed.  There can be at most one
[&lt;default&gt;](#default) child.

Otherwise, nothing will be parsed.

The following example illustrates the use of a [&lt;switch&gt;](#switch).  It describes a
message of three octets.  The first octet is used for the *expr* in the [&lt;switch&gt;](#switch)
element.  The second octet is read by the corresponding [&lt;case&gt;](#case) contents, and
the final octet is read into the *check* field.

    <xddl>
      <start>
    	 <field name="choice" length="8"/>
    	 <switch expr="choice">
    	  <case value="1">
    	   <field name="a" length="4"/>
    	   <field name="b" length="4"/>
    	  </case>
    	  <case value="2">
    	   <field name="c" length="1"/>
    	   <field name="d" length="7"/>
    	  </case>
    	  <case value="3"/> <!-- "fall through" -->
    	  <case value="4">
    	   <field name="e" length="2"/>
    	   <field name="f" length="6"/>
    	  </case>
    	  <default>
    	   <field name="g" length="2"/>
    	   <field name="h" length="6"/>
    	  </default>
    	 </switch>
    	 <field name="check" length="8"/>
      </start>
    </xddl>
    
We can parse the file with different messages to see the different paths
are followed:

Here we follow the first case:

    # idm choice.xddl 0104FF
    
    Name   Length  Value  Hex   Description
    choice 8       1      #01
    a      4       0      @0000
    b      4       4      @0100
    check  8       255    #FF
The "fall-through" case:

    # idm choice.xddl 031AFF 041AFF
    
    Name   Length  Value  Hex     Description
    choice 8       3      #03
    e      2       0      @00
    f      6       26     @011010
    check  8       255    #FF
    Name   Length  Value  Hex     Description
    choice 8       4      #04
    e      2       0      @00
    f      6       26     @011010
    check  8       255    #FF
Both of the above messages follow the `value="4"` case.

And finally the [&lt;default&gt;](#default) case can be followed if we specify a *choice* that
does not match any other [&lt;case&gt;](#case):

    # idm choice.xddl AAFEFF
    
    Name   Length  Value  Hex     Description
    choice 8       170    #AA
    g      2       3      @11
    h      6       62     @111110
    check  8       255    #FF
<h2 id="type">1.23 type</h2>

The [&lt;type&gt;](#type) tag is used to specify valid values for [&lt;field&gt;](#field) elements.
It is also used to specify a field's description.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | id | id_url| &#10004;  | 
 | name | string| | 


children: [item](#item), [range](#range), [script](#script)


The [field example](#type-Attribute) above shows a typical usage of [&lt;type&gt;](#type).

<h2 id="Anonymous-Types">1.23.1 Anonymous Types</h2>


Often it is easier to specify a field's valid values by placing them as children of the [&lt;field&gt;](#field).  The following 
example illustrates this.

    <xddl>
     <bit name="A">
        <item key="0" value="Goodbye World!"/>
        <item key="1" value="Hello World!"/>
     </bit>
    </xddl>
And running:

    # idm anon.xddl @1 @0
    
    Name  Length  Value  Hex  Description
    A     1       1      @1   Hello World!
    Name  Length  Value  Hex  Description
    A     1       0      @0   Goodbye World!
Note, since an anonymous type has no *id*, it cannot be referenced from any other field.

<h2 id="uint16">1.24 uint16</h2>

This is equivalent to a [&lt;field&gt;](#field) with length 16.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | name | string| &#10004;  | 
 | type | url| | 
 | bias | integer| | 
 | default | expression| | 


children: [comment](#comment), [item](#item), [range](#range), [script](#script)

<h2 id="uint32">1.25 uint32</h2>

This is equivalent to a [&lt;field&gt;](#field) with length 32.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | name | string| &#10004;  | 
 | type | url| | 
 | bias | integer| | 
 | default | expression| | 


children: [comment](#comment), [item](#item), [range](#range), [script](#script)

<h2 id="uint64">1.26 uint64</h2>

This is equivalent to a [&lt;field&gt;](#field) with length 64.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | name | string| &#10004;  | 
 | type | url| | 
 | bias | integer| | 
 | default | expression| | 


children: [comment](#comment), [item](#item), [range](#range), [script](#script)

<h2 id="uint8">1.27 uint8</h2>

This is equivalent to a [&lt;field&gt;](#field) with length 8.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | name | string| &#10004;  | 
 | type | url| | 
 | bias | integer| | 
 | default | expression| | 


children: [comment](#comment), [item](#item), [range](#range), [script](#script)

<h2 id="while">1.28 while</h2>

Repeat the contents of the [&lt;while&gt;](#while) as long as *expr* is true.

attributes | name  | [type](#AttributeTypes) | required
-----------|-------|-------------------------|---------
 | name | string| | 
 | expr | expression| &#10004;  | 


children: [Common Children](#Common-Children)

<h2 id="xddl">1.29 xddl</h2>

The root element.

attributes: none



children: [export](#export), [start](#start), [type](#type), [Common Children](#Common-Children)

<h2 id="Attribute-Types">2 Attribute Types</h2>


Type | Default | Description
-----|---------|------------
bool | false | *true* or *false*
integer | 0 | Any integer will do
pos_integer | 1 | Positive integer
size | 0 | Non-negative integer
string |  | 
expression |  | XDDL expression
setprop_name |  | Name of a property that is in scope
url |  | Link to a record
id_url |  | id used in record definitions
jump_name |  | Field name used for jump element
<h2 id="Common-Children">3 Common Children</h2>

[bit](#bit), [cstr](#cstr), [enc](#enc), [field](#field), [fragment](#fragment), [if](#if), [jump](#jump), [oob](#oob), [pad](#pad), [peek](#peek), [prop](#prop), [record](#record), [repeat](#repeat), [setprop](#setprop), [switch](#switch), [uint16](#uint16), [uint32](#uint32), [uint64](#uint64), [uint8](#uint8), [while](#while)
