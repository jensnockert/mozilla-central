/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99:
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla SpiderMonkey JavaScript 1.9 code, released
 * May 28, 2008.
 *
 * The Initial Developer of the Original Code is
 *   Mozilla Foundation
 * Portions created by the Initial Developer are Copyright (C) 2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Andreas Gal <gal@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <string.h>
#include "jsapi.h"
#include "jscntxt.h"
#include "jsprvtd.h"
#include "jsnum.h"
#include "jsobj.h"
#include "jsproxy.h"
#include "jsscope.h"

#include "jsobjinlines.h"

using namespace js;

namespace js {

static bool
OperationInProgress(JSContext *cx, JSObject *proxy)
{
    JSPendingProxyOperation *op = JS_THREAD_DATA(cx)->pendingProxyOperation;
    while (op) {
        if (op->object == proxy)
            return true;
        op = op->next;
    }
    return false;
}

JSProxyHandler::~JSProxyHandler()
{
}

bool
JSProxyHandler::has(JSContext *cx, JSObject *proxy, jsid id, bool *bp)
{
    JS_ASSERT(OperationInProgress(cx, proxy));
    AutoDescriptor desc(cx);
    if (!getPropertyDescriptor(cx, proxy, id, &desc))
        return false;
    *bp = !!desc.obj;
    return true;
}

bool
JSProxyHandler::hasOwn(JSContext *cx, JSObject *proxy, jsid id, bool *bp)
{
    JS_ASSERT(OperationInProgress(cx, proxy));
    AutoDescriptor desc(cx);
    if (!getOwnPropertyDescriptor(cx, proxy, id, &desc))
        return false;
    *bp = !!desc.obj;
    return true;
}

bool
JSProxyHandler::get(JSContext *cx, JSObject *proxy, JSObject *receiver, jsid id, jsval *vp)
{
    JS_ASSERT(OperationInProgress(cx, proxy));
    AutoDescriptor desc(cx);
    if (!getPropertyDescriptor(cx, proxy, id, &desc))
        return false;
    if (!desc.obj) {
        *vp = JSVAL_VOID;
        return true;
    }
    if (!desc.getter) {
        *vp = desc.value;
        return true;
    }
    if (desc.attrs & JSPROP_GETTER) {
        return js_InternalGetOrSet(cx, proxy, id, CastAsObjectJSVal(desc.getter),
                                   JSACC_READ, 0, 0, vp);
    }
    if (desc.attrs & JSPROP_SHORTID)
        id = INT_TO_JSID(desc.shortid);
    return desc.getter(cx, proxy, id, vp);
}

bool
JSProxyHandler::set(JSContext *cx, JSObject *proxy, JSObject *receiver, jsid id, jsval *vp)
{
    JS_ASSERT(OperationInProgress(cx, proxy));
    AutoDescriptor desc(cx);
    if (!getOwnPropertyDescriptor(cx, proxy, id, &desc))
        return false;
    /* The control-flow here differs from ::get() because of the fall-through case below. */
    if (desc.obj) {
        if (desc.setter) {
            if (desc.attrs & JSPROP_SETTER) {
                return js_InternalGetOrSet(cx, proxy, id, CastAsObjectJSVal(desc.setter),
                                           JSACC_READ, 0, 0, vp);
            }
            if (desc.attrs & JSPROP_SHORTID)
                id = INT_TO_JSID(desc.shortid);
            return desc.setter(cx, proxy, id, vp);
        }
        if (desc.attrs & JSPROP_READONLY)
            return true;
        desc.value = *vp;
        return defineProperty(cx, proxy, id, &desc);
    }
    if (!getPropertyDescriptor(cx, proxy, id, &desc))
        return false;
    if (desc.obj) {
        if (desc.setter) {
            if (desc.attrs & JSPROP_SETTER) {
                return js_InternalGetOrSet(cx, proxy, id, CastAsObjectJSVal(desc.setter),
                                           JSACC_READ, 0, 0, vp);
            }
            if (desc.attrs & JSPROP_SHORTID)
                id = INT_TO_JSID(desc.shortid);
            return desc.setter(cx, proxy, id, vp);
        }
        if (desc.attrs & JSPROP_READONLY)
            return true;
        /* fall through */
    }
    desc.obj = proxy;
    desc.value = *vp;
    desc.attrs = 0;
    desc.getter = JSVAL_NULL;
    desc.setter = JSVAL_NULL;
    desc.shortid = 0;
    return defineProperty(cx, proxy, id, &desc);
}

bool
JSProxyHandler::enumerateOwn(JSContext *cx, JSObject *proxy, JSIdArray **idap)
{
    JS_ASSERT(OperationInProgress(cx, proxy));
    if (!getOwnPropertyNames(cx, proxy, idap))
        return false;
    AutoIdArray ida(cx, *idap);
    size_t w = 0;
    jsid *vector = (*idap)->vector;
    AutoDescriptor desc(cx);
    for (size_t n = 0; n < ida.length(); ++n) {
        JS_ASSERT(n >= w);
        vector[w] = vector[n];
        if (!getOwnPropertyDescriptor(cx, proxy, vector[n], &desc))
            return false;
        if (desc.obj && (desc.attrs & JSPROP_ENUMERATE))
            ++w;
    }
    (*idap)->length = w;
    ida.steal();
    return true;
}

bool
JSProxyHandler::iterate(JSContext *cx, JSObject *proxy, uintN flags, jsval *vp)
{
    JS_ASSERT(OperationInProgress(cx, proxy));
    JSIdArray *ida;
    if (!enumerate(cx, proxy, &ida))
        return false;
    AutoIdArray idar(cx, ida);
    return JSIdArrayToIterator(cx, proxy, flags, ida, vp);
}

void
JSProxyHandler::finalize(JSContext *cx, JSObject *proxy)
{
}

void
JSProxyHandler::trace(JSTracer *trc, JSObject *proxy)
{
}

static bool
GetTrap(JSContext *cx, JSObject *handler, JSAtom *atom, jsval *fvalp)
{
    return handler->getProperty(cx, ATOM_TO_JSID(atom), fvalp);
}

static bool
FundamentalTrap(JSContext *cx, JSObject *handler, JSAtom *atom, jsval *fvalp)
{
    if (!GetTrap(cx, handler, atom, fvalp))
        return false;

    if (!js_IsCallable(*fvalp)) {
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_NOT_FUNCTION,
                             js_AtomToPrintableString(cx, atom));
        return false;
    }

    return true;
}

static bool
DerivedTrap(JSContext *cx, JSObject *handler, JSAtom *atom, jsval *fvalp)
{
    JS_ASSERT(atom == ATOM(has) ||
              atom == ATOM(hasOwn) ||
              atom == ATOM(get) ||
              atom == ATOM(set) ||
              atom == ATOM(enumerateOwn) ||
              atom == ATOM(iterate));
    
    return GetTrap(cx, handler, atom, fvalp);
}

static bool
Trap(JSContext *cx, JSObject *handler, jsval fval, uintN argc, jsval* argv, jsval *rval)
{
    JS_CHECK_RECURSION(cx, return false);

    return js_InternalCall(cx, handler, fval, argc, argv, rval);
}

static bool
Trap1(JSContext *cx, JSObject *handler, jsval fval, jsid id, jsval *rval)
{
    JSString *str = js_ValueToString(cx, ID_TO_VALUE(id));
    if (!str)
        return false;
    *rval = STRING_TO_JSVAL(str);
    return Trap(cx, handler, fval, 1, rval, rval);
}

static bool
Trap2(JSContext *cx, JSObject *handler, jsval fval, jsid id, jsval v, jsval *rval)
{
    JSString *str = js_ValueToString(cx, ID_TO_VALUE(id));
    if (!str)
        return false;
    *rval = STRING_TO_JSVAL(str);
    jsval argv[2] = { *rval, v };
    return Trap(cx, handler, fval, 2, argv, rval);
}

static bool
ParsePropertyDescriptorObject(JSContext *cx, JSObject *obj, jsid id, jsval v,
                              JSPropertyDescriptor *desc)
{
    AutoDescriptorArray descs(cx);
    PropertyDescriptor *d = descs.append();
    if (!d || !d->initialize(cx, id, v))
        return false;
    desc->obj = obj;
    desc->value = d->value;
    JS_ASSERT(!(d->attrs & JSPROP_SHORTID));
    desc->attrs = d->attrs;
    desc->getter = d->getter();
    desc->setter = d->setter();
    desc->shortid = 0;
    return true;
}

static bool
MakePropertyDescriptorObject(JSContext *cx, jsid id, JSPropertyDescriptor *desc, jsval *vp)
{
    if (!desc->obj) {
        *vp = JSVAL_VOID;
        return true;
    }
    uintN attrs = desc->attrs;
    jsval getter = (attrs & JSPROP_GETTER) ? CastAsObjectJSVal(desc->getter) : JSVAL_VOID;
    jsval setter = (attrs & JSPROP_SETTER) ? CastAsObjectJSVal(desc->setter) : JSVAL_VOID;
    return js_NewPropertyDescriptorObject(cx, id, attrs, getter, setter, desc->value, vp);
}

static bool
ValueToBool(JSContext *cx, jsval v, bool *bp)
{
    JSBool b;
    if (!JS_ValueToBoolean(cx, v, &b))
        return false;
    *bp = !!b;
    return true;
}

bool
ArrayToJSIdArray(JSContext *cx, jsval array, JSIdArray **idap)
{
    if (JSVAL_IS_PRIMITIVE(array))
        return (*idap = NewIdArray(cx, 0)) != NULL;

    JSObject *obj = JSVAL_TO_OBJECT(array);
    jsuint length;
    if (!js_GetLengthProperty(cx, obj, &length)) {
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_BAD_ARRAY_LENGTH);
        return false;
    }
    AutoIdArray ida(cx, *idap = NewIdArray(cx, length));
    if (!ida)
        return false;
    AutoValueRooter tvr(cx);
    jsid *vector = (*idap)->vector;
    for (jsuint n = 0; n < length; ++n) {
        if (!js_IndexToId(cx, n, &vector[n]))
            return false;
        if (!obj->getProperty(cx, vector[n], tvr.addr()))
            return false;
        if (!JS_ValueToId(cx, tvr.value(), &vector[n]))
            return false;
        vector[n] = js_CheckForStringIndex(vector[n]);
    }
    *idap = ida.steal();
    return true;
}

/* Derived class for all scripted proxy handlers. */
class JSScriptedProxyHandler : public JSProxyHandler {
  public:
    JSScriptedProxyHandler();
    virtual ~JSScriptedProxyHandler();

    /* ES5 Harmony fundamental proxy traps. */
    virtual bool getPropertyDescriptor(JSContext *cx, JSObject *proxy, jsid id,
                                       JSPropertyDescriptor *desc);
    virtual bool getOwnPropertyDescriptor(JSContext *cx, JSObject *proxy, jsid id,
                                          JSPropertyDescriptor *desc);
    virtual bool defineProperty(JSContext *cx, JSObject *proxy, jsid id,
                                JSPropertyDescriptor *desc);
    virtual bool getOwnPropertyNames(JSContext *cx, JSObject *proxy, JSIdArray **idap);
    virtual bool delete_(JSContext *cx, JSObject *proxy, jsid id, bool *bp);
    virtual bool enumerate(JSContext *cx, JSObject *proxy, JSIdArray **idap);
    virtual bool fix(JSContext *cx, JSObject *proxy, jsval *vp);

    /* ES5 Harmony derived proxy traps. */
    virtual bool has(JSContext *cx, JSObject *proxy, jsid id, bool *bp);
    virtual bool hasOwn(JSContext *cx, JSObject *proxy, jsid id, bool *bp);
    virtual bool get(JSContext *cx, JSObject *proxy, JSObject *receiver, jsid id, jsval *vp);
    virtual bool set(JSContext *cx, JSObject *proxy, JSObject *receiver, jsid id, jsval *vp);
    virtual bool enumerateOwn(JSContext *cx, JSObject *proxy, JSIdArray **idap);
    virtual bool iterate(JSContext *cx, JSObject *proxy, uintN flags, jsval *vp);

    /* Spidermonkey extensions. */
    virtual const void *family();

    static JSScriptedProxyHandler singleton;
};

JSScriptedProxyHandler::JSScriptedProxyHandler()
{
}

JSScriptedProxyHandler::~JSScriptedProxyHandler()
{
}

static bool
ReturnedValueMustNotBePrimitive(JSContext *cx, JSObject *proxy, JSAtom *atom, jsval v)
{
    if (JSVAL_IS_PRIMITIVE(v)) {
        js_ReportValueError2(cx, JSMSG_BAD_TRAP_RETURN_VALUE,
                             JSDVG_SEARCH_STACK, OBJECT_TO_JSVAL(proxy), NULL,
                             js_AtomToPrintableString(cx, atom));
        return false;
    }
    return true;
}

static JSObject *
GetProxyHandlerObject(JSContext *cx, JSObject *proxy)
{
    JS_ASSERT(OperationInProgress(cx, proxy));
    return JSVAL_TO_OBJECT(proxy->getProxyHandler());
}

bool
JSScriptedProxyHandler::getPropertyDescriptor(JSContext *cx, JSObject *proxy, jsid id,
                                              JSPropertyDescriptor *desc)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    AutoValueRooter tvr(cx);
    return FundamentalTrap(cx, handler, ATOM(getPropertyDescriptor), tvr.addr()) &&
           Trap1(cx, handler, tvr.value(), id, tvr.addr()) &&
           ReturnedValueMustNotBePrimitive(cx, proxy, ATOM(getPropertyDescriptor), tvr.value()) &&
           ParsePropertyDescriptorObject(cx, proxy, id, tvr.value(), desc);
}

bool
JSScriptedProxyHandler::getOwnPropertyDescriptor(JSContext *cx, JSObject *proxy, jsid id,
                                                 JSPropertyDescriptor *desc)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    AutoValueRooter tvr(cx);
    return FundamentalTrap(cx, handler, ATOM(getOwnPropertyDescriptor), tvr.addr()) &&
           Trap1(cx, handler, tvr.value(), id, tvr.addr()) &&
           ReturnedValueMustNotBePrimitive(cx, proxy, ATOM(getPropertyDescriptor), tvr.value()) &&
           ParsePropertyDescriptorObject(cx, proxy, id, tvr.value(), desc);
}

bool
JSScriptedProxyHandler::defineProperty(JSContext *cx, JSObject *proxy, jsid id,
                                       JSPropertyDescriptor *desc)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    AutoValueRooter tvr(cx);
    AutoValueRooter fval(cx);
    return FundamentalTrap(cx, handler, ATOM(defineProperty), fval.addr()) &&
           MakePropertyDescriptorObject(cx, id, desc, tvr.addr()) &&
           Trap2(cx, handler, fval.value(), id, tvr.value(), tvr.addr());
}

bool
JSScriptedProxyHandler::getOwnPropertyNames(JSContext *cx, JSObject *proxy, JSIdArray **idap)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    AutoValueRooter tvr(cx);
    return FundamentalTrap(cx, handler, ATOM(getOwnPropertyNames), tvr.addr()) &&
           Trap(cx, handler, tvr.value(), 0, NULL, tvr.addr()) &&
           ArrayToJSIdArray(cx, tvr.value(), idap);
}

bool
JSScriptedProxyHandler::delete_(JSContext *cx, JSObject *proxy, jsid id, bool *bp)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    AutoValueRooter tvr(cx);
    return FundamentalTrap(cx, handler, ATOM(delete), tvr.addr()) &&
           Trap1(cx, handler, tvr.value(), id, tvr.addr()) &&
           ValueToBool(cx, tvr.value(), bp);
}

bool
JSScriptedProxyHandler::enumerate(JSContext *cx, JSObject *proxy, JSIdArray **idap)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    AutoValueRooter tvr(cx);
    return FundamentalTrap(cx, handler, ATOM(enumerate), tvr.addr()) &&
           Trap(cx, handler, tvr.value(), 0, NULL, tvr.addr()) &&
           ArrayToJSIdArray(cx, tvr.value(), idap);
}

bool
JSScriptedProxyHandler::fix(JSContext *cx, JSObject *proxy, jsval *vp)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    return FundamentalTrap(cx, handler, ATOM(fix), vp) &&
           Trap(cx, handler, *vp, 0, NULL, vp);
}

bool
JSScriptedProxyHandler::has(JSContext *cx, JSObject *proxy, jsid id, bool *bp)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    AutoValueRooter tvr(cx);
    if (!DerivedTrap(cx, handler, ATOM(has), tvr.addr()))
        return false;
    if (!js_IsCallable(tvr.value()))
        return JSProxyHandler::has(cx, proxy, id, bp);
    return Trap1(cx, handler, tvr.value(), id, tvr.addr()) &&
           ValueToBool(cx, tvr.value(), bp);
}

bool
JSScriptedProxyHandler::hasOwn(JSContext *cx, JSObject *proxy, jsid id, bool *bp)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    AutoValueRooter tvr(cx);
    if (!DerivedTrap(cx, handler, ATOM(hasOwn), tvr.addr()))
        return false;
    if (!js_IsCallable(tvr.value()))
        return JSProxyHandler::hasOwn(cx, proxy, id, bp);
    return Trap1(cx, handler, tvr.value(), id, tvr.addr()) &&
           ValueToBool(cx, tvr.value(), bp);
}

bool
JSScriptedProxyHandler::get(JSContext *cx, JSObject *proxy, JSObject *receiver, jsid id, jsval *vp)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    JSString *str = js_ValueToString(cx, ID_TO_VALUE(id));
    if (!str)
        return false;
    AutoValueRooter tvr(cx, STRING_TO_JSVAL(str));
    jsval argv[] = { OBJECT_TO_JSVAL(receiver), tvr.value() };
    AutoValueRooter fval(cx);
    if (!DerivedTrap(cx, handler, ATOM(get), fval.addr()))
        return false;
    if (!js_IsCallable(fval.value()))
        return JSProxyHandler::get(cx, proxy, receiver, id, vp);
    return Trap(cx, handler, fval.value(), 2, argv, vp);
}

bool
JSScriptedProxyHandler::set(JSContext *cx, JSObject *proxy, JSObject *receiver, jsid id, jsval *vp)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    JSString *str = js_ValueToString(cx, ID_TO_VALUE(id));
    if (!str)
        return false;
    AutoValueRooter tvr(cx, STRING_TO_JSVAL(str));
    jsval argv[] = { OBJECT_TO_JSVAL(receiver), tvr.value(), *vp };
    AutoValueRooter fval(cx);
    if (!DerivedTrap(cx, handler, ATOM(set), fval.addr()))
        return false;
    if (!js_IsCallable(fval.value()))
        return JSProxyHandler::set(cx, proxy, receiver, id, vp);
    return Trap(cx, handler, fval.value(), 3, argv, tvr.addr());
}

bool
JSScriptedProxyHandler::enumerateOwn(JSContext *cx, JSObject *proxy, JSIdArray **idap)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    AutoValueRooter tvr(cx);
    if (!DerivedTrap(cx, handler, ATOM(enumerateOwn), tvr.addr()))
        return false;
    if (!js_IsCallable(tvr.value()))
        return JSProxyHandler::enumerateOwn(cx, proxy, idap);
    return Trap(cx, handler, tvr.value(), 0, NULL, tvr.addr()) &&
           ArrayToJSIdArray(cx, tvr.value(), idap);
}

bool
JSScriptedProxyHandler::iterate(JSContext *cx, JSObject *proxy, uintN flags, jsval *vp)
{
    JSObject *handler = GetProxyHandlerObject(cx, proxy);
    AutoValueRooter tvr(cx);
    if (!DerivedTrap(cx, handler, ATOM(iterate), tvr.addr()))
        return false;
    if (!js_IsCallable(tvr.value()))
        return JSProxyHandler::iterate(cx, proxy, flags, vp);
    return Trap(cx, handler, tvr.value(), 0, NULL, vp) &&
           ReturnedValueMustNotBePrimitive(cx, proxy, ATOM(iterate), *vp);
}

const void *
JSScriptedProxyHandler::family()
{
    return &singleton;
}

JSScriptedProxyHandler JSScriptedProxyHandler::singleton;

static JSProxyHandler *
JSVAL_TO_HANDLER(jsval handler)
{
    return (JSProxyHandler *) JSVAL_TO_PRIVATE(handler);
}

class AutoPendingProxyOperation {
    JSThreadData *data;
    JSPendingProxyOperation op;
  public:
    AutoPendingProxyOperation(JSContext *cx, JSObject *proxy) : data(JS_THREAD_DATA(cx)) {
        op.next = data->pendingProxyOperation;
        op.object = proxy;
        data->pendingProxyOperation = &op;
    }

    ~AutoPendingProxyOperation() {
        JS_ASSERT(data->pendingProxyOperation == &op);
        data->pendingProxyOperation = op.next;
    }
};

bool
JSProxy::getPropertyDescriptor(JSContext *cx, JSObject *proxy, jsid id, JSPropertyDescriptor *desc)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.getPropertyDescriptor(cx, proxy, id, desc);
    return JSVAL_TO_HANDLER(handler)->getPropertyDescriptor(cx, proxy, id, desc);
}

bool
JSProxy::getPropertyDescriptor(JSContext *cx, JSObject *proxy, jsid id, jsval *vp)
{
    AutoPendingProxyOperation pending(cx, proxy);
    AutoDescriptor desc(cx);
    return JSProxy::getPropertyDescriptor(cx, proxy, id, &desc) &&
           MakePropertyDescriptorObject(cx, id, &desc, vp);
}

bool
JSProxy::getOwnPropertyDescriptor(JSContext *cx, JSObject *proxy, jsid id,
                                  JSPropertyDescriptor *desc)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.getOwnPropertyDescriptor(cx, proxy, id, desc);
    return JSVAL_TO_HANDLER(handler)->getOwnPropertyDescriptor(cx, proxy, id, desc);
}

bool
JSProxy::getOwnPropertyDescriptor(JSContext *cx, JSObject *proxy, jsid id, jsval *vp)
{
    AutoPendingProxyOperation pending(cx, proxy);
    AutoDescriptor desc(cx);
    return JSProxy::getOwnPropertyDescriptor(cx, proxy, id, &desc) &&
           MakePropertyDescriptorObject(cx, id, &desc, vp);
}

bool
JSProxy::defineProperty(JSContext *cx, JSObject *proxy, jsid id, JSPropertyDescriptor *desc)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.defineProperty(cx, proxy, id, desc);
    return JSVAL_TO_HANDLER(handler)->defineProperty(cx, proxy, id, desc);
}

bool
JSProxy::defineProperty(JSContext *cx, JSObject *proxy, jsid id, jsval v)
{
    AutoPendingProxyOperation pending(cx, proxy);
    AutoDescriptor desc(cx);
    return ParsePropertyDescriptorObject(cx, proxy, id, v, &desc) &&
           JSProxy::defineProperty(cx, proxy, id, &desc);
}

bool
JSProxy::getOwnPropertyNames(JSContext *cx, JSObject *proxy, JSIdArray **idap)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.getOwnPropertyNames(cx, proxy, idap);
    return JSVAL_TO_HANDLER(handler)->getOwnPropertyNames(cx, proxy, idap);
}

bool
JSProxy::delete_(JSContext *cx, JSObject *proxy, jsid id, bool *bp)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.delete_(cx, proxy, id, bp);
    return JSVAL_TO_HANDLER(handler)->delete_(cx, proxy, id, bp);
}

bool
JSProxy::enumerate(JSContext *cx, JSObject *proxy, JSIdArray **idap)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.enumerate(cx, proxy, idap);
    return JSVAL_TO_HANDLER(handler)->enumerate(cx, proxy, idap);
}

bool
JSProxy::fix(JSContext *cx, JSObject *proxy, jsval *vp)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.fix(cx, proxy, vp);
    return JSVAL_TO_HANDLER(handler)->fix(cx, proxy, vp);
}

bool
JSProxy::has(JSContext *cx, JSObject *proxy, jsid id, bool *bp)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.has(cx, proxy, id, bp);
    return JSVAL_TO_HANDLER(handler)->has(cx, proxy, id, bp);
}

bool
JSProxy::hasOwn(JSContext *cx, JSObject *proxy, jsid id, bool *bp)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.hasOwn(cx, proxy, id, bp);
    return JSVAL_TO_HANDLER(handler)->hasOwn(cx, proxy, id, bp);
}

bool
JSProxy::get(JSContext *cx, JSObject *proxy, JSObject *receiver, jsid id, jsval *vp)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.get(cx, proxy, receiver, id, vp);
    return JSVAL_TO_HANDLER(handler)->get(cx, proxy, receiver, id, vp);
}

bool
JSProxy::set(JSContext *cx, JSObject *proxy, JSObject *receiver, jsid id, jsval *vp)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.set(cx, proxy, receiver, id, vp);
    return JSVAL_TO_HANDLER(handler)->set(cx, proxy, receiver, id, vp);
}

bool
JSProxy::enumerateOwn(JSContext *cx, JSObject *proxy, JSIdArray **idap)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.enumerateOwn(cx, proxy, idap);
    return JSVAL_TO_HANDLER(handler)->enumerateOwn(cx, proxy, idap);
}

bool
JSProxy::iterate(JSContext *cx, JSObject *proxy, uintN flags, jsval *vp)
{
    AutoPendingProxyOperation pending(cx, proxy);
    jsval handler = proxy->getProxyHandler();
    if (JSVAL_IS_OBJECT(handler))
        return JSScriptedProxyHandler::singleton.iterate(cx, proxy, flags, vp);
    return JSVAL_TO_HANDLER(handler)->iterate(cx, proxy, flags, vp);
}

JS_FRIEND_API(JSBool)
GetProxyObjectClass(JSContext *cx, JSObject *proxy, const char **namep)
{
    if (!proxy->isProxy()) {
        char *bytes = js_DecompileValueGenerator(cx, JSDVG_SEARCH_STACK,
                                                 OBJECT_TO_JSVAL(proxy), NULL);
        if (!bytes)
            return JS_FALSE;
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL,
                             JSMSG_UNEXPECTED_TYPE, bytes, "not a proxy");
        return false;
    }
    if (proxy->isFunctionProxy()) {
        *namep = "Function";
        return true;
    }
    jsval nameval = proxy->fslots[JSSLOT_PROXY_CLASS];
    if (nameval == JSVAL_VOID) {
        *namep ="Object";
        return true;
    }
    JS_ASSERT(JSVAL_IS_STRING(nameval));
    *namep = JS_GetStringBytesZ(cx, JSVAL_TO_STRING(nameval));
    return *namep != NULL;
}

static JSBool
proxy_LookupProperty(JSContext *cx, JSObject *obj, jsid id, JSObject **objp,
                     JSProperty **propp)
{
    bool found;
    if (!JSProxy::has(cx, obj, id, &found))
        return false;

    if (found) {
        *propp = (JSProperty *)id;
        *objp = obj;
    } else {
        *objp = NULL;
        *propp = NULL;
    }
    return true;
}

static JSBool
proxy_DefineProperty(JSContext *cx, JSObject *obj, jsid id, jsval value,
                     JSPropertyOp getter, JSPropertyOp setter, uintN attrs)
{
    AutoDescriptor desc(cx);
    desc.obj = obj;
    desc.value = value;
    desc.attrs = (attrs & (~JSPROP_SHORTID));
    desc.getter = getter;
    desc.setter = setter;
    desc.shortid = 0;
    return JSProxy::defineProperty(cx, obj, id, &desc);
}

static JSBool
proxy_GetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp)
{
    return JSProxy::get(cx, obj, obj, id, vp);
}

static JSBool
proxy_SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    return JSProxy::set(cx, obj, obj, id, vp);
}

static JSBool
proxy_GetAttributes(JSContext *cx, JSObject *obj, jsid id, uintN *attrsp)
{
    AutoDescriptor desc(cx);
    if (!JSProxy::getOwnPropertyDescriptor(cx, obj, id, &desc))
        return false;
    *attrsp = desc.attrs;
    return true;
}

static JSBool
proxy_SetAttributes(JSContext *cx, JSObject *obj, jsid id, uintN *attrsp)
{
    /* Lookup the current property descriptor so we have setter/getter/value. */
    AutoDescriptor desc(cx);
    if (!JSProxy::getOwnPropertyDescriptor(cx, obj, id, &desc))
        return false;
    desc.attrs = (*attrsp & (~JSPROP_SHORTID));
    return JSProxy::defineProperty(cx, obj, id, &desc);
}

static JSBool
proxy_DeleteProperty(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
    bool deleted;
    if (!JSProxy::delete_(cx, obj, id, &deleted))
        return false;
    *rval = BOOLEAN_TO_JSVAL(deleted);
    return true;
}

static void
proxy_TraceObject(JSTracer *trc, JSObject *obj)
{
    JSContext *cx = trc->context;

    if (!JS_CLIST_IS_EMPTY(&cx->runtime->watchPointList))
        js_TraceWatchPoints(trc, obj);

    JSClass *clasp = obj->getClass();
    if (clasp->mark) {
        if (clasp->flags & JSCLASS_MARK_IS_TRACE)
            ((JSTraceOp) clasp->mark)(trc, obj);
        else if (IS_GC_MARKING_TRACER(trc))
            (void) clasp->mark(cx, obj, trc);
    }

    obj->traceProtoAndParent(trc);

    jsval handler = obj->fslots[JSSLOT_PROXY_HANDLER];
    if (!JSVAL_IS_PRIMITIVE(handler))
        JS_CALL_OBJECT_TRACER(trc, JSVAL_TO_OBJECT(handler), "handler");
    else
        JSVAL_TO_HANDLER(handler)->trace(trc, obj);
    if (obj->isFunctionProxy()) {
        JS_CALL_VALUE_TRACER(trc, obj->fslots[JSSLOT_PROXY_CALL], "call");
        JS_CALL_VALUE_TRACER(trc, obj->fslots[JSSLOT_PROXY_CONSTRUCT], "construct");
    } else {
        JS_CALL_VALUE_TRACER(trc, obj->fslots[JSSLOT_PROXY_PRIVATE], "private");
    }
}

static JSType
proxy_TypeOf_obj(JSContext *cx, JSObject *obj)
{
    return JSTYPE_OBJECT;
}

extern JSObjectOps js_ObjectProxyObjectOps;

static const JSObjectMap SharedObjectProxyMap(&js_ObjectProxyObjectOps, JSObjectMap::SHAPELESS);

JSObjectOps js_ObjectProxyObjectOps = {
    &SharedObjectProxyMap,
    proxy_LookupProperty,
    proxy_DefineProperty,
    proxy_GetProperty,
    proxy_SetProperty,
    proxy_GetAttributes,
    proxy_SetAttributes,
    proxy_DeleteProperty,
    js_DefaultValue,
    js_Enumerate,
    js_CheckAccess,
    proxy_TypeOf_obj,
    proxy_TraceObject,
    NULL,   /* thisObject */
    NULL,   /* call */
    NULL,   /* construct */
    js_HasInstance,
    NULL
};

static JSObjectOps *
obj_proxy_getObjectOps(JSContext *cx, JSClass *clasp)
{
    return &js_ObjectProxyObjectOps;
}

JS_FRIEND_API(JSClass) ObjectProxyClass = {
    "Proxy",
    JSCLASS_HAS_RESERVED_SLOTS(3),
    JS_PropertyStub,        JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub,       JS_ResolveStub,  JS_ConvertStub,  NULL,
    obj_proxy_getObjectOps, NULL,            NULL,            NULL,
    NULL,                   NULL,            NULL,            NULL
};

JSBool
proxy_Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSObject *proxy = JSVAL_TO_OBJECT(argv[-2]);
    JS_ASSERT(proxy->isProxy());
    AutoPendingProxyOperation pending(cx, proxy);
    return js_InternalCall(cx, obj, proxy->fslots[JSSLOT_PROXY_CALL], argc, argv, rval);
}

JSBool
proxy_Construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSObject *proxy = JSVAL_TO_OBJECT(argv[-2]);
    JS_ASSERT(proxy->isProxy());
    AutoPendingProxyOperation pending(cx, proxy);
    jsval fval = proxy->fslots[JSSLOT_PROXY_CONSTRUCT];
    if (fval == JSVAL_VOID) {
        /*
         * We don't have an explicit constructor trap so allocate a new
         * object and use the call trap.
         */
        fval = proxy->fslots[JSSLOT_PROXY_CALL];
        JS_ASSERT(JSVAL_IS_OBJECT(fval));

        /*
         * proxy is the constructor, so get proxy.prototype as the proto
         * of the new object.
         */
        if (!JSProxy::get(cx, proxy, obj, ATOM_TO_JSID(ATOM(classPrototype)), rval))
            return false;
        JSObject *proto = !JSVAL_IS_PRIMITIVE(*rval) ? JSVAL_TO_OBJECT(*rval) : NULL;

        JSObject *newobj = NewObject(cx, &js_ObjectClass, proto, NULL);
        *rval = OBJECT_TO_JSVAL(newobj);

        /* If the call returns an object, return that, otherwise the original newobj. */
        if (!js_InternalCall(cx, newobj, proxy->fslots[JSSLOT_PROXY_CALL], argc, argv, rval))
            return false;
        if (JSVAL_IS_PRIMITIVE(*rval))
            *rval = OBJECT_TO_JSVAL(newobj);

        return true;
    }
    return js_InternalCall(cx, obj, fval, argc, argv, rval);
}

static JSType
proxy_TypeOf_fun(JSContext *cx, JSObject *obj)
{
    return JSTYPE_FUNCTION;
}

extern JSObjectOps js_FunctionProxyObjectOps;

static const JSObjectMap SharedFunctionProxyMap(&js_FunctionProxyObjectOps, JSObjectMap::SHAPELESS);

#define proxy_HasInstance js_FunctionClass.hasInstance

JSObjectOps js_FunctionProxyObjectOps = {
    &SharedFunctionProxyMap,
    proxy_LookupProperty,
    proxy_DefineProperty,
    proxy_GetProperty,
    proxy_SetProperty,
    proxy_GetAttributes,
    proxy_SetAttributes,
    proxy_DeleteProperty,
    js_DefaultValue,
    js_Enumerate,
    js_CheckAccess,
    proxy_TypeOf_fun,
    proxy_TraceObject,
    NULL,   /* thisObject */
    proxy_Call,
    proxy_Construct,
    proxy_HasInstance,
    NULL
};

static JSObjectOps *
fun_proxy_getObjectOps(JSContext *cx, JSClass *clasp)
{
    return &js_FunctionProxyObjectOps;
}

JS_FRIEND_API(JSClass) FunctionProxyClass = {
    "Proxy",
    JSCLASS_HAS_RESERVED_SLOTS(3),
    JS_PropertyStub,        JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub,       JS_ResolveStub,  JS_ConvertStub,  NULL,
    fun_proxy_getObjectOps, NULL,            NULL,            NULL,
    NULL,                   NULL,            NULL,            NULL
};

JS_FRIEND_API(JSObject *)
NewObjectProxy(JSContext *cx, jsval handler, JSObject *proto, JSObject *parent, JSString *className)
{
    JSObject *obj = NewObjectWithGivenProto(cx, &ObjectProxyClass, proto, parent);
    if (!obj)
        return NULL;
    obj->fslots[JSSLOT_PROXY_HANDLER] = handler;
    obj->fslots[JSSLOT_PROXY_CLASS] = className ? STRING_TO_JSVAL(className) : JSVAL_VOID;
    obj->fslots[JSSLOT_PROXY_PRIVATE] = JSVAL_VOID;
    return obj;
}

JS_FRIEND_API(JSObject *)
NewFunctionProxy(JSContext *cx, jsval handler, JSObject *proto, JSObject *parent,
                 JSObject *call, JSObject *construct)
{
    JSObject *obj = NewObjectWithGivenProto(cx, &FunctionProxyClass, proto, parent);
    if (!obj)
        return NULL;
    obj->fslots[JSSLOT_PROXY_HANDLER] = handler;
    obj->fslots[JSSLOT_PROXY_CALL] = call ? OBJECT_TO_JSVAL(call) : JSVAL_VOID;
    obj->fslots[JSSLOT_PROXY_CONSTRUCT] = construct ? OBJECT_TO_JSVAL(construct) : JSVAL_VOID;
    return obj;
}

static JSObject *
NonNullObject(JSContext *cx, jsval v)
{
    if (JSVAL_IS_PRIMITIVE(v)) {
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_NOT_NONNULL_OBJECT);
        return NULL;
    }
    return JSVAL_TO_OBJECT(v);
}

static JSBool
proxy_create(JSContext *cx, uintN argc, jsval *vp)
{
    if (argc < 1) {
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_MORE_ARGS_NEEDED,
                             "create", "0", "s");
        return false;
    }
    JSObject *handler;
    if (!(handler = NonNullObject(cx, vp[2])))
        return false;
    JSObject *proto, *parent;
    if (argc > 1 && !JSVAL_IS_PRIMITIVE(vp[3])) {
        proto = JSVAL_TO_OBJECT(vp[3]);
        parent = proto->getParent();
    } else {
        JS_ASSERT(VALUE_IS_FUNCTION(cx, vp[0]));
        proto = NULL;
        parent = JSVAL_TO_OBJECT(vp[0])->getParent();
    }
    JSString *className = (argc > 2 && JSVAL_IS_STRING(vp[4])) ? JSVAL_TO_STRING(vp[4]) : NULL;
    JSObject *proxy = NewObjectProxy(cx, OBJECT_TO_JSVAL(handler), proto, parent, className);
    if (!proxy)
        return false;

    *vp = OBJECT_TO_JSVAL(proxy);
    return true;
}

static JSBool
proxy_createFunction(JSContext *cx, uintN argc, jsval *vp)
{
    if (argc < 2) {
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_MORE_ARGS_NEEDED,
                             "createFunction", "1", "");
        return false;
    }
    JSObject *handler;
    if (!(handler = NonNullObject(cx, vp[2])))
        return false;
    JSObject *proto, *parent;
    parent = JSVAL_TO_OBJECT(vp[0])->getParent();
    if (!js_GetClassPrototype(cx, parent, JSProto_Function, &proto))
        return false;
    parent = proto->getParent();

    JSObject *call = js_ValueToCallableObject(cx, &vp[3], JSV2F_SEARCH_STACK);
    if (!call)
        return false;
    JSObject *construct = NULL;
    if (argc > 2) {
        construct = js_ValueToCallableObject(cx, &vp[4], JSV2F_SEARCH_STACK);
        if (!construct)
            return false;
    }

    JSObject *proxy = NewFunctionProxy(cx, OBJECT_TO_JSVAL(handler), proto, parent,
                                       call, construct);
    if (!proxy)
        return false;

    *vp = OBJECT_TO_JSVAL(proxy);
    return true;
}

#ifdef DEBUG

static JSBool
proxy_isTrapping(JSContext *cx, uintN argc, jsval *vp)
{
    if (argc < 1) {
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_MORE_ARGS_NEEDED,
                             "isTrapping", "0", "s");
        return false;
    }
    JSObject *obj;
    if (!(obj = NonNullObject(cx, vp[2])))
        return false;
    *vp = BOOLEAN_TO_JSVAL(obj->isProxy());
    return true;
}

static JSBool
proxy_fix(JSContext *cx, uintN argc, jsval *vp)
{
    if (argc < 1) {
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_MORE_ARGS_NEEDED,
                             "fix", "0", "s");
        return false;
    }
    JSObject *obj;
    if (!(obj = NonNullObject(cx, vp[2])))
        return false;
    if (obj->isProxy()) {
        JSBool flag;
        if (!FixProxy(cx, obj, &flag))
            return false;
        *vp = BOOLEAN_TO_JSVAL(flag);
    } else {
        *vp = JSVAL_TRUE;
    }
    return true;
}

#endif

static JSFunctionSpec static_methods[] = {
    JS_FN("create",         proxy_create,          2, 0),
    JS_FN("createFunction", proxy_createFunction,  3, 0),
#ifdef DEBUG
    JS_FN("isTrapping",     proxy_isTrapping,      1, 0),
    JS_FN("fix",            proxy_fix,             1, 0),
#endif
    JS_FS_END
};

extern JSClass CallableObjectClass;

static const uint32 JSSLOT_CALLABLE_CALL = JSSLOT_PRIVATE;
static const uint32 JSSLOT_CALLABLE_CONSTRUCT = JSSLOT_PRIVATE + 1;

static JSBool
callable_Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSObject *callable = JSVAL_TO_OBJECT(argv[-2]);
    JS_ASSERT(callable->getClass() == &CallableObjectClass);
    jsval fval = callable->fslots[JSSLOT_CALLABLE_CALL];
    return js_InternalCall(cx, obj, fval, argc, argv, rval);
}

static JSBool
callable_Construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSObject *callable = JSVAL_TO_OBJECT(argv[-2]);
    JS_ASSERT(callable->getClass() == &CallableObjectClass);
    jsval fval = callable->fslots[JSSLOT_CALLABLE_CONSTRUCT];
    if (fval == JSVAL_VOID) {
        /* We don't have an explicit constructor so allocate a new object and use the call. */
        fval = callable->fslots[JSSLOT_CALLABLE_CALL];
        JS_ASSERT(JSVAL_IS_OBJECT(fval));

        /* callable is the constructor, so get callable.prototype is the proto of the new object. */
        if (!callable->getProperty(cx, ATOM_TO_JSID(ATOM(classPrototype)), rval))
            return false;
        JSObject *proto = !JSVAL_IS_PRIMITIVE(*rval) ? JSVAL_TO_OBJECT(*rval) : NULL;

        JSObject *newobj = NewObject(cx, &js_ObjectClass, proto, NULL);
        *rval = OBJECT_TO_JSVAL(newobj);

        /* If the call returns an object, return that, otherwise the original newobj. */
        if (!js_InternalCall(cx, newobj, callable->fslots[JSSLOT_CALLABLE_CALL], argc, argv, rval))
            return false;
        if (JSVAL_IS_PRIMITIVE(*rval))
            *rval = OBJECT_TO_JSVAL(newobj);

        return true;
    }
    return js_InternalCall(cx, obj, fval, argc, argv, rval);
}

JSClass CallableObjectClass = {
    "Function",
    JSCLASS_HAS_RESERVED_SLOTS(2),
    JS_PropertyStub,        JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub,       JS_ResolveStub,  JS_ConvertStub,  NULL,
    NULL,                   NULL,            callable_Call,   callable_Construct,
    NULL,                   NULL,            NULL,            NULL
};

JS_FRIEND_API(JSBool)
FixProxy(JSContext *cx, JSObject *proxy, JSBool *bp)
{
    AutoValueRooter tvr(cx);
    if (!JSProxy::fix(cx, proxy, tvr.addr()))
        return false;
    if (tvr.value() == JSVAL_VOID) {
        *bp = false;
        return true;
    }

    if (OperationInProgress(cx, proxy)) {
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_BAD_PROXY_FIX);
        return false;
    }

    JSObject *props;
    if (!(props = NonNullObject(cx, tvr.value())))
        return false;

    JSObject *proto = proxy->getProto();
    JSObject *parent = proxy->getParent();
    JSClass *clasp = proxy->isFunctionProxy() ? &CallableObjectClass : &js_ObjectClass;

    /* Make a blank object from the recipe fix provided to us. */
    JSObject *newborn = NewObjectWithGivenProto(cx, clasp, proto, parent);
    if (!newborn)
        return NULL;
    AutoValueRooter tvr2(cx, newborn);

    if (clasp == &CallableObjectClass) {
        newborn->fslots[JSSLOT_CALLABLE_CALL] = proxy->fslots[JSSLOT_PROXY_CALL];
        newborn->fslots[JSSLOT_CALLABLE_CONSTRUCT] = proxy->fslots[JSSLOT_PROXY_CONSTRUCT];
    }

    {
        AutoPendingProxyOperation pending(cx, proxy);
        if (!js_PopulateObject(cx, newborn, props))
            return false;
    }

    /* Trade spaces between the newborn object and the proxy. */
    proxy->swap(newborn);

    /* The GC will dispose of the proxy object. */

    *bp = true;
    return true;
}

}

JSClass js_ProxyClass = {
    "Proxy",
    JSCLASS_HAS_CACHED_PROTO(JSProto_Proxy),
    JS_PropertyStub,        JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub,       JS_ResolveStub,  JS_ConvertStub,  NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

JS_FRIEND_API(JSObject *)
js_InitProxyClass(JSContext *cx, JSObject *obj)
{
    JSObject *module = NewObject(cx, &js_ProxyClass, NULL, obj);
    if (!module)
        return NULL;
    if (!JS_DefineProperty(cx, obj, "Proxy", OBJECT_TO_JSVAL(module),
                           JS_PropertyStub, JS_PropertyStub, 0)) {
        return NULL;
    }
    if (!JS_DefineFunctions(cx, module, static_methods))
        return NULL;
    return obj;
}