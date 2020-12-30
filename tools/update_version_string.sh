#!/usr/bin/bash
#
# update_version_string.sh
# Updates the version string for EmulationStation Desktop Edition.
#
# This script takes as arguments the major, minor and patch numbers as well as an optional
# alphanumeric suffix and updates all the necessary files to indicate a new software version.
# The script has to be run from within the tools directory.
#
# Example use:
# ./update_version_string.sh 1 0 0 beta1
#
# The following files are updated by this script:
# es-app/CMakeLists.txt
# es-app/src/EmulationStation.h
# es-app/assets/EmulationStation-DE_Info.plist
# es-app/assets/emulationstation.desktop
#
# This script is intended to only be used on Linux systems.
#
# Leon Styhre
# 2020-12-30
#

if [ ! -f ../es-app/CMakeLists.txt ]; then
  echo "You need to run this script from within the tools directory."
  exit
fi

if [ $# -ne 3 ] && [ $# -ne 4 ]; then
  echo "Usage: ./update_version_string.sh <major version> <minor version> <patch version> [<suffix>]"
  echo "For example:"
  echo "./update_version_string.sh 1 0 0 beta1"
  exit
fi

if [ $# -eq 4 ]; then
  SUFFIX=-$4
else
  SUFFIX=""
fi

TEMPFILE=update_version_string.tmp

##### CMakeLists.txt

MODIFYFILE=../es-app/CMakeLists.txt
MODIFYSTRING=$(grep "set(CPACK_PACKAGE_VERSION" $MODIFYFILE)
NEWSTRING="set(CPACK_PACKAGE_VERSION \"${1}.${2}.${3}${SUFFIX}\")"

cat $MODIFYFILE | sed s/"${MODIFYSTRING}"/"${NEWSTRING}"/ > $TEMPFILE
mv $TEMPFILE $MODIFYFILE

##### EmulationStation.h

MODIFYFILE=../es-app/src/EmulationStation.h
MODIFYSTRING=$(grep "PROGRAM_VERSION_STRING" $MODIFYFILE)
NEWSTRING="#define PROGRAM_VERSION_STRING \"${1}.${2}.${3}${SUFFIX}\""

cat $MODIFYFILE | sed s/"${MODIFYSTRING}"/"${NEWSTRING}"/ > $TEMPFILE
mv $TEMPFILE $MODIFYFILE

MODIFYSTRING=$(grep "RESOURCE_VERSION_STRING" $MODIFYFILE)
MODIFYSTRING=$(echo $MODIFYSTRING | sed s/"...$"//)
NEWSTRING="#define RESOURCE_VERSION_STRING \"${1},${2},${3}"

cat $MODIFYFILE | sed s/"${MODIFYSTRING}"/"${NEWSTRING}"/ > $TEMPFILE
mv $TEMPFILE $MODIFYFILE

##### EmulationStation-DE_Info.plist

MODIFYFILE=../es-app/assets/EmulationStation-DE_Info.plist
MODIFYSTRING=$(grep "<string>EmulationStation Desktop Edition " $MODIFYFILE)
OLDVERSION=$(echo $MODIFYSTRING | cut -f4 -d" " | sed s/".........$"//)
MODIFYSTRING=$(echo $MODIFYSTRING | sed s/".........$"//)
NEWSTRING="<string>EmulationStation Desktop Edition ${1}.${2}.${3}"

cat $MODIFYFILE | sed s/"${MODIFYSTRING}"/"${NEWSTRING}"/ > $TEMPFILE
mv $TEMPFILE $MODIFYFILE

MODIFYSTRING=$(grep "<string>${OLDVERSION}" $MODIFYFILE)
MODIFYSTRING=$(echo $MODIFYSTRING | sed s/".........$"//)
NEWSTRING="<string>${1}.${2}.${3}${SUFFIX}"

cat $MODIFYFILE | sed s/"${MODIFYSTRING}"/"${NEWSTRING}"/ > $TEMPFILE
mv $TEMPFILE $MODIFYFILE

##### emulationstation.desktop

MODIFYFILE=../es-app/assets/emulationstation.desktop
MODIFYSTRING=$(grep "Version=" $MODIFYFILE)
NEWSTRING="Version=${1}.${2}.${3}${SUFFIX}"

cat $MODIFYFILE | sed s/"${MODIFYSTRING}"/"${NEWSTRING}"/ > $TEMPFILE
mv $TEMPFILE $MODIFYFILE

echo "Done updating, don't forget to run generate_man_page.sh once the binary has been compiled with the new version string."
