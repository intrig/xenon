//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <IT/Xenon.h>
#include <IT/XddlCode.h>
#include <IT/MessagePrivate.h>
#include "encode.h"

using namespace IT;
using namespace std;

void Encode::enhanced1()
{
    string xddl = 
        "<xddl>"
          "<start>"
            "<field name=''A'' length=''8''/>"
            "<field name=''B'' length=''8''/>"
            "<field name=''C'' length=''8''/>"
            "<field name=''D'' length=''8''/>"
            "<field name=''E'' length=''8''/>"
          "</start>"
        "</xddl>";

    Spec spec;
    spec.parseString(xddl.c_str());
    IT_ASSERT(!spec.empty());
    Message msg(spec);
    msg.reserve(1024);

    msg["A"] = 0x01;
    msg["B"] = 0x02;
    msg["C"] = 0x03;
    msg["D"] = 0x04;
    msg["E"] = 0x05;

    msg.trim();

    //cout << '\n' << msg.text();

    IT_ASSERT_MSG(msg.xml(), msg["A"] == 0x01);
    IT_ASSERT_MSG(msg.xml(), msg["B"] == 0x02);
    IT_ASSERT_MSG(msg.xml(), msg["C"] == 0x03);
    IT_ASSERT_MSG(msg.xml(), msg["D"] == 0x04);
    IT_ASSERT_MSG(msg.xml(), msg["E"] == 0x05);
}

void Encode::enhanced2()
{
    string xddl = 
        "<xddl>"
          "<start>"
            "<field name=''A'' length=''8''/>"
            "<if expr=''A''>"
                "<field name=''a'' length=''8''/>"
            "</if>"
            "<field name=''B'' length=''8''/>"
            "<field name=''C'' length=''8''/>"
            "<field name=''D'' length=''8''/>"
            "<field name=''E'' length=''8''/>"
          "</start>"
        "</xddl>";

    Spec spec;
    spec.parseString(xddl.c_str());
    IT_ASSERT(!spec.empty());
    Message msg(spec);
    msg.reserve(8);


    msg["A"] = 0x00;
    msg["B"] = 0x02;
    msg["C"] = 0x03;
    msg["D"] = 0x04;
    msg["E"] = 0x05;

    msg.xml();
    return;
    IT_ASSERT_MSG(msg.xml(), msg["A"] == 0x00);
    IT_ASSERT_MSG(msg["B"], msg["B"] == 0x02);
    IT_ASSERT_MSG(msg.xml(), msg["C"] == 0x03);
    IT_ASSERT_MSG(msg.xml(), msg["D"] == 0x04);
    IT_ASSERT_MSG(msg.xml(), msg["E"] == 0x05);

    msg["A"] = 0x01;

    IT_ASSERT_MSG(msg.xml(), msg["A"] == 0x01);
    IT_ASSERT_MSG(msg.xml(), msg["a"] == 0x00);
    IT_ASSERT_MSG(msg.xml(), msg["B"] == 0x02);
    IT_ASSERT_MSG(msg.xml(), msg["C"] == 0x03);
    IT_ASSERT_MSG(msg.xml(), msg["D"] == 0x04);
    IT_ASSERT_MSG(msg.xml(), msg["E"] == 0x05);

    msg.trim();
}

void Encode::sms()
{
    Message sms(icd);
    BitString bs= "0101014309334A07CFF3D5720218099000001080101020082428003008E04110001880000008D88720A0E9979F4410E24D82C983060829CDA6836E5E7CF0E7CAB8003A7FE5BA";

    sms = bs;
    IT_ASSERT(sms.bitString() == bs);
    Node pad = sms.find("PDU_PADDING");
    IT_ASSERT(!pad.empty());
    IT_ASSERT_MSG("a", pad == 0);
    Node n = sms.find("NUM_FIELDS");
    IT_ASSERT_MSG("n = " << n, n == 50);
    n = 60;
    IT_ASSERT(n == 60);
    //IT_WARN(sms.bitString().toHexString());
    pad = sms.find("PDU_PADDING");
    IT_ASSERT(!pad.empty());
    IT_ASSERT(pad == 0);

    IT_ASSERT(sms.bitString() != bs);
    n = sms.find("NUM_FIELDS");
    n = 50;
    sms.trim();
    IT_ASSERT_MSG(bs.length() << ", " << sms.bitString().length(), bs.length() == sms.bitString().length());

    if (bs != sms.bitString())
    {
        string s1 = bs.toHexString().c_str();
        string s2 = sms.bitString().toHexString().c_str();

        for (unsigned i = 0; i < s1.size(); ++i)
        {
            IT_ASSERT_MSG(s1[i] << "!= " << s2[i] << " at index " << i << "\n" << s2 << "\n", s1[i] == s2[i]);
        }
    }

    IT_ASSERT_MSG('\n' << bs.toHexString() << "!=\n" << sms.bitString().toHexString(),
    sms.bitString() == bs);
#if 0
#endif
}

void Encode::enhanced3()
{
    Message m(icd); 
    m = BitString("0101016017A97F063541C88877265445");
    m["MSG_LENGTH"] = 98;

    IT_ASSERT(m["MSG_LENGTH"] == 98);
    IT_ASSERT(m["MSG_LENGTH"] == 98);
    IT_ASSERT(m.find("/MSG_LENGTH") == 98);
    IT_ASSERT(m.findAll("/MSG_LENGTH").at(0) == 98);
}

void Encode::peek_if()
{
    string xddl = 
        "<xddl>"
          "<peek name=''iei'' length=''8'' offset=''0''/>"
          "<if expr=''iei == #01''>"
            "<field name=''A'' length=''8''/>"
          "</if>"
          "<if expr=''iei == #02''>"
            "<field name=''B'' length=''8'' default=''2''/>"
          "</if>"
        "</xddl>";

    Spec spec;
    spec.parseString(xddl.c_str());

    Message m(spec);

    m = BitString("01");

    IT_ASSERT_MSG(m.xml(), m["A"] == 1);

    m["A"] = 2;
    IT_ASSERT_MSG(m.xml(), !m["B"].empty());
    IT_ASSERT_MSG(m.xml(), m["B"] == 2);
}

void Encode::peek_record()
{
    string xddl = 
        "<xddl>"
          "<peek name=''iei'' length=''8'' offset=''0''/>"
          "<if expr=''iei == #01''>"
            "<record name=''A''>"
              "<field name=''IEI'' length=''8''/>"
            "</record>"
          "</if>"
          "<if expr=''iei == #02''>"
            "<record name=''B''>"
              "<field name=''IEI'' length=''8'' default=''#02''/>"
            "</record>"
          "</if>"
        "</xddl>";

    Spec spec;
    spec.parseString(xddl.c_str());

    Message m(spec);

    m = BitString("01");

    IT_ASSERT_MSG(m.xml(), m["A"]["IEI"] == 1);
    IT_ASSERT_MSG(m.xml(), m["A/IEI"] == 1);

    m["A"]["IEI"] = 2;
    IT_ASSERT_MSG(m.xml(), !m["B"].empty());
    IT_ASSERT_MSG(m.xml(), m["B"]["IEI"] == 2);
    IT_ASSERT_MSG(m.xml(), m["B/IEI"] == 2);
}


void Encode::enhanced4()
{
    Message m(icd); 
    m = BitString("01020012000000045003020100");
    m["Length"] = 5;
    IT_ASSERT(m["Length"] == 5);
}

void Encode::enhanced5()
{
    string xddl = 
        "<xddl>"
          "<field name=''Length'' length=''32''/>"
          "<record name=''Data'' length=''Length * 8''>"
              "<repeat name=''Attributes''>"
                "<field name=''ID'' length=''16''/>"
                "<field name=''Value'' length=''16''/>"
              "</repeat>"
          "</record>"
        "</xddl>";

    Spec spec;
    spec.parseString(xddl.c_str());
    Message m(spec); 
    m = BitString("000000040004000500");
    m["Length"] = 6;
    IT_ASSERT(m["Length"] == 6);
}


void Encode::enhanced6()
{
    Spec bug("bug-9.xddl");
    Message m(bug);
    m = BitString("020201");

    m["Length1"] = 3;
    IT_ASSERT(m["Length1"] == 3);
}
