package com.zhenl.riru.xposed.entry.bootstrap;

import com.zhenl.riru.common.KeepMembers;
import com.zhenl.riru.xposed.entry.hooker.OnePlusWorkAroundHooker;

public class WorkAroundHookInfo implements KeepMembers {
    public static String[] hookItemNames = {
            OnePlusWorkAroundHooker.class.getName()
    };
}
