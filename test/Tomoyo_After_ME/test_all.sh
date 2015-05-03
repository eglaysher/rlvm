#!/bin/bash

# Runs the single path through Tomoyo After, but do it in a script with the
# same syntax as all the other test_all.sh scripts.
if [ ! -n "$1" ]
then
  echo "Usage: `basename $0` <path to Tomoyo After ME directory>"
  exit 65
fi
GAMEDIR=$1

rm -Rf ~/.rlvm/KEY_智代アフター_ME_ALL/
mkdir -p Tomoyo_ME.log

echo "Running Tomoyo_After_ME.lua..."
time build/lua_rlvm --count-undefined test/Tomoyo_After_ME/Tomoyo_After_ME.lua $GAMEDIR > Tomoyo_ME.log/Tomoyo_After_ME.log 2>&1
