#include "libnge2.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "nge_app.h"

/**
 * 测试nge2的文件系统
 */

#ifdef ANDROID
#define RES_PATH(path) ("/sdcard/libnge2/demo14/" path)
#else
#define RES_PATH(path) (path)
#endif

int game_quit = 0;

#ifdef NGE_INPUT_BUTTON_SUPPORT
void btn_down(int keycode)
{
	switch(keycode)
    {
	case PSP_BUTTON_UP:
		break;
	case PSP_BUTTON_DOWN:
		break;
	case PSP_BUTTON_LEFT:
		break;
	case PSP_BUTTON_RIGHT:
		break;
	case PSP_BUTTON_TRIANGLE:
		break;
	case PSP_BUTTON_CIRCLE:
		break;
	case PSP_BUTTON_CROSS:
		break;
    case PSP_BUTTON_SQUARE:
		break;
	case PSP_BUTTON_SELECT:
		break;
	case PSP_BUTTON_START:
		//开始键退出
		game_quit = 1;
		break;
    }
}

void btn_up(int keycode)
{
	switch(keycode)
    {
	case PSP_BUTTON_UP:
		break;
	case PSP_BUTTON_DOWN:
		break;
	case PSP_BUTTON_LEFT:
		break;
	case PSP_BUTTON_RIGHT:
		break;
	case PSP_BUTTON_TRIANGLE:
		break;
	case PSP_BUTTON_CIRCLE:
		break;
	case PSP_BUTTON_CROSS:
		break;
    case PSP_BUTTON_SQUARE:
		break;
	case PSP_BUTTON_SELECT:
		break;
	case PSP_BUTTON_START:
		break;
    }
}
#endif

void DrawScene()
{

	BeginScene(1);
	EndScene();
}

void test_ref_handle(ngeRefHandle pkg) {
	ngeRefHandle h = ngeRefHandleCopy(pkg);
	ngeRef(pkg);
	ngeUnref(pkg);
	ngeRefHandleRelease(h);
}

int fd[32];

int init()
{
	int i, size, zfd;
	ngePackage* pkg;
	ngeRefHandle handle;
	char* txt;

	NGE_Init(INIT_VIDEO);
	NGE_SetFontEncoding(NGE_ENCODING_UTF_8);
	ngePackageZipInit();

#ifdef NGE_INPUT_BUTTON_SUPPORT
	InitInput(btn_down,btn_up,1);
#endif

	for (i=0; i<32; i++) {
		fd[i] = io_fopen(RES_PATH("images/demo2_bg.jpg"), IO_RDONLY);
	}

	size = io_fsize(fd[0]);
	for (i=0; i<32; i++) {
		io_fclose(fd[i]);
	}

	pkg = ngeLoadPackage(RES_PATH("database/data.zip"), 0);
	handle = ngeCreatePackageHandle(pkg);
	test_ref_handle(handle);

	zfd = ngeOpenInPackage(NGE_REF_CAST(ngePackage*, handle), "1.txt", IO_RDONLY);
	size = io_fsize(zfd);
	txt = (char*)malloc(size+1);
	size = io_fread(txt, 1, size, zfd);
	txt[size+1] = '\0';
	free(txt);
	io_fclose(zfd);
	ngeRefHandleRelease(handle);
	return 0;
}

int mainloop()
{
	if (game_quit)
		return NGE_APP_QUIT;

	InputProc();
	LimitFps(60);

	return NGE_APP_NORMAL;
}

int fini()
{
	NGE_Quit();

	return 0;
}

static nge_app_t app;

extern "C"
int main(int argc, char* argv[])
{
	app.init = init;
	app.mainloop = mainloop;
	app.fini = fini;
	nge_register_app(&app);
	return 0;
}
