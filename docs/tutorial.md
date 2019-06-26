#  Xenon Decoder Engine Tutorial
<h2 id="Tutorial">1 Tutorial</h2>



The xenon decoder engine is a convenient way to decode and analyze messages in your programs.  It is used by the online
message decoder at <http://intrig.com>.  Here is an [example SMS message](http://intrig.com/x82da86).  Scroll down to
the bottom to see what the SMS message is.

Let's write a program using xenon to print the same message to `std::cout`.

```c++
#include <xenon/xenon.h>

int main () {
    try {
        xenon::spec_server s;
        auto rec = xenon::get_record(s, "3GPP/TS-36.331/DL-DCCH-Message");
        auto m = xenon::parse(rec,
            "0C01513C9FB9C248283B11084808F0080824810A1FA800A8202C090A1FA800010C0098090808C82E4194DFE830");

        // now find the SM field
        auto c = xenon::find_first(m, "TP-User-Data/SM");
        if (c == m.end()) throw std::runtime_error("Cannot find message!\n");

        std::cout << "Message is " << c->description() << '\n';
    } catch (std::exception & e) {
        std::cerr << "e.what()";
    }
}
```

<h2 id="Reference">2 Reference</h2>
<h2 id="Types">2.1 Types</h2>
<h2 id="node">2.1.1 node</h2>

   
The `xenon::node` represents a node in a [message](#message).  Users do not have to create them.
A node is assigned a `node_type` during runtime as the message is being parsed.  

node_type type          |  mnemonic() |  Description
------------------------|-------------|------------------------------------------------
nil_node                |  "EMP"      |  The node exists but is undefined ?
root_node               |  "ROT"      |  This is the root of the message ?
extra_node              |  "EXT"      |  Data that is unparsed is placed in this node
field_node              |  "FLD"      |  A field of a message that has a length and contains data
float_node              |  "FLT"      |  A float field
incomplete_node         |  "INC"      |  A field that doesn't have enough data to parse fully
message_node            |  "MSG"      |  The message root ?
record_node             |  "REC"      |  A <record> node
repeat_node             |  "REP"      |  A <repeat> node
repeat_record_node      |  "RPR"      |  A repeat record node?
prop_node               |  "PRP"      |  A <prop> node
setprop_node            |  "SET"      |  A <setprop> node
peek_node               |  "PEK"      |  A <peek> node
error_node              |  "ERR"      |  An error node ?
<h2 id="message">2.1.2 message</h2>
<h2 id="spec_server">2.1.3 spec_server</h2>
<h2 id="Functions">2.2 Functions</h2>
