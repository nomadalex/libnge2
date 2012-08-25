NGE2
====

NGE2(Not a Game Engine的递归缩写)，NGE第一版原为制作中国象棋PSP所写，第二版得到了更全面的强化，
是PSP/Windows/iPhone/Linux/android开发的一个程序库，设计的初衷不是一个游戏引擎，设计思想是简单易用，
用来做各个显示程序或者游戏引擎，NGE2追求的是性能，所以全部用的c函数写，而显示函数也分为各个性能要求，
提供不同的显示函数。 
NGE2的最初的设计就是让开发人员不需要用PSP调试就能开发出PSP的应用程序。后来衍生到了IOS和Android，
就是说在Windows/linux下调试，在PSP/iphone/android上运行，大大降低了PSP/iphone/android调试程序浪费的时间。
NGE2 v2.1采用BSD许可证。

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
* 支持HGE的粒子库。
* J2ME的开发者能很容易上手，支持图像裁剪SetClip，绘图函数DrawRegion。
* 提供硬件加速图片到图片绘制image_to_image。

主页:
----------
http://www.iacger.com/

安装
---------

NGE2使用 CMake 进行构建。
在windows上推荐使用cmake-gui，生成VS200x的工程文件后编译。
目前已通过 Windows，linux 、android 和 PSP 平台的工程文件的生成和编译。iOs平台目前需要自行手动建立xCoder工程。
在类UNIX平台下（psp、android、linux）推荐使用项目根目录下提供的
脚本，例：

PSP:
./script/make-psp.sh

linux:
./script/make-linux.sh

android:
./script/make-android.sh

更详细的用法请通过--help选项查看，例：

./script/make-android.sh --help

如果你缺失了任何依赖，CMake 会将其指出。你可以在安装完依赖后继续构建，当然，你需要先删除 `CMakeCache.txt` 文件。

cache的修改：
在 Unix/Mac 平台上，你可以使用 `ccmake` 图形式设定你需要的选项。
在 windows 上，则可以使用 `cmake-gui` (关于如何使用cmake-gui，请自行G之)。

Windows平台的编译流程：
1 安装VS2003或者以上版本。
2 使用git下载libnge2的源代码。
3 下载Win32Depends.7z(https://github.com/downloads/ifreedom/libnge2/Win32Depends.7z)后解压到libnge2的根目录。
4 下载例子的资源文件res.7z(https://github.com/downloads/ifreedom/libnge2/res.7z)后解压放到sample目录下。
5 使用CMake-gui选择libnge2根目录生成一个vs2003工程。
6 打开CMake-gui生成的工程编译即可。

Android平台的编译流程：
NGE2采用的是NDK方式，所以需要搭建NDK的开发环境，这里以ubuntu作为实例。
1 下载NDK,写本文档时最新的为android-ndk-r8b，这里以这个版本为例，解压到目录，本例用当前用户的home目录(即~/目录)。
2 打开命令行终端，设置NDK的环境变量export ANDROID_NDK=~/android-ndk-r8b。
3 使用git下载libnge2解压。
4 下载[AndroidDepends.7z](https://github.com/downloads/ifreedom/libnge2/AndroidDepends.7z)解压到libnge2的根目录。
5 下载例子的资源文件res.7z(https://github.com/downloads/ifreedom/libnge2/res.7z)后解压放到sample目录下。
6 命令行终端进入libnge2的目录下，命令行执行./script/make-android.sh

如果有任何疑问，可以到讨论版：
http://www.iacger.com/forum.php?mod=forumdisplay&fid=180

依赖
---------
你可以直接下载由我们打包好的压缩包解压到项目根目录即可。
* [Win32Depends.7z](https://github.com/downloads/ifreedom/libnge2/Win32Depends.7z)
* [AndroidDepends.7z](https://github.com/downloads/ifreedom/libnge2/AndroidDepends.7z)

NGE2使用的开发库
----------
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

附： PSP SDK - 编译PSP用
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
