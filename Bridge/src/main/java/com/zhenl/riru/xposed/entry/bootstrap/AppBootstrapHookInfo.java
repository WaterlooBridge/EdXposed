package com.zhenl.riru.xposed.entry.bootstrap;

import com.zhenl.riru.common.KeepMembers;
import com.zhenl.riru.xposed.entry.hooker.HandleBindAppHooker;
import com.zhenl.riru.xposed.entry.hooker.LoadedApkConstructorHooker;
import com.zhenl.riru.xposed.entry.hooker.OnePlusWorkAroundHooker;

public class AppBootstrapHookInfo implements KeepMembers {
    public static String[] hookItemNames = {
            HandleBindAppHooker.class.getName(),
            LoadedApkConstructorHooker.class.getName(),
            OnePlusWorkAroundHooker.class.getName()
    };
}
