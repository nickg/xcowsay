#!/bin/bash

set -e

echo Normal mode
$BUILD_DIR/src/xcowsay Hello World

echo Left bubble
$BUILD_DIR/src/xcowsay --left Hello World

echo Thought bubble
$BUILD_DIR/src/xcowsay --think Hello World

echo Dream
$BUILD_DIR/src/xcowsay --dream $SRC_DIR/cow_small.png -t 2
