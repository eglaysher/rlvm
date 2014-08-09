#!/bin/bash

# Runs all paths of CLANNAD; the result should be a save file with everything
# unlocked.
if [ ! -n "$1" ]
then
  echo "Usage: `basename $0` <path to CLANNAD directory>"
  exit 65
fi
GAMEDIR=$1

rm -Rf ~/.rlvm/KEY_CLANNAD/
mkdir -p CLANNAD.log

function runPath() {
  SCRIPT=$1
  LOG=`echo $SCRIPT | sed s/\.lua/\.log/g;`

  echo "Running $SCRIPT..."
  time build/lua_rlvm --count-undefined test/CLANNAD/$SCRIPT $GAMEDIR > CLANNAD.log/$LOG 2>&1
}

# The School Life Section
runPath "Misae_Sagara.lua"
runPath "Tomoyo_Sakagami.lua"
runPath "Yukine_Miyazawa.lua"
runPath "Kyou_Fujibayashi.lua"
runPath "Ryou_Fujibayashi.lua"
runPath "Kappei_Hiiragi.lua"
runPath "Mei_Sunohara.lua"
runPath "Kotomi.lua"
runPath "Fuko_Ibuki_True.lua"
# Do I need Fuko's Good ending for the after story? Might remove for speed...
runPath "Fuko_Ibuki_Good.lua"
runPath "Toshio_Koumura.lua"
runPath "Nagisa_Furukawa.lua"

# The After Story
runPath "After_Story_Part_I.lua"
runPath "After_Story_Part_II.lua"
runPath "After_Story_Part_III.lua"
