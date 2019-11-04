package com.zhenl.riru.xposed.entry.hooker;

import com.zhenl.riru.xposed.entry.Router;
import com.zhenl.riru.xposed.util.PrebuiltMethodsDeopter;

import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;

import static de.robv.android.xposed.XposedInit.logD;
import static de.robv.android.xposed.XposedInit.logE;


// system_server initialization
// ed: only support sdk >= 21 for now
public class SystemMainHooker extends XC_MethodHook {

    public static String className = "android.app.ActivityThread";
    public static String methodName = "systemMain";
    public static String methodSig = "()Landroid/app/ActivityThread;";

    public static volatile ClassLoader systemServerCL;

    @Override
    protected void afterHookedMethod(MethodHookParam param) throws Throwable {
        if (XposedBridge.disableHooks) {
            return;
        }
        logD("ActivityThread#systemMain() starts");
        try {
            // get system_server classLoader
            systemServerCL = Thread.currentThread().getContextClassLoader();
            // deopt methods in SYSTEMSERVERCLASSPATH
            PrebuiltMethodsDeopter.deoptSystemServerMethods(systemServerCL);
            Router.startSystemServerHook();
        } catch (Throwable t) {
            logE("error when hooking systemMain", t);
        }
    }
}