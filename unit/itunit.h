#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license

#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>

namespace IT {
enum Platform {

    MSVC = 0x000000001,  
    Apple = 0x000000002, 
    Linux = 0x000000004,
    Unknown = 0x000000008
};

inline Platform hostPlatform()
{
#if defined(_MSC_VER) 
    return MSVC;
#endif
#if defined(__APPLE__)
    return Apple;
#endif
#if defined(__linux__)
    return Linux;
#endif
    return Unknown;
}
}

#ifdef IT_ASSERT
#undef IT_ASSERT
#endif
#define IT_ASSERT(condition) \
    do { \
    if (!(condition)) throw IT::UnitError(#condition, __FILE__, \
        __LINE__); \
    } while (0)

#ifdef IT_ASSERT_MSG
#undef IT_ASSERT_MSG
#endif
#define IT_ASSERT_MSG(desc, condition) \
    do { \
        if (!(condition)) { \
            std::ostringstream os; \
            os << desc << ": " << #condition; \
            throw IT::UnitError(os.str(), __FILE__, __LINE__); \
        } \
    } while (0)

#define IT_FORCE_ASSERT(desc) \
    do { \
        std::ostringstream os;  \
        os << desc; \
        throw IT::UnitError(os.str(), __FILE__, __LINE__); \
    } while (0)

#define IT_WARN_MSG(desc, condition) \
    do { \
        if (!(condition)) { \
            std::ostringstream os; \
            os << desc << ": " << #condition; \
            throw UnitError(os.str(), __FILE__, __LINE__, true); \
        } \
    } while (0)

#include <IT/intrig.h>
#include <IT/Exception.h>

namespace IT {

class UnitError : public std::logic_error
{
    public :

    UnitError(const std::string & what, const char *file, int line,
        bool warn = false) : std::logic_error(what), srcFile(file), srcLine(line), warn(warn) {}
    virtual ~UnitError() throw() { }
    std::string srcFile;
    int srcLine;
    bool warn;
};

template <typename T> 
class UnitTest {
    public:
        typedef void (T::* Pointer)();

        struct Test {
            Test(Pointer p, unsigned int mask = 0, bool iso = false) : 
                p(p), mask(mask), iso(iso) {}
            Pointer p;
            unsigned int mask;
            bool iso;
        };

        UnitTest (T * test) : _test(test), _mask(0), _iso(false) {
            tests.reserve(100);
            _test->register_tests(*this);
        }
        void skip(unsigned int mask = 0xFFFFFFFF) { _mask = mask; }
        void skip(Pointer p) {
            Test test(p, 0xFFFFFFFF);
            tests.push_back(test);
        }
        void cont() { _mask = 0; }
        void add(Pointer p) {
            Test test(p, _mask);
            tests.push_back(test);
        }
        void iso(Pointer p) {
            Test test(p, 0, true);
            tests.push_back(test);
            _iso = true;
        }
        int run() {
            ict::timer tmr;
            int skipped = 0;
            typename std::vector<Test>::iterator it;
            tmr.start();
            for (it = tests.begin(); it != tests.end(); ++it) {
                Pointer p = it->p;
                try {
                    if ((_iso && !it->iso) || (IT::hostPlatform() & it->mask)) {
                        std::cerr << "S";
                        skipped++;
                    } else {
                        std::cerr << ".";
                        (_test->*p)();

                    }
                } catch (UnitError & e) {
                    std::cerr << std::endl << e.srcFile << ":" << 
                        e.srcLine << ": ";
                    if (e.warn) std::cerr << "warning: ";
                    else std::cerr << "error: ";
                    std::cerr << e.what() << std::endl;

                    if (!e.warn) return 1;
                } 
                catch (IT::Exception & e) {
                    std::cerr << "\n The unit test framework caught an unexpected exception: " << e.what() << '\n';
                    return 1;
                }
            }
            tmr.stop();
            std::cerr << "\ntotal time: " << ict::to_string(tmr);

            if (skipped) {
                std::cerr << std::endl << "warning: " << skipped << 
                    " test" << ((skipped > 1) ? "s " : " ") << "skipped";
            }
            std::cerr << std::endl;
            return 0;
        }

    private:
        std::vector<Test> tests;
        T * _test;
        unsigned int _mask;
        bool _iso;
};

}
