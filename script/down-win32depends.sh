#!/bin/bash

rm -f Win32Depends.7z
rm -rf Win32Depends
wget https://github.com/downloads/ifreedom/libnge2/Win32Depends.7z --no-check-certificate
7za x Win32Depends.7z
