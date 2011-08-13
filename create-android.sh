#!/bin/sh

if [ -z "$1" ]; then
	echo "Please give me project root dir!";exit 1;
fi

targetDir="$1"
armTarget="$2"
if [ "x$armTarget" = "x" ];then
	armTarget="armeabi"
fi

dir=`pwd`

if [ ! -d $targetDir/jni ]; then
	mkdir -p $targetDir/jni
fi
if [ ! -d $targetDir/src/org/libnge/nge2 ]; then
	mkdir -p $targetDir/src/org/libnge/nge2
fi
if [ ! -d $targetDir/jni/libnge2 ]; then
	mkdir -p $targetDir/jni/libnge2
fi

cp -f $dir/Android/Android.mk $targetDir/jni/

cp -f $dir/src/android/NGE2.java $targetDir/src/org/libnge/nge2/
cp -f $dir/coolaudio/LibCoolAudio.java $targetDir/src/org/libnge/nge2/

ln -sf $dir/AndroidDepends $targetDir/jni/depends

cp -f $dir/coolaudio/audio_interface.h $targetDir/jni/$targetDir/jni/
ln -sf $dir/include $targetDir/jni/libnge2/
ln -sf $dir/Android/libs/$armTarget $targetDir/jni/libnge2/libs

cp -f $dir/Android/AndroidManifest.xml.in $targetDir/AndroidManifest.xml
cp -f $dir/Android/default.properties $targetDir/default.properties
cp -f $dir/Android/project_create.sh.in $targetDir/project_create.sh
cp -f $dir/Android/strings.xml.in $targetDir/res/values/strings.xml
