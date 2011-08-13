#!/bin/sh

if [ "x$1" = "x--help" ]; then
	cat <<EOF
Usage:
./make-psp.sh [BuildType [FORCE] ]
EOF
	exit 0
fi

dir=`pwd`
if [ "x$1" = "x" ]; then
	type=Debug
else
	type="$1"
fi

tarDir=build/psp/$type

if [ "x$2" != "x" ]; then
	rm -rf $tarDir
fi

if [ ! -d $tarDir ]; then
	mkdir -p $tarDir
fi
cd $tarDir
cmake -DBUILD_FOR=Psp -DCMAKE_BUILD_TYPE=$type $dir
make
