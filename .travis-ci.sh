#!/bin/bash

# This script is triggered from the script section of .travis.yml
# It runs the appropriate commands depending on the task requested.

# Otherwise compile and check as normal
qmake QMAKE_CXX=$CXX && make && ./unittest.sh
