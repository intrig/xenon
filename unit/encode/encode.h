#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "../itunit.h"

namespace IT {
class Encode
{
    public:
    Encode() : icd("icd.xddl") {}
    void register_tests(UnitTest<Encode> & ut) 
    {
        ut.skip();
        ut.cont();
        ut.add(&Encode::enc_field01);
        ut.add(&Encode::enc_if04);
        ut.add(&Encode::enc_if03);
        ut.add(&Encode::enc_repeat01);
        ut.add(&Encode::assign_bs_to_record);
        ut.add(&Encode::enc_repeat02);
        ut.add(&Encode::enc_repeat06);
        ut.add(&Encode::enc_duplicate);
        ut.add(&Encode::enc_cdma2000);
        ut.add(&Encode::enc_lte_errors);
        ut.add(&Encode::enc_lte);
        ut.add(&Encode::enc_float32);
        ut.add(&Encode::enc_int64);
        ut.add(&Encode::enhanced1); 
        ut.add(&Encode::enhanced2); 
        ut.add(&Encode::enhanced3); 
        ut.add(&Encode::sms); 
        ut.add(&Encode::peek_if); 
        ut.add(&Encode::peek_record); 
        ut.add(&Encode::enhanced4); 
        ut.add(&Encode::enhanced5); 
        ut.add(&Encode::enhanced6); 

        ut.skip();
        ut.cont();
    }


    /* Tests */
    void enc_field01();
    void enc_if04();
    void enc_if03();
    void enc_repeat01();
    void assign_bs_to_record();
    void enc_repeat02();
    void enc_repeat06();
    void enc_duplicate();
    void enc_cdma2000();
    void enc_lte_errors();
    void enc_lte();
    void enc_float32();
    void enc_int64();

    void enhanced1();
    void enhanced2();
    void sms();
    void enhanced3();
    void peek_if();
    void peek_record();
    void enhanced4();
    void enhanced5();
    void enhanced6();

    private:
    IT::Spec icd;
};
}
