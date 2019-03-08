package com.zhenl.riru.xposed.entry.bootstrap;

import com.zhenl.riru.common.KeepMembers;
import com.zhenl.riru.xposed.entry.hooker.StartBootstrapServicesHooker;

public class SysInnerHookInfo implements KeepMembers {
    public static String[] hookItemNames = {
            StartBootstrapServicesHooker.class.getName()
    };
}
