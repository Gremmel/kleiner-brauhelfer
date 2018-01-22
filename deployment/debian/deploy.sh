#!/bin/sh

if [ "$#" -ne 2 ] || ! [ -f "$1" ] || ! [ -d "$2" ]; then
  echo "Usage: $0 <path_to_kleiner-brauhelfer_executable> <path_to_qt_bin>" >&2
  echo "Example: $0 ./build/bin/kleiner-brauhelfer /opt/Qt/5.10.0/gcc_64/bin/" >&2
  exit 1
fi

BASE_DIR="$(dirname $0)"
APP_PATH="$1"
QT_DIR="$2"
DEPLOY_DIR="$BASE_DIR/deploy"
SOURCE_DIR="$BASE_DIR/../../source"

# Path to the .pro-file
PRO="${SOURCE_DIR}/brauhelfer.pro"

echo "* Extracting version numbers from .pro file:"
VERSION=`grep "${PRO}" -e "^ *VERSION *=" | tr -d '[:space:]' | cut -d= -f2` || exit 1

mkdir -p "$DEPLOY_DIR/usr/local/bin/kleiner-brauhelfer"
cp "$APP_PATH" "$DEPLOY_DIR/usr/local/bin/kleiner-brauhelfer"

mkdir -p "$DEPLOY_DIR/usr/local/bin/kleiner-brauhelfer/languages"
cp "$SOURCE_DIR/languages/"*.qm "$DEPLOY_DIR/usr/local/bin/kleiner-brauhelfer/languages"
cp "$SOURCE_DIR/languages/"*.png "$DEPLOY_DIR/usr/local/bin/kleiner-brauhelfer/languages"

"$QT_DIR/lconvert" -o "$DEPLOY_DIR/usr/local/bin/kleiner-brauhelfer/languages/qt_de.qm" "$QT_DIR/../translations/qtbase_de.qm"

mkdir -p "$DEPLOY_DIR/usr/share/pixmaps"
cp "$SOURCE_DIR/res/logo.svg" "$DEPLOY_DIR/usr/share/pixmaps/kleiner-brauhelfer.svg"

mkdir -p "$DEPLOY_DIR/usr/share/applications"
cp "$BASE_DIR/kleiner-brauhelfer.desktop" "$DEPLOY_DIR/usr/share/applications"

mkdir -p "$DEPLOY_DIR/DEBIAN"
cp "$BASE_DIR/control" "$DEPLOY_DIR/DEBIAN"

# nano "$DEPLOY_DIR/DEBIAN/control" #### FIXME

dpkg-deb --build "$DEPLOY_DIR"

TARVERSION=`echo "${VERSION}" | tr '.' '_'`
TAR="kb_linux_v${TARVERSION}.zip"

tar -zcvf "$BASE_DIR/${ZIP}" -C "$DEPLOY_DIR/usr/local/bin" kleiner-brauhelfer
