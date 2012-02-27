#!/bin/sh

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

cd samples

rm -f res.7z

$_7z_cmd a res.7z res
