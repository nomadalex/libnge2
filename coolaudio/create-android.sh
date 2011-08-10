#!/bin/sh

if [ ! -z "$1" ]; then
	targetDir="$1"
else
	targetDir=../Android
fi

mkdir -p $targetDir/jni/coolaudio $targetDir/src/org/libnge/nge2
cp -f Android.mk audio_android.c audio_interface.h $targetDir/jni/coolaudio/
cp -f LibCoolAudio.java $targetDir/src/org/libnge/nge2/
