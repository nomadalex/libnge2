#!/bin/bash

rm -f AndroidDepends.7z
rm -rf AndroidDepends
wget https://github.com/downloads/ifreedom/libnge2/AndroidDepends.7z --no-check-certificate
7za x AndroidDepends.7z
