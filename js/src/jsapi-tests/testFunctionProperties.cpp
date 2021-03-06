/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sw=4 et tw=99:
 */

#include "tests.h"

BEGIN_TEST(testFunctionProperties)
{
    jsvalRoot x(cx);
    EVAL("(function f() {})", x.addr());

    JSObject *obj = JSVAL_TO_OBJECT(x.value());
    jsvalRoot y(cx);

    CHECK(JS_GetProperty(cx, obj, "arguments", y.addr()));
    CHECK_SAME(y, JSVAL_NULL);

    CHECK(JS_GetProperty(cx, obj, "caller", y.addr()));
    CHECK_SAME(y, JSVAL_NULL);

    return true;
}
END_TEST(testFunctionProperties)
