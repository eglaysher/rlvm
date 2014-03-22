#!/bin/bash

find src -type f \( -name "*.h" -o -name "*.cc" \) | grep -v "gettext.h" | grep -v "SDLMain\." | xargs cpplint.py --filter=-build/include_order,-runtime/references,-readability/streams,-readability/casting,-runtime/rtti,-runtime/int

