#!/bin/sh

BIN="bin/sdp"
if [ -f $BIN ]; then
    exec $BIN "$@"
else
    echo "Seismic Data Playback not build, read INSTALL first"
fi
