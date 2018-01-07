#!/bin/sh

if [ "$#" -ne 2 ] || ! [ -d "$1" ] || ! [ -d "$2" ]; then
  echo "Usage: $0 <path_to_kleiner-brauhelfer_bundle> <path_to_qt_bin>" >&2
  echo "Example: $0 ./bin/kleiner-brauhelfer.app /opt/Qt/5.10.0/clang_64/bin" >&2
  exit 1
fi

# Project basedir
BASE_DIR="$(dirname $0)"

# Path to the created bundle
BUNDLE="$1"

# Path to Qt utilities
QT_DIR="$2"

# Path to the sources directory
SOURCES="${BASE_DIR}/../../source"

# Path to the .pro-file
PRO="${SOURCES}/brauhelfer.pro"

# Path to the deployment resources
RESOURCES="${BASE_DIR}"


###
### Extract version numbers from .pro file
###

echo "* Extracting version numbers from .pro file:"
VERSION=`grep "${PRO}" -e "^ *VERSION *=" | tr -d '[:space:]' | cut -d= -f2` || exit 1
VERSION_INT=`grep "${PRO}" -e "^ *VERSION_INT *=" | tr -d '[:space:]' | cut -d= -f2` || exit 1
echo "  - Version: '${VERSION}'"
echo "  - Internal version: '${VERSION_INT}'"

###
### Patch Info.plist in generated bundle
###

PLIST="${BUNDLE}/Contents/Info.plist"
echo "* Patching '${PLIST}'..."

# Remove QT advertising
/usr/libexec/PlistBuddy -c "Delete :NOTE" ${PLIST}

# Remove deprecated creator code for old MacOS
/usr/libexec/PlistBuddy -c "Delete :CFBundleSignature" ${PLIST}

# Remove deprecated get-info string
/usr/libexec/PlistBuddy -c "Delete :CFBundleGetInfoString" ${PLIST}

# Set bundle identifier instead of usieng default of 'yourcompany.com'
/usr/libexec/PlistBuddy -c "Set :CFBundleIdentifier 'com.github.gremmel.kleiner-brauhelfer'" ${PLIST}

# Add information for Finder
/usr/libexec/PlistBuddy -c "Delete :CFBundleDisplayName" ${PLIST}
/usr/libexec/PlistBuddy -c "Add :CFBundleDisplayName string 'kleiner-brauhelfer'" ${PLIST} || exit 1

/usr/libexec/PlistBuddy -c "Delete :CFBundleName" ${PLIST}
/usr/libexec/PlistBuddy -c "Add :CFBundleName string 'Kleiner Brauhelfer'" ${PLIST} || exit 1

/usr/libexec/PlistBuddy -c "Delete :CFBundleShortVersionString" ${PLIST}
/usr/libexec/PlistBuddy -c "Add :CFBundleShortVersionString string '${VERSION}'" ${PLIST} || exit 1

/usr/libexec/PlistBuddy -c "Delete :CFBundleVersion" ${PLIST}
/usr/libexec/PlistBuddy -c "Add :CFBundleVersion string '${VERSION_INT}'" ${PLIST} || exit 1

/usr/libexec/PlistBuddy -c "Delete :NSHumanReadableCopyright" ${PLIST}
/usr/libexec/PlistBuddy -c "Add :NSHumanReadableCopyright string 'Copyright © 2018 Gremmelsoft. All rights reserved.'" ${PLIST} || exit 1

###
### Copy resource files
###

echo "* Copying resource files..."
cp "${RESOURCES}/InfoPlist.strings" "${BUNDLE}/Contents/Resources" || exit 1

# TODO: duno how Qt on macOS does internationalization
# cp "${SOURCES}/languages/*" "${BUNDLE}/Contents/Resources" || exit 1

###
### Running QT deployment
###

echo "* Creating self-contained bundle..."
"${QT_DIR}/macdeployqt" ${BUNDLE} || exit 1

###
### Create distribution archive
###

echo "* Creating ZIP-archive for distribution..."
DIR="$(dirname ${BUNDLE})"
ZIPVERSION=`echo "${VERSION}" | tr '.' '_'`
ZIP="kb_macos_v${ZIPVERSION}.zip"
pushd "${DIR}" || exit 1
zip -r -o "${ZIP}" `basename ${BUNDLE}` || exit 1
popd
echo "  - Created archive: ${DIR}/${ZIP}"

echo "=> Deployment finished successfully"
