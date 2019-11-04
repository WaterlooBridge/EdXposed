package com.zhenl.riru.xposed.entry;

import android.app.ActivityThread;
import android.content.pm.ApplicationInfo;
import android.content.res.CompatibilityInfo;
import android.text.TextUtils;

import com.zhenl.riru.xposed.entry.hooker.HandleBindAppHooker;
import com.zhenl.riru.xposed.entry.hooker.LoadedApkConstructorHooker;
import com.zhenl.riru.xposed.entry.hooker.OnePlusWorkAroundHooker;
import com.zhenl.riru.xposed.entry.hooker.StartBootstrapServicesHooker;
import com.zhenl.riru.xposed.entry.hooker.SystemMainHooker;
import com.zhenl.riru.xposed.util.Utils;

import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.XposedInit;

public class Router {

    public volatile static boolean forkCompleted = false;

    public static void prepare(boolean isSystem) {
        // this flag is needed when loadModules
        XposedInit.startsSystemServer = isSystem;
//        InstallerChooser.setup();
    }

    public static void checkHookState(String appDataDir) {
        // determine whether allow xposed or not
//        XposedBridge.disableHooks = ConfigManager.shouldHook(parsePackageName(appDataDir));
    }

    private static String parsePackageName(String appDataDir) {
        if (TextUtils.isEmpty(appDataDir)) {
            return "";
        }
        int lastIndex = appDataDir.lastIndexOf("/");
        if (lastIndex < 1) {
            return "";
        }
        return appDataDir.substring(lastIndex + 1);
    }

    public static void installBootstrapHooks(boolean isSystem) {
        // Initialize the Xposed framework
        try {
            XposedInit.initForZygote(isSystem);
        } catch (Throwable t) {
            Utils.logE("error during Xposed initialization", t);
            XposedBridge.disableHooks = true;
        }
    }

    public static void loadModulesSafely(boolean isInZygote) {
        try {
            // FIXME some coredomain app can't reading modules.list
            XposedInit.loadModules(isInZygote);
        } catch (Exception exception) {
            Utils.logE("error loading module list", exception);
        }
    }

    public static void startBootstrapHook(boolean isSystem) {
        Utils.logD("startBootstrapHook starts: isSystem = " + isSystem);
        ClassLoader classLoader = XposedBridge.BOOTCLASSLOADER;
        if (isSystem) {
            XposedHelpers.findAndHookMethod(SystemMainHooker.className, classLoader,
                    SystemMainHooker.methodName, new SystemMainHooker());
        }
        XposedHelpers.findAndHookMethod(HandleBindAppHooker.className, classLoader,
                HandleBindAppHooker.methodName,
                "android.app.ActivityThread$AppBindData",
                new HandleBindAppHooker());
        XposedHelpers.findAndHookConstructor(LoadedApkConstructorHooker.className, classLoader,
                ActivityThread.class, ApplicationInfo.class, CompatibilityInfo.class,
                ClassLoader.class, boolean.class, boolean.class, boolean.class,
                new LoadedApkConstructorHooker());
    }

    public static void startSystemServerHook() {
        XposedHelpers.findAndHookMethod(StartBootstrapServicesHooker.className,
                SystemMainHooker.systemServerCL,
                StartBootstrapServicesHooker.methodName, new StartBootstrapServicesHooker());
    }

    public static void startWorkAroundHook() {
        try {
            XposedHelpers.findAndHookMethod(OnePlusWorkAroundHooker.className,
                    Router.class.getClassLoader(), OnePlusWorkAroundHooker.methodName,
                    int.class, String.class, new OnePlusWorkAroundHooker());
        } catch (Throwable throwable) {
        }
    }

    public static void onEnterChildProcess() {
        forkCompleted = true;
    }
}
