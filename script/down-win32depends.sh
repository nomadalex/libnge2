#!/bin/bash

if [ $1 = "clean" ]; then
	CLEAN=1
fi

test_cmd() {
	for cmd in $@; do
		which $cmd 1>/dev/null 2>&1
		if [ $? = 0 ]; then
			echo $cmd
			break
		fi
	done
}

_7z_cmd="`test_cmd 7z 7zr 7za`"

(( CLEAN )) && rm -f Win32Depends.7z
(( CLEAN )) && rm -rf Win32Depends

wget https://github.com/downloads/ifreedom/libnge2/Win32Depends.7z --no-check-certificate
$_7z_cmd x -y Win32Depends.7z
