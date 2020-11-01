#!/bin/bash
MODULE_NAME=$1
if [ "$MODULE_NAME" == "" ]; then
  echo "Usage: sh build.sh <module name> [<version name>]"
  exit 1
fi

if [ ! -d "$MODULE_NAME" ]; then
  echo "$MODULE_NAME not exists"
  exit 1
fi

VERSION=$2
[[ "$VERSION" == "" ]] && VERSION=v1

ZIP_NAME_PREFIX=$3

# create tmp dir
TMP_DIR=build/zip
TMP_DIR_MAGISK=$TMP_DIR/magisk

rm -rf $TMP_DIR
mkdir -p $TMP_DIR

# run custom script
if [ -f $MODULE_NAME/build-module.sh ]; then
  source $MODULE_NAME/build-module.sh
  copy_files
fi

# zip
mkdir -p $MODULE_NAME/release
ZIP_NAME=magisk-$ZIP_NAME_PREFIX-"$VERSION".zip
rm -f $MODULE_NAME/release/$ZIP_NAME
rm -f $TMP_DIR_MAGISK/$ZIP_NAME
(cd $TMP_DIR_MAGISK; zip -r $ZIP_NAME * > /dev/null)
mv $TMP_DIR_MAGISK/$ZIP_NAME $MODULE_NAME/release/$ZIP_NAME

# clean tmp dir
rm -rf $TMP_DIR
