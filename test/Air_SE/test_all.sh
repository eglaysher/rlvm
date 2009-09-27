#!/bin/bash

# Runs all paths of AIR; the result should be a save file with everything
# unlocked.

rm -Rf ~/.rlvm/KEY_AIR_SE/
mkdir -p AIR.log

function runPath() {
  SCRIPT=$1
  LOG=`echo $SCRIPT | sed s/\.lua/\.log/g;`

  echo "Running $SCRIPT..."
  time build/luaRlvm --count-undefined test/Air_SE/$SCRIPT /storage/GameImages/AIR/ > AIR.log/$LOG 2>&1
}

runPath "Dream_Kano.lua"
runPath "Dream_Minagi.lua"
runPath "Dream_Misuzu.lua"
runPath "Summer.lua"
runPath "Air.lua"

