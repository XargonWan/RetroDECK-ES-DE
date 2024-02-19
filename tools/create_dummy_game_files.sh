#!/usr/bin/bash
#  SPDX-License-Identifier: MIT
#
#  ES-DE
#  create_dummy_game_files.sh
#
# This script generates dummy files for each system in the ROM directory and is intended
# primarily for theme testing purposes. You need to run it from a ROM directory previously
# generated by ES-DE as the systems.txt and systeminfo.txt files are used to create the dummy
# files.
#
# This script is only intended to be used on Linux systems.
#

if [ ! -f ./systems.txt ]; then
  echo "Can't find the systems.txt file, you need to run this script from your ROM directory."
  exit
fi

for folder in $(cat systems.txt | cut -f1 -d":" | sed s/"(custom system)"/""/g); do
  echo Creating dummy file for system ${folder}
  touch ${folder}/dummy$(grep "^\." ${folder}/systeminfo.txt | cut -f1 -d " ")
done
