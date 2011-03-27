/***************************************************************************
 *            nge_input.h
 *
 *  2011/03/25 06:18:44
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _NGE_INPUT_H
#define _NGE_INPUT_H

//input proc define
#if defined WIN32 || defined(__linux__)
#include "SDL.h"
#define PSP_BUTTON_UP            SDLK_w
#define PSP_BUTTON_DOWN          SDLK_s
#define PSP_BUTTON_LEFT          SDLK_a
#define PSP_BUTTON_RIGHT         SDLK_d
#define PSP_BUTTON_TRIANGLE      SDLK_i
#define PSP_BUTTON_CIRCLE        SDLK_l
#define PSP_BUTTON_CROSS         SDLK_k
#define PSP_BUTTON_SQUARE        SDLK_j
#define PSP_BUTTON_LEFT_TRIGGER  SDLK_e
#define PSP_BUTTON_RIGHT_TRIGGER SDLK_u
#define PSP_BUTTON_SELECT        SDLK_v
#define PSP_BUTTON_START         SDLK_b
#define PSP_BUTTON_HOME          SDLK_n
#define PSP_BUTTON_HOLD          SDLK_m
#elif defined _PSP || defined IPHONEOS
#define PSP_BUTTON_UP            8
#define PSP_BUTTON_DOWN          6
#define PSP_BUTTON_LEFT          7
#define PSP_BUTTON_RIGHT         9
#define PSP_BUTTON_TRIANGLE      0
#define PSP_BUTTON_CIRCLE        1
#define PSP_BUTTON_CROSS         2
#define PSP_BUTTON_SQUARE        3
#define PSP_BUTTON_LEFT_TRIGGER  4
#define PSP_BUTTON_RIGHT_TRIGGER 5
#define PSP_BUTTON_SELECT        10
#define PSP_BUTTON_START         11
#define PSP_BUTTON_HOME          12
#define PSP_BUTTON_HOLD          13
#endif

#define MOUSE_LBUTTON_DOWN 1
#define MOUSE_LBUTTON_UP   2
#define MOUSE_RBUTTON_DOWN 3
#define MOUSE_RBUTTON_UP   4
#define MOUSE_MBUTTON_DOWN 5
#define MOUSE_MBUTTON_UP   6

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ButtonProc)(int key);
typedef void (*AnalogProc)(unsigned char analog_x,unsigned char analog_y);

typedef void (*MouseMoveProc)(int x,int y);
typedef void (*MouseButtonProc)(int type,int x,int y);

typedef void (*TouchMoveProc)(int which,int x,int y,int rx,int ry);
typedef void (*TouchButtonProc)(int which,int type,int x,int y);

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
#define FiniInput(...) // we do not need it at all!!

/**
 *初始化mouse-touch
 */
void InitMouse(MouseButtonProc mouse_btn,MouseMoveProc mouse_move);

void InitTouch(TouchButtonProc touchbuttonproc,TouchMoveProc touchmoveproc);

#ifndef IPHONEOS
MouseMoveProc GetMouseMoveProc();
MouseButtonProc GetMouseButtonProc();

void SetSwapXY(int flag);
int  GetSwapXY();
#endif

void EmulateTouchMove(int flag);

#ifdef __cplusplus
}
#endif

#endif /* _NGE_INPUT_H */
