/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: sw=2 ts=8 et :
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Code.
 *
 * The Initial Developer of the Original Code is
 *   The Mozilla Foundation
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#ifndef mozilla_X11Util_h
#define mozilla_X11Util_h

// Utilities common to all X clients, regardless of UI toolkit.

#if defined(MOZ_WIDGET_GTK2)
#  include <gdk/gdkx.h>
#elif defined(MOZ_WIDGET_QT)
#include "gfxQtPlatform.h"
#undef CursorShape
#  include <X11/Xlib.h>
#else
#  error Unknown toolkit
#endif 

#include "mozilla/Scoped.h"

#include "gfxCore.h"
#include "nsDebug.h"

namespace mozilla {

/**
 * Return the default X Display created and used by the UI toolkit.
 */
inline Display*
DefaultXDisplay()
{
#if defined(MOZ_WIDGET_GTK2)
  return GDK_DISPLAY();
#elif defined(MOZ_WIDGET_QT)
  return gfxQtPlatform::GetXDisplay();
#endif
}

/**
 * Sets *aVisual to point to aDisplay's Visual struct corresponding to
 * aVisualID, and *aDepth to its depth.  When aVisualID is None, these are set
 * to NULL and 0 respectively.  Both out-parameter pointers are assumed
 * non-NULL.  Returns true in both of these situations, but false if aVisualID
 * is not None and not found on the Display.
 */
bool
XVisualIDToInfo(Display* aDisplay, VisualID aVisualID,
                Visual** aVisual, unsigned int* aDepth);

/**
 * Invoke XFree() on a pointer to memory allocated by Xlib (if the
 * pointer is nonnull) when this class goes out of scope.
 */
template <typename T>
struct ScopedXFreePtrTraits
{
  typedef T *type;
  static T *empty() { return NULL; }
  static void release(T *ptr) { if (ptr!=NULL) XFree(ptr); }
};
SCOPED_TEMPLATE(ScopedXFree, ScopedXFreePtrTraits)

/**
 * On construction, set a graceful X error handler that doesn't crash the application and records X errors.
 * On destruction, restore the X error handler to what it was before construction.
 * 
 * The SyncAndGetError() method allows to know whether a X error occurred, optionally allows to get the full XErrorEvent,
 * and resets the recorded X error state so that a single X error will be reported only once.
 *
 * Nesting is correctly handled: multiple nested ScopedXErrorHandler's don't interfere with each other's state. However,
 * if SyncAndGetError is not called on the nested ScopedXErrorHandler, then any X errors caused by X calls made while the nested
 * ScopedXErrorHandler was in place may then be caught by the other ScopedXErrorHandler. This is just a result of X being
 * asynchronous and us not doing any implicit syncing: the only method in this class what causes syncing is SyncAndGetError().
 *
 * This class is not thread-safe at all. It is assumed that only one thread is using any ScopedXErrorHandler's. Given that it's
 * not used on Mac, it should be easy to make it thread-safe by using thread-local storage with __thread.
 */
class NS_GFX ScopedXErrorHandler
{
public:
    // trivial wrapper around XErrorEvent, just adding ctor initializing by zero.
    struct ErrorEvent
    {
        XErrorEvent mError;

        ErrorEvent()
        {
            memset(this, 0, sizeof(ErrorEvent));
        }
    };

private:

    // this ScopedXErrorHandler's ErrorEvent object
    ErrorEvent mXError;

    // static pointer for use by the error handler
    static ErrorEvent* sXErrorPtr;

    // what to restore sXErrorPtr to on destruction
    ErrorEvent* mOldXErrorPtr;

    // what to restore the error handler to on destruction
    int (*mOldErrorHandler)(Display *, XErrorEvent *);

public:

    static int
    ErrorHandler(Display *, XErrorEvent *ev);

    ScopedXErrorHandler();

    ~ScopedXErrorHandler();

    /** \returns true if a X error occurred since the last time this method was called on this ScopedXErrorHandler object,
     *           or since the creation of this ScopedXErrorHandler object if this method was never called on it.
     *
     * \param ev this optional parameter, if set, will be filled with the XErrorEvent object. If multiple errors occurred,
     *           the first one will be returned.
     */
    bool SyncAndGetError(Display *dpy, XErrorEvent *ev = nsnull);

    /** Like SyncAndGetError, but does not sync. Faster, but only reliably catches errors in synchronous calls.
     *
     * \param ev this optional parameter, if set, will be filled with the XErrorEvent object. If multiple errors occurred,
     *           the first one will be returned.
     */
    bool GetError(XErrorEvent *ev = nsnull);
};

} // namespace mozilla

#endif  // mozilla_X11Util_h
