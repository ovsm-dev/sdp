#!/bin/bash
# This is a sample script for compiling and running sdp!
mkdir -p build
#rm -rf build/*
cd build/
cmake .. && make && make install
echo "-- Launching application..."

if [[ "$OSTYPE" == "linux-gnu" ]]; then

	BIN="bin/sdp"
	if [ -f $BIN ]; then
		FILESIZE=$(du -hs ${BIN} | sed 's/\([0-9]*\)\(.*\)/\1/')
		echo "-- Binary size: ${FILESIZE}Kb"
    		./bin/sdp --debug
    		echo "-- Application retcode: "$?;
	else
    		echo "-- Seismic Data Playback not build, read INSTALL first"
	fi

elif [[ "$OSTYPE" == "darwin"* ]]; then

	DIR=`pwd`
	BINARY="bin/sdp.app/Contents/MacOS/sdp"
	APP="$DIR/bin/sdp.app"
	if [ -d $APP ]; then
		APPFILESIZE=$(du -hs ${APP} | sed 's/\([0-9]*\)\(.*\)/\1/')
		FILESIZE=$(du -hs ${BIN} | sed 's/\([0-9]*\)\(.*\)/\1/')
		echo "-- App size is ${APPFILESIZE}Kb and binary size is ${FILESIZE}Kb"
#		open -a $APP
		./bin/sdp.app/Contents/MacOS/sdp --debug
		echo "-- Application retcode: "$?;
	else
		echo "-- SeismicDataPlayback not build for Darwin"
	fi
fi

exit 0;
