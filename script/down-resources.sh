#!/bin/bash
### down-resources.sh --- download res.zip and extract it to samples dir.

wget https://github.com/downloads/ifreedom/libnge2/res.7z --no-check-certificate
mv res.7z samples/
cd samples/
7z x -y res.7z
