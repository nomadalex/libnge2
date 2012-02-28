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

wget -O Win32Depends2.7z https://open.ge.tt/1/files/7Th6r3E/0/blob --no-check-certificate
$_7z_cmd x -y Win32Depends.7z
