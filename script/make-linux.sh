#!/bin/sh

if [ "x$1" = "x--help" ]; then
	cat <<EOF
Usage:
./make-linux.sh [BuildType [FORCE] ]
EOF
	exit 0
fi

dir=`pwd`
if [ "x$1" = "x" ]; then
	type=Debug
else
	type="$1"
fi

tarDir=build/linux/$type

if [ "x$2" != "x" ]; then
	rm -rf $tarDir
fi

if [ ! -d $tarDir ]; then
	mkdir -p $tarDir
fi
cd $tarDir
cmake -DBUILD_FOR=Normal -DCMAKE_BUILD_TYPE=$type $dir
make
