#include <xenon/xenon.h>
// Decoding this message:
// http://intrig.com/decode/3GPP/TS-25.331/SysInfoType19/61BA04C697D176821A7A9F4EA20663F3
// and searching for earfcn fields.
int main() {
    try {
        xenon::spec_server s("~/wythe/xenon/xddl");
        auto rec = xenon::get_record(s, "3GPP/TS-25.331/SysInfoType19");
        auto m = xenon::parse(rec, "61BA04C697D176821A7A9F4EA20663F3");

        xenon::for_each_path(
            m, "eutra-FrequencyAndPriorityInfoList/repeat/record/earfcn",
            [&](xenon::message::cursor c) {
                std::cout << "found " << c->name() << " field with value "
                          << c->value() << "\n";
            });

    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}
