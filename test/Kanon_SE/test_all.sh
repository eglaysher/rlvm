#!/bin/bash

# Runs all paths of Kanon; the result should be a save file with everything
# unlocked.

rm -Rf ~/.rlvm/KEY_KANON_SE_ALL/
mkdir -p KANON.log

function runPath() {
  SCRIPT=$1
  LOG=`echo $SCRIPT | sed s/\.lua/\.log/g;`

  echo "Running $SCRIPT..."
  time build/luaRlvm --count-undefined test/Kanon_SE/$SCRIPT > KANON.log/$LOG 2>&1
}

runPath "Mai.lua"
runPath "Shiori.lua"
runPath "Makoto.lua"
runPath "Nayuki.lua"
runPath "Ayu.lua"
