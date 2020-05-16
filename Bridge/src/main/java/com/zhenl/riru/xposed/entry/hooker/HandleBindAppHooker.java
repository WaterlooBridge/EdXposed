package com.zhenl.riru.xposed.entry.hooker;

import android.app.ActivityThread;
import android.app.LoadedApk;
import android.content.ComponentName;
import android.content.pm.ApplicationInfo;
import android.content.res.CompatibilityInfo;

import com.zhenl.riru.xposed.Main;
import com.zhenl.riru.xposed.util.Utils;

import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

import static com.zhenl.riru.xposed.config.InstallerChooser.INSTALLER_PACKAGE_NAME;
import static com.zhenl.riru.xposed.entry.hooker.XposedBlackListHooker.BLACK_LIST_PACKAGE_NAME;
import static com.zhenl.riru.xposed.util.ClassLoaderUtils.replaceParentClassLoader;
import static de.robv.android.xposed.XposedHelpers.getObjectField;
import static de.robv.android.xposed.XposedHelpers.setObjectField;
import static de.robv.android.xposed.XposedInit.loadedPackagesInProcess;
import static de.robv.android.xposed.XposedInit.logD;
import static de.robv.android.xposed.XposedInit.logE;

// normal process initialization (for new Activity, Service, BroadcastReceiver etc.)
public class HandleBindAppHooker extends XC_MethodHook {

    public static String className = "android.app.ActivityThread";
    public static String methodName = "handleBindApplication";
    public static String methodSig = "(Landroid/app/ActivityThread$AppBindData;)V";

    @Override
    protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
        if (XposedBlackListHooker.shouldDisableHooks("")) {
            return;
        }
        try {
            logD("ActivityThread#handleBindApplication() starts");
            ActivityThread activityThread = (ActivityThread) param.thisObject;
            Object bindData = param.args[0];
            ApplicationInfo appInfo = (ApplicationInfo) getObjectField(bindData, "appInfo");
            // save app process name here for later use
            Main.appProcessName = (String) getObjectField(bindData, "processName");
            String reportedPackageName = appInfo.packageName.equals("android") ? "system" : appInfo.packageName;
            Utils.logD("processName=" + Main.appProcessName +
                    ", packageName=" + reportedPackageName + ", appDataDir=" + Main.appDataDir);

            if (XposedBlackListHooker.shouldDisableHooks(reportedPackageName)) {
                return;
            }

            ComponentName instrumentationName = (ComponentName) getObjectField(bindData, "instrumentationName");
            if (instrumentationName != null) {
                logD("Instrumentation detected, disabling framework for");
                XposedBridge.disableHooks = true;
                return;
            }
            CompatibilityInfo compatInfo = (CompatibilityInfo) getObjectField(bindData, "compatInfo");
            if (appInfo.sourceDir == null) {
                return;
            }

            setObjectField(activityThread, "mBoundApplication", bindData);
            loadedPackagesInProcess.add(reportedPackageName);
            LoadedApk loadedApk = activityThread.getPackageInfoNoCheck(appInfo, compatInfo);

//            replaceParentClassLoader(loadedApk.getClassLoader());

            XC_LoadPackage.LoadPackageParam lpparam = new XC_LoadPackage.LoadPackageParam(XposedBridge.sLoadedPackageCallbacks);
            lpparam.packageName = reportedPackageName;
            lpparam.processName = (String) getObjectField(bindData, "processName");
            lpparam.classLoader = loadedApk.getClassLoader();
            lpparam.appInfo = appInfo;
            lpparam.isFirstApplication = true;
            XC_LoadPackage.callAll(lpparam);

            if (reportedPackageName.equals(INSTALLER_PACKAGE_NAME)) {
                XposedInstallerHooker.hookXposedInstaller(lpparam.classLoader);
            }
            if (reportedPackageName.equals(BLACK_LIST_PACKAGE_NAME)) {
                XposedBlackListHooker.hook(lpparam.classLoader);
            }

        } catch (Throwable t) {
            logE("error when hooking bindApp", t);
        }
    }
}