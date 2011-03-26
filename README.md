NGE2
====

NGE2(Not a Game Engine的递归缩写)，NGE第一版原为制作中国象棋psp所写，第二版得到了更全面的强化，是PSP/Windows/iPhone/Linux开发的一个程序库，不是一个游戏引擎，设计思想是简单易用，用来做各个显示程序或者游戏引擎，NGE2追求的是性能，所以全部用的c函数写，而显示函数也分为各个性能要求，提供不同的显示函数。 NGE2的设计就是让开发人员不需要用PSP调试就能开发出PSP的应用程序。就是说在Windows下调试，在PSP上运行。 大大降低了PSP调试程序浪费的时间。

特点:
-----
* 支持16位（RGBA5650 RGBA5551 RGBA4444）/32位(RGBA_8888)显示。
* 支持JPG,BMP(24,32位),PNG,TGA（24,32位）图片读取，并转化到上述显示模式。
* 支持HZK，GBK点阵字体，FreeType字体显示。
* 支持高效的图片缩放旋转，alpha混色效果。
* 多种显示函数，提供不同的性能。
* 直线，矩形，三角形，圆形等几何图形的绘制。
* 音频支持，用于播放声音，mp3，wav，可扩展。
* 视频支持，用于播放动画，pmp支持。
* 支持各种图像裁剪，例如J2ME2的SetClip函数。
* 支持VC6.0，VS2003，VS2005，VS2008编译环境。

主页:
-----
http://www.iacger.com/product/official/nge2.php

安装
-----

nge2使用 CMake 进行构建，请使用 out-of-source 方式进行构建。 建议：
    $ mkdir Build
    $ cd Build

    $ cmake ..
    or
    $ cmake -G "MinGW Makefiles" ..
如果是为 psp 编译 NGE2， 请先下载 [cmaketoolchain4psp](https://github.com/ifreedom/cmaketoolchain4psp)。然后输入 ：
    $ psp-cmake ..

在 Unix/Mac 平台上，你可以使用 `ccmake` 来设定你需要的选项。在 windows 上，
你可以使用 `cmake-gui`。

如果你缺失了任何依赖，CMake 会将其指出。你可以在安装完依赖后继续构建，当然，
你需要先删除 `CMakeCache.txt` 文件。

使用生成的 makefiles 或项目文件来构建 NGE2. 如果使用 makefiles, 键入 `make` 或
`mingw32-make`. 如果使用项目文件，用你的 IDE 打开它。如果使用 Xcode, 你可以使用
IDE 或从命令行里键入 `xcodebuild`.

如果你需要，你可以安装库文件和头文件。

	$ make install
	or
	$ make install DESTDIR=/path/to/destination

目前已通过 psp 端以及 linux 端的编译以及 demos 。

依赖
----
如果需要开发psp端的软件则需要安装 minpspw(pspsdk) 。

下载地址：
* [linux (ubuntu 32bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/minpspw_0.11.1-1ubuntu0_i386.deb/download)

* [linux (ubuntu 64bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/minpspw_0.11.1-1ubuntu0_amd64.deb/download)


* [linux (rpm 32bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/minpspw-pspsdk-0.11.1-1.i386.rpm/download)

* [linux (rpm 64bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/minpspw-pspsdk-0.11.1-1.x86_64.rpm/download)

* [windows (32bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/pspsdk-setup-0.11.1.exe/download)

* [Mac](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/MinPSP%200.11%20Mac.mpkg/download)