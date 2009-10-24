#!/bin/bash

# Runs all paths of Princess Bride!; the result should be a save file with
# everything unlocked.

rm -Rf ~/.rlvm/130CM_PRINCESS_BRIDE_SE/
mkdir -p P_BRIDE.log

function runPath() {
  SCRIPT=$1
  LOG=`echo $SCRIPT | sed s/\.lua/\.log/g;`

  echo "Running $SCRIPT..."
  time build/luaRlvm --count-undefined test/P_BRIDE/$SCRIPT /storage/GameImages/P_BRIDE > P_BRIDE.log/$LOG 2>&1
}

runPath "Kiyomi.lua"
runPath "Aimi.lua"
runPath "Kao.lua"
runPath "Haruna.lua"
runPath "Ciel.lua"
