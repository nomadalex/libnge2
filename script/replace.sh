#! /bin/bash

temp=temp.tmp

do_sed () {
	sed $1 $2 >$temp
	mv -f $temp $2
}

do_change() {
	do_sed 's/uint8_t/t____001_____t/g' $1
	do_sed 's/uint16_t/t____002_____t/g' $1
	do_sed 's/uint32_t/t____003_____t/g' $1

	do_sed 's/int8_t/t____004_____t/g' $1
	do_sed 's/int16_t/t____005_____t/g' $1
	do_sed 's/int32_t/t____006_____t/g' $1

	do_sed 's/uint8/t____001_____t/g' $1
	do_sed 's/uint16/t____002_____t/g' $1
	do_sed 's/uint32/t____003_____t/g' $1

	do_sed 's/sint8/t____004_____t/g' $1
	do_sed 's/sint16/t____005_____t/g' $1
	do_sed 's/sint32/t____006_____t/g' $1

	do_sed 's/int8/t____004_____t/g' $1
	do_sed 's/int16/t____005_____t/g' $1
	do_sed 's/int32/t____006_____t/g' $1

	do_sed 's/t____001_____t/uint8_t/g' $1
	do_sed 's/t____002_____t/uint16_t/g' $1
	do_sed 's/t____003_____t/uint32_t/g' $1

	do_sed 's/t____004_____t/int8_t/g' $1
	do_sed 's/t____005_____t/int16_t/g' $1
	do_sed 's/t____006_____t/int32_t/g' $1
}

get_ext() {
	local filename extension
	filename=`basename "$1"`
	extension="${filename##*.}"
	echo $extension
}

do_dir() {
	local ext
	ls $1| while read f; do
	  if [ -f $1/$f ]; then
		  ext="`get_ext $f`"

		  if [[ "$ext" == "c" || "$ext" == "cpp" || "$ext" == "h" || "$ext" == "hpp" ]]; then
			  do_change $1/$f
		  fi
	  fi
	  if [ -d $1/$f ]; then
		  do_dir $1/$f
	  fi
	done
}

do_dir include
do_dir src
do_dir dirent
do_dir hge
do_dir movie
do_dir net
