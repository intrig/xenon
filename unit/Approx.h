#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
// Helper class for comparison of floating point numbers.
// You can do something like this:       
// IT_ASSERT_MSG("float value: " << f, f == Approx(40.875));
namespace IT {
class Approx
{
    public:
    Approx(double value, double delta = 0.001) : v(value), d(delta) {}
    bool operator==(double f) const 
    {
        return (f > (v - d)) && (f < (v + d));
    }

    private:
        double v, d;
};
bool operator==(double f, Approx const & a) { return a == f; }
}
