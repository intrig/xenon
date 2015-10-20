#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license

#include <string.h>
#include <iostream>
#include <ict/bitstring.h>
#include <ict/exception.h>
#include <cstdint>
#include <ict/netvar.h>

namespace ict {
namespace util {
    uint16_t char_to_number(char D) {
        return D == '0' ? 10 : D - '0';
    }

    char number_to_char(uint16_t n) {
        return n == 10 ? '0' : n + '0';
    }

    bitstring encode_three(std::string & source, int start_index) {
        auto D1 = char_to_number(source[start_index++]);
        auto D2 = char_to_number(source[start_index++]);
        auto D3 = char_to_number(source[start_index]);

        uint16_t D = 100 * D1 + 10 * D2 + D3 - 111;

        auto nvar = ict::netvar<uint16_t>(D);

        auto bs = bitstring((const char *) nvar.data.data(), 16);
        bs.remove(0, 6); 

        return bs;
    }

    std::string decode_three(bitstring bs) {
        auto D = to_integer<uint16_t>(bs);
        auto d1 = (D + 111) / 100;
        auto d2 = (D + 110) / 10 % 10;
        auto d3 = (D % 10) + 1;

        std::string value;
        value.reserve(3);
        value += number_to_char(d1);
        value += number_to_char(d2);
        value += number_to_char(d3);
        return value;
    }

    bitstring encode_bcd(char ch) {
        switch (ch) {
            case '1' : return bitstring("@0001");
            case '2' : return bitstring("@0010");
            case '3' : return bitstring("@0011");
            case '4' : return bitstring("@0100");
            case '5' : return bitstring("@0101");
            case '6' : return bitstring("@0110");
            case '7' : return bitstring("@0111");
            case '8' : return bitstring("@1000");
            case '9' : return bitstring("@1001");
            case '0' : return bitstring("@1010");
            default: IT_THROW("encoding invalid bcd character : " << ch);
        }
    }

    char decode_bcd(bitstring bs) {
        switch (to_integer<char>(bs)) {
            case 1 : return '1';
            case 2 : return '2';
            case 3 : return '3';
            case 4 : return '4';
            case 5 : return '5';
            case 6 : return '6';
            case 7 : return '7';
            case 8 : return '8';
            case 9 : return '9';
            case 10 : return '0';
            default: IT_THROW("decoding invalid bcd character : " << to_string(bs));
        }
    }

} // namesapce ict::util

std::string decode_imsi(bitstring const & imsi_s) {
    auto first_three = util::decode_three(imsi_s.substr(0, 10));
    auto second_three = util::decode_three(imsi_s.substr(10, 10));
    auto thousands = util::decode_bcd(imsi_s.substr(20, 4));
    auto last_three = util::decode_three(imsi_s.substr(24, 10));

    std::ostringstream os;
    os << first_three << second_three << thousands << last_three;
    return os.str();
}

bitstring encode_imsi(std::string & number) {
    auto first_three = util::encode_three(number, 0);
    auto second_three = util::encode_three(number, 3);
    auto thousands = util::encode_bcd(number[6]);
    auto last_three = util::encode_three(number, 7);

    obitstream os;
    os << first_three << second_three << thousands << last_three;
    return os.bits();
}

} // namespace ict
