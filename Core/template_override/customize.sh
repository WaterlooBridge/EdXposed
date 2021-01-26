SKIPUNZIP=1
NO_PERSIST=true
PERSIST="
/persist
/mnt/vendor/persist
"

fail() {
    echo "$1"
    exit 1
}

check_architecture() {
    if [[ "$ARCH" != "arm" && "$ARCH" != "arm64" ]]; then
        ui_print "- Unsupported platform: $ARCH"
        exit 1
    else
        ui_print "- Device platform: $ARCH"
    fi
}

check_persist() {
    for TARGET in ${PERSIST}; do
        if [[ -d ${TARGET} ]]; then
            NO_PERSIST=false
        fi
    done
}

check_persist
check_architecture

ui_print "- Extracting module files"
unzip -o "${ZIPFILE}" module.prop post-fs-data.sh sepolicy.rule system.prop 'system/*' -d "${MODPATH}" >&2

if [[ "$IS64BIT" = false ]]; then
    ui_print "- Removing 64-bit libraries"
    rm -rf "$MODPATH/system/lib64"
fi

if [[ "${NO_PERSIST}" == true ]]; then
    ui_print "- Persist not detected, remove SEPolicy rule"
	echo "- Mount: persist:" >&2
	mount | grep persist >&2
    rm ${MODPATH}/sepolicy.rule
fi

ui_print "- Extracting extra files"
unzip -o "$ZIPFILE" 'data/*' -d "$MODPATH" >&2

TARGET="/data/adb/riru/modules"

# TODO: do not overwrite if file exists
[[ -d "$TARGET" ]] || mkdir -p "$TARGET" || fail "- Can't mkdir -p $TARGET"
cp -af "$MODPATH$TARGET/." "$TARGET" || fail "- Can't cp -af $MODPATH$TARGET/. $TARGET"

rm -rf "$MODPATH/data" 2>/dev/null

mkdir -p $MODPATH/system/etc
cp -af /system/etc/public.libraries.txt $MODPATH/system/etc/
echo "libwhale.so">>$MODPATH/system/etc/public.libraries.txt

ui_print "- Files copied"

set_perm_recursive "${MODPATH}" 0 0 0755 0644