/***************************************************************************
 *            nge_utils.c
 *
 *  2011/03/25 06:36:26
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
#include "nge_utils.h"
#include <time.h>

//一些有用的公共函数
//生成一个min---max的浮点数
float rand_float(float min, float max)
{
	static int g_seed ;
	static int inited = 0;
	if(inited == 0){
		g_seed = time(NULL);
		inited = 1;
	}
	g_seed=214013*g_seed+2531011;
	return min+(g_seed>>16)*(1.0f/65535.0f)*(max-min);
}
//生成一个(min,max)的整数
int rand_int(int min, int max)
{
	static int g_seed ;
	static int inited = 0;
	if(inited == 0){
		g_seed = time(NULL);
		inited = 1;
	}
	g_seed=214013*g_seed+2531011;
	return min+(g_seed ^ g_seed>>15)%(max-min+1);
}
