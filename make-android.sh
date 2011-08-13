#!/bin/sh

if [ "x$1" = "x--help" ]; then
	cat <<EOF
Usage:
./make-android.sh [BuildType [ArmTarget [FORCE] ] ]
EOF
	exit 0
fi

dir=`pwd`
if [ "x$1" = "x" ]; then
	type=Debug
else
	type="$1"
fi

if [ "x$2" = "x" ]; then
	target=armeabi
else
	target="$2"
fi

if [ "x$3" != "x" ]; then
	rm -rf build/android/$type/$target
fi

if [ ! -d build/android/$type/$target ]; then
	mkdir -p build/android/$type/$target
fi
cd build/android/$type/$target
cmake -DBUILD_FOR=Android -DCMAKE_BUILD_TYPE=$type -DARM_TARGET=$target $dir
make
