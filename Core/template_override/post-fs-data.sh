#!/system/bin/sh
# Please don't hardcode /magisk/modname/... ; instead, please use $MODDIR/...
# This will make your scripts compatible even if Magisk change its mount point in the future
MODDIR=${0%/*}

# This script will be executed in post-fs-data mode
# More info in the main Magisk thread

# EdXposed Version
edxp_ver="1.1.1_beta"

# necessary for using mmap in system_server process
supolicy --live "allow system_server system_server process {execmem}"
supolicy --live "allow system_server system_server memprotect {mmap_zero}"

# read configs set and module apk file in zygote
supolicy --live "allow zygote app_data_file * *"
supolicy --live "allow zygote apk_data_file * *"

# beginning of Log Catcher
android_sdk=`getprop ro.build.version.sdk`
path=/data/local/tmp/de.robv.android.xposed.installer/log
file=${path}/error.log
build_desc=`getprop ro.build.description`
product=`getprop ro.build.product`
manufacturer=`getprop ro.product.manufacturer`
brand=`getprop ro.product.brand`
fingerprint=`getprop ro.build.fingerprint`
arch=`getprop ro.product.cpu.abi`
device=`getprop ro.product.device`
android=`getprop ro.build.version.release`
build=`getprop ro.build.id`
mkdir -p ${path}
rm -rf ${file}
touch ${file}
chmod 755 ${file}
echo "--------- beginning of head">>${file}
echo "EdXposed Log">>${file}
echo "Powered by Log Catcher">>${file}
echo "--------- beginning of system info">>${file}
echo "Android version: ${android}">>${file}
echo "Android sdk: ${android_sdk}">>${file}
echo "Android build: ${build}">>${file}
echo "Fingerprint: ${fingerprint}">>${file}
echo "ROM build description: ${build_desc}">>${file}
echo "EdXposed Version: ${edxp_ver}">>${file}
echo "Architecture: ${arch}">>${file}
echo "Device: ${device}">>${file}
echo "Manufacturer: ${manufacturer}">>${file}
echo "Brand: ${brand}">>${file}
echo "Product: ${product}">>${file}
logcat -f ${file} *:S EdXposed-Bridge:V &
