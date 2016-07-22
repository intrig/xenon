//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <IT/Xenon.h>
#include <IT/Exception.h>
#include "encode.h"
#include "../Approx.h"

using namespace IT;
using namespace std;

void Encode::enc_float32()
{
    Spec doc("float32.xddl");
    IT_ASSERT(!doc.empty());
    Message xi(doc);
    xi.reserve();

    xi["a"] = 1.44;
    xi["b"] = 40.120;
    xi["c"] = 123.33;

    xi.trim();

    IT_ASSERT_MSG(xi["a"].toFloat(), xi["a"].toFloat() == Approx(1.44));
    IT_ASSERT_MSG(xi["b"].toFloat(), xi["b"].toFloat() == Approx(40.120));
    IT_ASSERT_MSG(xi["c"].toFloat(), xi["c"].toFloat() == Approx(123.33));

    double d = xi["a"].toFloat();
    IT_ASSERT_MSG(d, d == Approx(1.44));
    int l = xi["b"];
    IT_ASSERT_MSG(l, l == 40);
}

void Encode::enc_int64()
{
    Spec doc("int64.xddl");
    IT_ASSERT(!doc.empty());
    Message xi(doc);
    xi.reserve();

    xi["a"] = 1L;
    xi["b"] = 25L;
    xi["c"] = 128L;

    xi.trim();

    IT_ASSERT_MSG(xi["a"].toLongLong(), 
        xi["a"].toLongLong() == 1L);
    IT_ASSERT_MSG(xi["b"].toLongLong(), 
        xi["b"].toLongLong() == 25L);
    IT_ASSERT_MSG(xi["c"].toLongLong(), 
        xi["c"].toLongLong() == 128L);
}

