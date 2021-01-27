# EdXposed

A Riru module trying to provide a ART hooking framework (mainly for Android Pie) which delivers consistent APIs with the OG Xposed, leveraging Whale hooking framework.

## Supported versions

- Android Oreo (8.x)
- Android Pie (9.0)
- Android 10
- Android 11

## Credits 

- [Whale](https://github.com/asLody/whale): the core java hooking framework and inline hooking
- [Riru](https://github.com/RikkaApps/Riru): provides a way to inject codes into zygote process
- [XposedBridge](https://github.com/rovo89/XposedBridge): the OG xposed framework APIs

## Known issues

- resources hooking is not supported yet
- may not be compatible with all ART devices
- only a few Xposed modules has been tested for working
- file access services are not implemented yet, now simply use magiskpolicy to enable needed SELinux policies

## Build requirements

same as [Riru-Core's](https://github.com/RikkaApps/Riru/blob/master/README.md#build-requirements)
and zip binaries can be downloaded from [here](http://gnuwin32.sourceforge.net/packages/zip.htm)

## Build

1. run `:Bridge:makeAndCopyRelease` in Gradle window to build `edxposed.dex`
2. run `:Core:zipRelease` to build Magisk Riru module flashable zip file
3. find the flashable under `Core/release/`
4. flash the zip in recovery mode or in Magisk Manager

## Install

1. make sure Magisk v20.4 or higher is installed.
2. download [Riru-core](https://github.com/RikkaApps/Riru/releases) v22 or higher and install it in Magisk Manager or recovery.
3. download [EdXposed](https://github.com/WaterlooBridge/EdXposed/releases) and install it in Magisk Manager or recovery.
4. Install [Xposed Installer](https://github.com/DVDAndroid/XposedInstaller)
4. reboot.
5. have fun :)

## Get help

GitHub issues (recommend): [Issues](https://github.com/WaterlooBridge/EdXposed/issues/)
