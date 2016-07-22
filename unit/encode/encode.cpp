//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <IT/Xenon.h>
#include "encode.h"

using namespace IT;
using namespace std;

void Encode::enc_field01()
{
    Spec doc("xddlunit/field01.xddl");
    IT_ASSERT(!doc.empty());
    Message msg(doc);
    msg.reserve();
    //IT_WARN(msg.text());
    msg.trim();
    IT_ASSERT_MSG(msg.length() << endl << msg.xml(), msg.length() == 1);
    IT_ASSERT(msg["foo"] == 0);
    msg["foo"] = 1;
    IT_ASSERT(msg["foo"] == 1);
}

void Encode::enc_if04()
{
    streambuf* old = cerr.rdbuf();
#if 1
    ostringstream out;
    cerr.rdbuf(out.rdbuf());
#endif

    Spec doc("xddlunit/if04.xddl");
    Message xi(doc);
    xi.reserve();
    IT_ASSERT(xi["Included"] == 0);
    IT_ASSERT(xi["Audio"].empty());
#if 1
    IT_ASSERT(xi["Mix"] == 0);

    xi["Included"] = 1;
    IT_ASSERT(xi["Included"] == 1);
    IT_ASSERT(xi["Audio"] == 0);
    IT_ASSERT(xi["Mix"] == 0);
#endif
#if 1
    cerr.rdbuf(old);
#endif
}

void Encode::enc_if03()
{
    streambuf* old = cerr.rdbuf();
    ostringstream out;
    cerr.rdbuf(out.rdbuf());
    {
        Spec doc("xddlunit/if03.xddl");
        IT_ASSERT(!doc.empty());

        Message xi(doc);
        xi.reserve();


        IT_ASSERT(xi["expr"] == 0);
        IT_ASSERT(xi["M"] == 0);
        IT_ASSERT(xi["A"] == 0);
        IT_ASSERT(xi["B"] == 0);
        IT_ASSERT(xi["is"] == 0);
        IT_ASSERT(xi["OK"] == 0);

        IT_ASSERT(!xi["expr"].empty());
        IT_ASSERT(xi["Mark"].empty());
        IT_ASSERT(xi["Beckwith"].empty());
        xi["expr"] = 1;

        IT_ASSERT(!xi["Mark"].empty());
        IT_ASSERT(!xi["Beckwith"].empty());

        xi["A"] = 1;
        IT_ASSERT(xi["A"].empty());

        xi["B"] = 2;
        IT_ASSERT(xi["B"].empty());

        xi["is"] = 1;
        xi["OK"] = 1;

        IT_ASSERT(xi.length() != 8);
        xi.trim();
        IT_ASSERT(xi.length() == 8);
    }
    {
        Spec doc("xddlunit/if03.xddl");
        IT_ASSERT(!doc.empty());

        Message xi(doc);
        xi.reserve();

        xi["expr"] = 0;
        xi["Mark"] = 1;
        IT_ASSERT(xi["Mark"].empty());

        xi["Beckwith"] = 2;
        IT_ASSERT(xi["Beckwith"].empty());

        xi["A"] = 1;
        xi["B"] = 2;

        xi["is"] = 1;
        xi["OK"] = 1;

        IT_ASSERT(xi.length() != 8);
        xi.trim();
        IT_ASSERT(xi.length() == 8);
    } 
    cerr.rdbuf(old);
}

void Encode::enc_repeat01()
{
    Spec doc("xddlunit/repeat01.xddl");
    IT_ASSERT(!doc.empty());
    Message xi(doc);
    xi.reserve(); // reserve 1024 bits, adjustable
    xi["header"] = 0xFF;

    xi["repeat"][0]["neighbor"] = 0;

    xi["repeat"][1]["neighbor"] = 1;
    xi.trim(); // remove extra, unused bits
    IT_ASSERT(xi.length() == 24);
    IT_ASSERT(xi["header"] == 0xFF);
    IT_ASSERT(xi["repeat"][0]["neighbor"] == 0);
    IT_ASSERT(xi["repeat"][1]["neighbor"] == 1);
}

void Encode::assign_bs_to_record()
{
    {
        Spec doc("xddlunit/repeat01.xddl");
        IT_ASSERT(!doc.empty());
        Message xi(doc);
        xi.reserve(); // reserve 1024 bits, adjustable
        xi["header"] = 0xFF;
        xi["repeat"][0]["neighbor"] = BitString("00");
        xi["repeat"][1]["neighbor"] = BitString("01");
        xi.trim(); // remove extra, unused bits
        IT_ASSERT(xi.length() == 24);
        IT_ASSERT(xi["header"] == 0xFF);
        IT_ASSERT(xi["repeat"][0]["neighbor"] == 0);
        IT_ASSERT(xi["repeat"][1]["neighbor"] == 1);
    } 
    {
        Spec doc("xddlunit/repeat01.xddl");
        IT_ASSERT(!doc.empty());
        Message xi(doc);
        xi.reserve(); // reserve 1024 bits, adjustable
        xi["header"] = 0xFF;
        xi["repeat"] = BitString("0001");
        xi.trim(); // remove extra, unused bits
        IT_ASSERT(xi.length() == 24);
        IT_ASSERT(xi["header"] == 0xFF);
        IT_ASSERT(xi["repeat"][0]["neighbor"] == 0);
        IT_ASSERT(xi["repeat"][1]["neighbor"] == 1);
    } 
}


void Encode::enc_repeat02()
{
    Spec doc("xddlunit/repeat02.xddl");
    IT_ASSERT(!doc.empty());
    Message xi;
    xi.spec(doc);
    xi.reserve();
    xi["length"] = 4;
    xi["repeat"][0]["a"] = 1;
    xi["repeat"][0]["b"] = 2;
    xi["repeat"][1]["a"] = 3;
    xi["repeat"][1]["b"] = 4;
    xi["repeat"][2]["a"] = 5;
    xi["repeat"][2]["b"] = 6;
    xi["repeat"][3]["a"] = 7;
    xi["repeat"][3]["b"] = 8;

    IT_ASSERT(xi["repeat"][0]["a"] == 1);
    IT_ASSERT(xi["repeat"][0]["b"] == 2);
    IT_ASSERT(xi["repeat"][1]["a"] == 3);
    IT_ASSERT(xi["repeat"][1]["b"] == 4);
    IT_ASSERT(xi["repeat"][2]["a"] == 5);
    IT_ASSERT(xi["repeat"][2]["b"] == 6);
    IT_ASSERT(xi["repeat"][3]["a"] == 7);
    IT_ASSERT(xi["repeat"][3]["b"] == 8);

    xi.trim();
    IT_ASSERT_MSG(xi.length(), xi.length() == (8 + 4 * 8));
}

void Encode::enc_repeat06()
{
    Spec doc("xddlunit/repeat06.xddl");
    IT_ASSERT(!doc.empty());
    Message msg(doc);
    msg.reserve(48);

    msg["rep"][0]["type"] = 1;
    msg["rep"][0]["len"] = 1;
    msg["rep"][0]["char"] = 1;
    msg["rep"][1]["type"] = 1;
    msg["rep"][1]["len"] = 1;
    msg["rep"][1]["char"] = 2;

    msg.trim();
}

void Encode::enc_duplicate()
{
    Spec doc("xddlunit/duplicate.xddl");
    IT_ASSERT(!doc.empty());
    Message xi(doc);
    xi.reserve();

    xi["r1"]["a"] = 1;
    xi["r1"]["b"] = 2;

    xi["r2"]["a"] = 8;
    xi["r2"]["b"] = 2;
}


void Encode::enc_cdma2000()
{
    Message xi;
    xi.spec(icd);
    xi.reserve();

    // Feature Notification Message on f-csch
    xi["Technology"] = 1;
    IT_ASSERT(xi["Technology"].element()->hasDependent());
    //IT_WARN(xi.xml());
    xi["Sublayer"] = 1;
#if 1
    xi["Channel"] = 1;
    IT_ASSERT(xi["Channel"].element());
    IT_ASSERT(xi["Channel"].element()->hasDependent());

    IT_ASSERT(xi["MSG_LENGTH"].element()->hasDependent());
    xi["MSG_LENGTH"] = 0x2F;
    //cout << xi.bitString().toHexString() <<  '\n';

    IT_ASSERT_MSG(xi["MSG_LENGTH"], xi["MSG_LENGTH"] == 0x2F);

    Node data = xi["DATA"];
    data["MSG_TYPE"] = 0x0C; // Feature Notification (FNM)
    IT_ASSERT(data["MSG_TYPE"].element()->hasDependent());

    IT_ASSERT(xi["DATA"] == data);

    data["ACK_SEQ"] = 0x7;
    data["MSG_SEQ"] =          0x3; 
    data["ACK_REQ"] =          0x0; 
    data["VALID_ACK"] =        0x0; 
    data["ADDR_TYPE"] =        0x2; // IMSI
    data["ADDR_LEN"] =         0x5; 
    data["IMSI_CLASS"] =       0x0; 
    data["IMSI_class_specific_subfields"] =
        BitString("@000001111100111111110011110001111100110");
    data["RELEASE"] = 0;

    Node rec0 = data["repeat"][0];
    rec0["RECORD_TYPE"] = 0x01;
    rec0["RECORD_LEN"] = 0x05;
    Node char_list0 = rec0["INFO_REC_TYPE_SPEC_FLDS"]["repeat"];

    char_list0[0] = 0x41;
    char_list0[1] = 0x42;
    char_list0[2] = 0x43;
    char_list0[3] = 0x44;
    char_list0[4] = 0x45;

    Node rec2 = data["repeat"][1];
    rec2["RECORD_TYPE"] = 0x02;
    IT_ASSERT(rec2["RECORD_TYPE"].element()->hasDependent());
    rec2["RECORD_LEN"] = 0x0B;
    rec2["INFO_REC_TYPE_SPEC_FLDS"]["NUMBER_TYPE"] = 0x00;
    rec2["INFO_REC_TYPE_SPEC_FLDS"]["NUMBER_PLAN"] = 0x00;

    Node char_list = rec2["INFO_REC_TYPE_SPEC_FLDS"]["repeat"];

    char_list[0] = 0x00;
    char_list[1] = 0x01;
    char_list[2] = 0x02;
    char_list[3] = 0x03;
    char_list[4] = 0x04;
    char_list[5] = 0x05;
    char_list[6] = 0x06;
    char_list[7] = 0x07;
    char_list[8] = 0x08;
    char_list[9] = 0x09;

    Node rec3 = data["repeat"][2];
    rec3["RECORD_LEN"] = 0x0C;
    rec3["RECORD_TYPE"] = 0x03;
    rec3["INFO_REC_TYPE_SPEC_FLDS"]["NUMBER_TYPE"] = 0x00;
    rec3["INFO_REC_TYPE_SPEC_FLDS"]["NUMBER_PLAN"] = 0x00;
    rec3["INFO_REC_TYPE_SPEC_FLDS"]["PI"] = 0x00;
    rec3["INFO_REC_TYPE_SPEC_FLDS"]["SI"] = 0x00;
    Node char_list2 = rec3["INFO_REC_TYPE_SPEC_FLDS"]["repeat"];
    char_list2[0] = 0x00;
    char_list2[1] = 0x01;
    char_list2[2] = 0x02;
    char_list2[3] = 0x03;
    char_list2[4] = 0x04;
    char_list2[5] = 0x05;
    char_list2[6] = 0x06;
    char_list2[7] = 0x07;
    char_list2[8] = 0x08;
    char_list2[9] = 0x09;
    xi["CRC"] = 0x1C7C1038;
    xi.trim();

    IT_ASSERT_MSG(xi.bitString().toHexString(), xi.bitString().toHexString() ==
    "0101012F0CEC4A07CFF3C7CC01054142434445020B0000020406080A0C0E1012030C00000020406080A0C0E101201C7C1038");
#endif
}

void Encode::enc_lte_errors()
{
    Message msg(icd);
    msg.reserve(1024 * 4);
    streambuf* old = cerr.rdbuf();
    ostringstream out;
    cerr.rdbuf(out.rdbuf());

    msg = BitString("03000220002C0E82E2103222444286CB0E120581C0000A0403A023A2");
    
    // misspell first field
    Node n = msg.find("c/rrcConnectionSetupComplete/criticalExtensions/c1/rrcConnectionSetupComplete_r8/dedicatedInfoNASLen");
    IT_ASSERT(n.empty());
    n = 24;
    IT_ASSERT(n.empty());

    // misspell second field
    msg["c1"]
        ["rrcConnexionSetupComplete"]
         ["criticalExtensions"]
          ["c1"]
           ["rrcConnectionSetupComplete_r8"]
            ["dedicatedInfoNASLen"] = 24;

    // misspell last field
    msg["c1"]
        ["rrcConnectionSetupComplete"]
         ["criticalExtensions"]
          ["c1"]
           ["rrcConnectionSetupComplete_r8"]
            ["dedicatedInfoNAZLen"] = 24;

    // access a child that isn't there
    msg["c1"]
        ["rrcConnectionSetupComplete"]
         ["criticalExtensions"]
          ["c1"]
           ["rrcConnectionSetupComplete_r8"]
            ["dedicatedInfoNASLen"]["notThere"]= 24;

    cerr.rdbuf(old);
}

void Encode::enc_lte()
{
    Message msg(icd);
    msg.reserve(1024 * 8);

    BitString bs("074300035200C2");

    msg = BitString("03000220002C0E82E2103222444286CB0E120581C0000A0403A023A2");
    msg["c1"]
        ["rrcConnectionSetupComplete"]
         ["criticalExtensions"]
          ["c1"]
           ["rrcConnectionSetupComplete-r8"]
            ["length"] = bs.length() / 8;
    msg["c1"]
        ["rrcConnectionSetupComplete"]
         ["criticalExtensions"]
          ["c1"]
           ["rrcConnectionSetupComplete-r8"]
            ["dedicatedInfoNAS"] = bs;

}

int main (int , char **)
{
    Encode test;
    UnitTest<Encode> ut(&test);
    return ut.run();
}
