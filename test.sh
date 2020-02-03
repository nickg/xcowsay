#!/bin/bash

set -e

echo Normal mode
$BUILD_DIR/src/xcowsay Hello World

echo Left bubble
$BUILD_DIR/src/xcowsay --left Hello World

echo Thought bubble
$BUILD_DIR/src/xcowsay --think Hello World

echo Left thought bubble
$BUILD_DIR/src/xcowsay --think Hello World --left

echo Dream
$BUILD_DIR/src/xcowsay --dream $SRC_DIR/cow_small.png -t 2

echo Unicode and Pango attributes
$BUILD_DIR/src/xcowsay "<b>你好</b> <i>world</i>"

echo Daemon mode
$BUILD_DIR/src/xcowsay --daemon --debug &
pid=$!
echo "PID is $pid"
sleep 0.5

$BUILD_DIR/src/xcowsay Hello World -t 100
echo "Sleep for one second"
sleep 1

kill $pid
wait
