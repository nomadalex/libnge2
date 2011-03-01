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

请看 INSTALL.md 文件。(目前只测试了psp端的构建)

如果需要开发psp端的软件则需要安装 minpspw(pspsdk) 。

下载地址：
* [linux (ubuntu 32bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/minpspw_0.11.1-1ubuntu0_i386.deb/download)

* [linux (ubuntu 64bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/minpspw_0.11.1-1ubuntu0_amd64.deb/download)


* [linux (rpm 32bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/minpspw-pspsdk-0.11.1-1.i386.rpm/download)

* [linux (rpm 64bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/minpspw-pspsdk-0.11.1-1.x86_64.rpm/download)

* [windows (32bit)](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/pspsdk-setup-0.11.1.exe/download)

* [Mac](http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.11.1/MinPSP%200.11%20Mac.mpkg/download)
