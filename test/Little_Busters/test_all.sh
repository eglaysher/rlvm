#!/bin/bash

# Runs all paths of Little Busters; the result should be a save file with
# everything unlocked.
if [ ! -n "$1" ]
then
  echo "Usage: `basename $0` <path to Little Busters directory>"
  exit 65
fi
GAMEDIR=$1

# TODO: Clean up directory how? Not in all roman letters.
#rm -Rf ~/.rlvm/KEY_KANON_SE_ALL/
mkdir -p LB.log

function runPath() {
  SCRIPT=$1
  LOG=`echo $SCRIPT | sed s/\.lua/\.log/g;`

  echo "Running $SCRIPT..."
  time build/lua_rlvm --count-undefined test/Little_Busters/$SCRIPT $GAMEDIR > LB.log/$LOG 2>&1
}

# r1 is the common route.
runPath "Yuiko.lua" # r2.
runPath "Rin-bad.lua" # r3.
runPath "Komari.lua" # r4
runPath "Mio.lua" # r5
runPath "Kudryavka.lua" # r6
runPath "Saigusa.lua" # r7
runPath "Rin.lua"  # r8
