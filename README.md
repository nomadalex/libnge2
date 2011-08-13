NGE2
====

NGE2(Not a Game Engine的递归缩写)，NGE第一版原为制作中国象棋psp所写，第二版得到了更全面的强化，是PSP/Windows/iPhone/Linux/android开发的一个程序库，不是一个游戏引擎，设计思想是简单易用，用来做各个显示程序或者游戏引擎，NGE2追求的是性能，所以全部用的c函数写，而显示函数也分为各个性能要求，提供不同的显示函数。 NGE2的设计就是让开发人员不需要用PSP调试就能开发出PSP的应用程序。就是说在Windows/linux下调试，在PSP/iphone/android上运行。 大大降低了PSP/iphone/android调试程序浪费的时间。

特点:
---------
* 支持16位（RGBA5650 RGBA5551 RGBA4444）/32位(RGBA_8888)显示。
* 支持JPG,BMP(24,32位),PNG,TGA（24,32位）图片读取，并转化到上述显示模式。
* 支持HZK，GBK点阵字体，FreeType字体显示。
* 支持高效的图片缩放旋转，alpha混色效果。
* 多种显示函数，提供不同的性能。
* 直线，矩形，三角形，圆形等几何图形的绘制。
* 音频支持，用于播放声音，mp3，wav，可扩展。
* 动画支持，用于播放动画，支持gif。
* 支持各种图像裁剪，例如J2ME2的SetClip函数。

主页:
----------
http://www.iacger.com/product/official/nge2.php

安装
---------

nge2使用 CMake 进行构建，在windows上请使用cmake-gui.
在类UNIX平台下（psp、android、linux）请使用项目根目录下提供的
脚本，例：

PSP:
./make-psp.sh
linux:
./make-linux.sh
android:
./make-android.sh

更详细的用法请通过--help选项查看，例：

./make-android.sh --help

如果你缺失了任何依赖，CMake 会将其指出。你可以在安装完依赖后继续构建，当然，你需要先删除 `CMakeCache.txt` 文件。

cache的修改：
在 Unix/Mac 平台上，你可以使用 `ccmake` 图形式设定你需要的选项。
在 windows 上，则可以使用 `cmake-gui` (关于如何使用cmake-gui，请自行G之)。

目前已通过 linux 、android 和 psp 端的编译。

windows下：
VC6.0已通过。
对VS2003，VS2005，VS2008编译环境的支持正在调教中（太大，没装）...

依赖
---------
你可以直接下载由我们打包好的压缩包解压到项目根目录即可。

* zlib 
Simple data compression library 
http://www.zlib.net

* libjpeg 
Library for JPEG image compression 
http://sourceforge.net/projects/libjpeg

* libpng 
Official PNG reference library 
http://www.libpng.org/pub/png/libpng.html

* giflib 
A library for processing GIF 
http://sourceforge.net/projects/giflib

* freetype 
Portable font engine 
http://www.freetype.org

* SDL (only need on win32) 
Simple DirectMedia Layer 
http://www.libsdl.org/

* OpenGL (need on win32 and linux) 
OpenGL render system 
http://www.opengl.org/

* Gstreamer (need on linux) 
Open source multimedia framework 
http://gstreamer.freedesktop.org/

* sqlite3 (need for demo5) 
a self-contained, serverless, zero-configuration, transactional SQL database engine 
http://www.sqlite.org/ 

* unzip (need for demo9) 
a library to uncompress .zip files 
on linux, you can download it from download page.

附： PSP SDK
---------------
推荐：
minpspw(pspsdk)(http://sourceforge.net/projects/minpspw/)

二进制版本下载地址（未更新）：

* [linux (ubuntu 32bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/minpspw_0.11.1-1ubuntu0_i386.deb/download)

* [linux (ubuntu 64bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/minpspw_0.11.1-1ubuntu0_amd64.deb/download)

* [linux (rpm 32bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/minpspw-pspsdk-0.11.1-1.i386.rpm/download)

* [linux (rpm 64bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/minpspw-pspsdk-0.11.1-1.x86_64.rpm/download)

* [windows (32bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/pspsdk-setup-0.11.1.exe/download)

* [Mac](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/MinPSP%200.11%20Mac.mpkg/download)
