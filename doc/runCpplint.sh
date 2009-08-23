#!/bin/bash

find src -name "*.hpp" | grep -v libReallive | xargs cpplint.py --filter=-build/include_order,-runtime/references,-readability/streams,-readability/todo,-whitespace/todo,-build/namespaces,-readability/casting,-runtime/rtti
find src -name "*.cpp" | grep -v libReallive | xargs cpplint.py --filter=-build/include_order,-runtime/references,-readability/streams,-readability/todo,-whitespace/todo,-build/namespaces,-readability/casting,-runtime/rtti
