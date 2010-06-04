/* -*- Mode: c++; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
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
 * The Original Code is Mozilla Android code.
 *
 * The Initial Developer of the Original Code is Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Vladimir Vukicevic <vladimir@pobox.com>
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

#ifndef AndroidBridge_h__
#define AndroidBridge_h__

#include <jni.h>
#include <android/log.h>

#include "nsCOMPtr.h"
#include "nsIRunnable.h"

#include "AndroidJavaWrappers.h"

// Some debug #defines
// #define ANDROID_DEBUG_EVENTS
// #define ANDROID_DEBUG_WIDGET

class nsWindow;

namespace mozilla {

class AndroidBridge
{
public:
    static AndroidBridge *ConstructBridge(JNIEnv *jEnv,
                                          jclass jGeckoAppShellClass);

    static AndroidBridge *Bridge() {
        return sBridge;
    }

    static JavaVM *VM() {
        return sBridge->mJavaVM;
    }

    static JNIEnv *JNI() {
        sBridge->EnsureJNIThread();
        return sBridge->mJNIEnv;
    }

    static JNIEnv *JNIForThread() {
        return sBridge->AttachThread();
    }

    // The bridge needs to be constructed via ConstructBridge first,
    // and then once the Gecko main thread is spun up (Gecko side),
    // SetMainThread should be called which will create the JNIEnv for
    // us to use.  toolkit/xre/nsAndroidStartup.cpp calls
    // SetMainThread.
    PRBool SetMainThread(void *thr);

    JNIEnv* AttachThread(PRBool asDaemon = PR_TRUE);

    /* These are all implemented in Java */
    void ShowIME(int aState);

    void EnableAccelerometer(bool aEnable);

    void ReturnIMEQueryResult(const PRUnichar *result, PRUint32 len, int selectionStart, int selectionEnd);

    void NotifyXreExit();

    void ScheduleRestart();

    void SetSurfaceView(jobject jobj);
    AndroidGeckoSurfaceView& SurfaceView() { return mSurfaceView; }

    struct AutoLocalJNIFrame {
        AutoLocalJNIFrame(int nEntries = 128) : mEntries(nEntries) {
            AndroidBridge::Bridge()->JNI()->PushLocalFrame(mEntries);
        }
        // Note! Calling Purge makes all previous local refs created in
        // the AutoLocalJNIFrame's scope INVALID; be sure that you locked down
        // any local refs that you need to keep around in global refs!
        void Purge() {
            AndroidBridge::Bridge()->JNI()->PopLocalFrame(NULL);
            AndroidBridge::Bridge()->JNI()->PushLocalFrame(mEntries);
        }
        ~AutoLocalJNIFrame() {
            AndroidBridge::Bridge()->JNI()->PopLocalFrame(NULL);
        }
        int mEntries;
    };

protected:
    static AndroidBridge *sBridge;

    // the global JavaVM
    JavaVM *mJavaVM;

    // the JNIEnv for the main thread
    JNIEnv *mJNIEnv;
    void *mThread;

    // the GeckoSurfaceView
    AndroidGeckoSurfaceView mSurfaceView;

    // the GeckoAppShell java class
    jclass mGeckoAppShellClass;

    AndroidBridge() { }
    PRBool Init(JNIEnv *jEnv, jclass jGeckoApp);

    void EnsureJNIThread();

    // other things
    jmethodID jShowIME;
    jmethodID jEnableAccelerometer;
    jmethodID jReturnIMEQueryResult;
    jmethodID jNotifyXreExit;
    jmethodID jScheduleRestart;
    jmethodID jGetOutstandingDrawEvents;
};

}

extern "C" JNIEnv * GetJNIForThread();
extern PRBool mozilla_AndroidBridge_SetMainThread(void *);

#endif /* AndroidBridge_h__ */