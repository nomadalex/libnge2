#ifndef NGE_INPUT_PROC_H_
#define NGE_INPUT_PROC_H_
#include "nge_define.h"

typedef void (*ButtonProc)(int key);
typedef void (*AnalogProc)(unsigned char analog_x,unsigned char analog_y);

typedef void (*MouseMoveProc)(int x,int y);
typedef void (*MouseButtonProc)(int type,int x,int y);

#ifdef __cplusplus
extern "C"{
#endif
/**
 *初始化输入系统
 *@param ButtonProc downproc,按键按下的处理消息函数
 *@param ButtonProc upproc,按键释放的处理消息函数
 *@param int doneflag,是否自行处理退出消息(HOME键按下时),0(否),1(是)
 *@return 无
 */
void InitInput(ButtonProc downproc,ButtonProc upproc,int doneflag);
/**
 *初始化摇杆
 *@param AnalogProc,摇杆的回调函数
 *@return
 */
void InitAnalog(AnalogProc analogproc);
/**
 *输入响应,具体使用请看例子test/input_test.cpp
 *@return 无
 */
void InputProc();
/**
 *退出输入系统
 *@return 无
 */
void FiniInput();

void InitMouse(MouseButtonProc mouse_btn,MouseMoveProc mouse_move);

#ifdef __cplusplus
}
#endif

#endif
