#!/bin/bash

find src -name "*.hpp" | xargs cpplint.py --filter=-build/include_order,-runtime/references,-readability/streams,-readability/todo,-whitespace/todo
find src -name "*.cpp" | xargs cpplint.py --filter=-build/include_order,-runtime/references,-readability/streams,-readability/todo,-whitespace/todo
