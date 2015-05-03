#!/bin/bash

# Runs all paths of Princess Bride!; the result should be a save file with
# everything unlocked.
if [ ! -n "$1" ]
then
  echo "Usage: `basename $0` <path to P_BRIDE directory>"
  exit 65
fi
GAMEDIR=$1

rm -Rf ~/.rlvm/130CM_PRINCESS_BRIDE_SE/
mkdir -p P_BRIDE.log

function runPath() {
  SCRIPT=$1
  LOG=`echo $SCRIPT | sed s/\.lua/\.log/g;`

  echo "Running $SCRIPT..."
  time build/lua_rlvm --count-undefined test/P_BRIDE/$SCRIPT $GAMEDIR > P_BRIDE.log/$LOG 2>&1
}

runPath "Kiyomi.lua"
runPath "Aimi.lua"
runPath "Kao.lua"
runPath "Haruna.lua"
runPath "Ciel.lua"
