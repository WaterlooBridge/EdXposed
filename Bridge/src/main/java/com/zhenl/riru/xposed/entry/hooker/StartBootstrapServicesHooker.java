package com.zhenl.riru.xposed.entry.hooker;

import android.os.Build;

import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XC_MethodReplacement;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

import static com.zhenl.riru.xposed.util.ClassLoaderUtils.replaceParentClassLoader;
import static com.zhenl.riru.xposed.util.Utils.logD;
import static de.robv.android.xposed.XposedHelpers.findAndHookMethod;
import static de.robv.android.xposed.XposedInit.loadedPackagesInProcess;
import static de.robv.android.xposed.XposedInit.logE;

public class StartBootstrapServicesHooker extends XC_MethodHook {
    public static String className = "com.android.server.SystemServer";
    public static String methodName = "startBootstrapServices";
    public static String methodSig = "()V";

    @Override
    protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
        if (XposedBridge.disableHooks) {
            return;
        }

        logD("SystemServer#startBootstrapServices() starts");

        try {
            loadedPackagesInProcess.add("android");

            replaceParentClassLoader(SystemMainHooker.systemServerCL);

            XC_LoadPackage.LoadPackageParam lpparam = new XC_LoadPackage.LoadPackageParam(XposedBridge.sLoadedPackageCallbacks);
            lpparam.packageName = "android";
            lpparam.processName = "android"; // it's actually system_server, but other functions return this as well
            lpparam.classLoader = SystemMainHooker.systemServerCL;
            lpparam.appInfo = null;
            lpparam.isFirstApplication = true;
            XC_LoadPackage.callAll(lpparam);

            // Huawei
            try {
                findAndHookMethod("com.android.server.pm.HwPackageManagerService", SystemMainHooker.systemServerCL, "isOdexMode", XC_MethodReplacement.returnConstant(false));
            } catch (XposedHelpers.ClassNotFoundError | NoSuchMethodError ignored) {
            }

            try {
                String className = "com.android.server.pm." + (Build.VERSION.SDK_INT >= 23 ? "PackageDexOptimizer" : "PackageManagerService");
                findAndHookMethod(className, SystemMainHooker.systemServerCL, "dexEntryExists", String.class, XC_MethodReplacement.returnConstant(true));
            } catch (XposedHelpers.ClassNotFoundError | NoSuchMethodError ignored) {
            }
        } catch (Throwable t) {
            logE("error when hooking startBootstrapServices", t);
        }
    }
}
