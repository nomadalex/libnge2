 Building NGE2 with CMake
=================================

CMake is a cross-platform tool that can generate native makefiles and
workspaces that can be used in the compiler environment of your choice.
NGE2 uses CMake as its build system.  The process is essentially the
same for all compilers.  You will need CMake 2.6 or later.
The CMake web site is http://www.cmake.org/ .

CMake 是个跨平台的自动化建构系统，它用组态档控制建构过程（build process）
的方式和 Unix 的 Make 相似，只是 CMake 的组态档取名为 CmakeLists.txt。
Cmake 并不直接建构出最终的软件，而是产生标准的建构档（如 Unix 的 Makefile
或 Windows Visual C++ 的 projects/workspaces），然后再依一般的建构方式使用。
CMake 的官方主页是 http://www.cmake.org/ .

Create a temporary directory to build in.  This is optional but recommended.
If something goes wrong you can delete the build directory and try again.

我们强烈建议你建立一个临时目录用于构建 NGE2 ，这样，如果构建过程出了什么问题，
你可以直接删除目录然后再试一次。

    $ mkdir Build

    $ cd Build

Run CMake in the build directory.  This will detect your compiler
environment and generate makefiles or workspaces for you.  You may pass some
options to customise the build (see later), but the defaults should be fine.
You may need to tell CMake which type of files it should generate, using the
-G option.

在构建目录里运行 CMake. 它会自动检测你的编译环境并为你建立 makefile 或 项目文
件。 你可以传递一些选项来自定义构建过程。你可以通过 -G 选项来告诉 CMake 你需要
的项目文件类型。

Examples (you only need one):

例子 （你只需要输入一个）：

    $ cmake ..

    $ cmake -G "MinGW Makefiles" ..

To closs-compile NGE2 for psp, you need run CMake like this one (need set option PSP to 1 and it only can be set on console):

如果是为 psp 编译 NGE2， 则需要添加 PSP 选项(只能从命令行里设定)来运行 CMake :

    $ cmake -DPSP=1 ..

On Unix/Mac you can use `ccmake` which will present you with a console
program to adjust the options.  On Windows you can use `cmake-gui` instead
(it should be listed in the Start menu or on the desktop).

在 Unix/Mac 平台上，你可以使用 `ccmake` 来设定你需要的选项。在 windows 上，
你可以使用 `cmake-gui`。

If you are missing any dependencies, CMake will tell you at this point.  You
can install the dependencies and continue build.  You may need to delete the
`CMakeCache.txt` file first.

如果你缺失了任何依赖，CMake 会将其指出。你可以在安装完依赖后继续构建，当然，
你需要先删除 `CMakeCache.txt` 文件。

Use the generated makefiles or workspaces to build NGE2.  If using
makefiles, type `make` or `mingw32-make`.  If using workspaces, open them in
your IDE.  If using Xcode, you can use `xcodebuild` from the command-line or
the IDE.

使用生成的 makefiles 或项目文件来构建 NGE2. 如果使用 makefiles, 键入 `make` 或
`mingw32-make`. 如果使用项目文件，用你的 IDE 打开它。如果使用 Xcode, 你可以使用
IDE 或从命令行里键入 `xcodebuild`.

Optionally install the libraries and header files to your system.

如果你需要，你可以安装库文件和头文件。

	$ make install

DESTDIR is supported, e.g.

	$ make install DESTDIR=/tmp/

 Dependencies
---------------

libpng and zlib, for PNG image support.
These are used by loadpng.
Home page: http://www.libpng.org/pub/png/
Windows binaries: http://gnuwin32.sourceforge.net/packages/libpng.htm

Ogg Vorbis, a free lossy audio format. (libogg, libvorbis, libvorbisfile)
This is used by logg.
Home page: http://www.vorbis.com/

jpgalleg does not require any additional libraries.

freetype for font.
