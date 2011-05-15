#!/bin/bash
### down-resources.sh --- download res.zip and extract it to samples dir.

cd samples/
wget https://github.com/downloads/ifreedom/libnge2/res.zip
unzip res.zip
