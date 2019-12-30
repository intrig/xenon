#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
//
// This code was ported form the mono project to C++ by Mark Beckwith
// (mark@intrig.com).
//
// Copyright (C) 2010, 2011 Intrig (http://intrig.com)
//
// Original authors:
//   Marcel Narings (marcel@narings.nl)
//   Martin Baulig (martin@gnome.org)
//   Atsushi Enomoto (atsushi@ximian.com)
//
//   (C) 2001 Marcel Narings
// Copyright (C) 2004-2006 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
#include <ctime>
#include <iomanip>
#include <stdexcept>

#include "ict/ict.h"

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

namespace xenon {
class DateTime {
    enum DT { DTDay, DTDayYear, DTMonth, DTYear };

    static const long long MAX_VALUE_TICKS = 3155378975999999999L;
    static const long long TicksMask = 0x3fffffffffffffff;
    static const long long TicksPerDay = 864000000000L;
    static const long long TicksPerHour = 36000000000L;
    static const long long TicksPerMillisecond = 10000L;
    static const long long TicksPerMinute = 600000000L;
    static const long long TicksPerSecond = 10000000L;
    static const int dp400 = 146097;
    static const int dp100 = 36524;
    static const int dp4 = 1461;
    static const long long w32file_epoch = 504911232000000000L;

  public:
    DateTime() : _reverse(false) {}
    DateTime(long long ticks) : _reverse(false) {
        if (ticks < 0 || ticks > MAX_VALUE_TICKS) {
            throw std::out_of_range("invalid tick count");
        }
        encoded = ticks;
    }

    bool operator==(DateTime const &dt) const { return encoded == dt.encoded; }

    bool operator!=(DateTime const &dt) const { return encoded != dt.encoded; }

    bool operator==(long long ticks) const { return encoded == ticks; }

    bool operator!=(long long ticks) const { return encoded != ticks; }

    static DateTime Now() {
#if defined(_WIN32)
        SYSTEMTIME systemTime;
        GetLocalTime(&systemTime);
        FILETIME fileTime;
        SystemTimeToFileTime(&systemTime, &fileTime);
        ULARGE_INTEGER uli;
        uli.LowPart = fileTime.dwLowDateTime;
        uli.HighPart = fileTime.dwHighDateTime;

        return DateTime(uli.QuadPart + w32file_epoch);
#else
        return DateTime();
#endif
    }

    int Month() const { return FromTicks(DTMonth); }

    int Day() const { return FromTicks(DTDay); }

    int Year() const { return FromTicks(DTYear); }
    int Hour() const {
        return static_cast<int>((encoded & TicksMask) % TicksPerDay / TicksPerHour);
    }

    int Minute() const {
        return static_cast<int>((encoded & TicksMask) % TicksPerHour / TicksPerMinute);
    }

    int Second() const {
        return static_cast<int>((encoded & TicksMask) % TicksPerMinute / TicksPerSecond);
    }

    int Millisecond() const {
        return static_cast<int>((encoded & TicksMask) % TicksPerSecond /
                     TicksPerMillisecond);
    }

    // The value of this property represents the number of
    // 100-nanosecond intervals that have elapsed since 12:00:00
    // midnight, January 1, 0001.
    //
    long long Ticks() const { return encoded & TicksMask; }

    void ReverseDateTime(bool reverse) { _reverse = reverse; }
    bool ReverseDateTime() const { return _reverse; }

    static const int daysmonth[13];
    static int const daysmonthleap[13];

    int FromTicks(DT what) const {
        int num400, num100, num4, numyears;
        int M = 1;

        // int days[12]  = daysmonth;
        int totaldays = static_cast<int>((encoded & TicksMask) / TicksPerDay);

        num400 = (totaldays / dp400);
        totaldays -= num400 * dp400;

        num100 = (totaldays / dp100);
        if (num100 == 4) // leap
            num100 = 3;
        totaldays -= (num100 * dp100);

        num4 = totaldays / dp4;
        totaldays -= (num4 * dp4);

        numyears = totaldays / 365;

        if (numyears == 4) // leap
            numyears = 3;
        if (what == DTYear)
            return num400 * 400 + num100 * 100 + num4 * 4 + numyears + 1;

        totaldays -= (numyears * 365);
        if (what == DTDayYear)
            return totaldays + 1;

        if ((numyears == 3) &&
            ((num100 == 3) || !(num4 == 24))) // 31 dec leapyear
        {

            while (totaldays >= daysmonthleap[M])
                totaldays -= daysmonthleap[M++];
        } else {
            while (totaldays >= daysmonth[M])
                totaldays -= daysmonth[M++];
        }

        if (what == DTMonth)
            return M;

        return totaldays + 1;
    }

    long long encoded;
    bool _reverse;
};

static std::ostream &operator<<(std::ostream &strm, const DateTime &dt) {
    using std::setfill;
    using std::setw;
    if (dt.ReverseDateTime()) {
        strm << setw(2) << setfill('0') << dt.Hour() << ":" << setw(2)
             << setfill('0') << dt.Minute() << ":" << setw(2) << setfill('0')
             << dt.Second() << "." << setw(3) << setfill('0')
             << dt.Millisecond() << " " << dt.Month() << "-" << dt.Day() << "-"
             << dt.Year();
    } else {
        strm << dt.Month() << "-" << dt.Day() << "-" << dt.Year() << " "
             << setw(2) << setfill('0') << dt.Hour() << ":" << setw(2)
             << setfill('0') << dt.Minute() << ":" << setw(2) << setfill('0')
             << dt.Second() << "." << setw(3) << setfill('0')
             << dt.Millisecond();
    }
    return strm;
}

#if 0 // you have to define these in the source somewhere
         const int DateTime::daysmonth[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
         const int DateTime::daysmonthleap[13] = { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#endif
} // namespace xenon
