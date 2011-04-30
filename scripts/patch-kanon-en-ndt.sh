#!/bin/bash
#
###############################################################################
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
#
###############################################################################
#
# NDT's Kanon patch: Crazy cases and messed up spaces.
#
# So we have a problem: NDT is distributing their patch as a zip file designed
# to be unextracted on top of the Kanon install and the cases of the filenames
# in the archive do not match Linux or Mac. For example:
#
#   ./G00/ayu_02.g00 (in NDT's archive)
#   ./g00/AYU_02.g00 (in the local install)
#
# I'm not sure what the cause of this is (i.e. is this a difference between how
# window vs Mac/Linux read ISO filesystems?), but this causes problems! Windows
# is case insensitive (the two paths above are the same) so I can see why NDT
# wouldn't notice this and might not even think it's a bug. The fun begins with
# Mac OSX: Filesystems are case insensitive by default. On a case insensitive
# filesystem, you can make two files with different names when compared
# case-sensitively and the same name when compared case-insensitively. Which
# version the OS returns appears to be undefined!
#
# Linux filesystems are case sensitive, though rlvm has specific code to make
# file searching case insensitive because RealLive developers rarely have the
# same caseing as the filesystem in their code. It would similarly be confused
# by two files with the same case insensitive name.
#
# So we need a script to normalize the names. For each file in their "G00"
# directory, remove any files in the local Kanon install that have that name
# (insensitively compared), and then copy the new files on top.
#
# This is all complicated by the patch extracting to a directory with a space
# in the path, meaning I am now invoking `find` everywhere. :(

E_BADARGS=65

usage() {
  echo "Usage: `basename $0` </path/to/PatchFiles/> </path/to/KanonInstall>"
  echo ""
  echo "  Installs NDT's English Kanon patch on the Mac or Linux."
  echo ""
}

# Check that we have arguments.
if [ ! -n "$1" ] || [ ! -n "$2" ]
then
    usage
    exit $E_BADARGS
fi

# Check source root
SRCROOT="$1"
if [ ! -e "$SRCROOT/seen.txt" ]
then
    usage
    echo "  ERROR: First argument does not appear to be an unextracted NDT Patch."
    echo ""
    exit $E_BADARGS
fi

DESTROOT="$2"
if [ ! -e "$DESTROOT/Kanon.ico" ]
then
    usage
    echo "  ERROR: Second argument does not appear to be Kanon: Standard Edition."
    echo ""
    exit $E_BADARGS
fi

echo "Deleting untranslated SEEN.TXT from local Kanon install..."
find "$DESTROOT" -iname "seen.txt" -delete

echo "Copying SEEN files..."
# Spaces in filenames: ARRRRGGGHHHH!
find "$SRCROOT" -iname "seen.txt" | while read FILE
do
    cp "$FILE" "$DESTROOT"
done

echo "Deleting untranslated images from local Kanon install..."
find "$SRCROOT" -iname "*.g00" | while read FILE
do
    BASE=`basename "$FILE"`
    find $DESTROOT/g00/ -iname "$BASE" -delete
done

echo "Copying translated images into Kanon install..."
find "$SRCROOT/G00/" -iname "*.g00" | while read FILE
do
    cp "$FILE" "$DESTROOT/g00/"
done

echo "Copying other files..."
find "$SRCROOT" -iname "gameexe.ini" | while read FILE
do
    cp "$FILE" "$DESTROOT/Gameexe.ini"
done
cp "$SRCROOT/readme.txt" $DESTROOT/
cp "$SRCROOT/License.txt" $DESTROOT/
