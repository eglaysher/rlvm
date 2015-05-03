#!/bin/bash

# Runs all paths of AIR; the result should be a save file with everything
# unlocked.
if [ ! -n "$1" ]
then
  echo "Usage: `basename $0` <path to AIR directory>"
  exit 65
fi
GAMEDIR=$1

rm -Rf ~/.rlvm/KEY_AIR_SE/
mkdir -p AIR.log

function runPath() {
  SCRIPT=$1
  LOG=`echo $SCRIPT | sed s/\.lua/\.log/g;`

  echo "Running $SCRIPT..."
  time build/lua_rlvm --count-undefined test/Air_SE/$SCRIPT $GAMEDIR > AIR.log/$LOG 2>&1
}

runPath "Dream_Kano.lua"
runPath "Dream_Minagi.lua"
runPath "Dream_Misuzu.lua"
runPath "Summer.lua"
runPath "Air.lua"

