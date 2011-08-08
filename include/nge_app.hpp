/**
 * @file  nge_app.hpp
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2011/08/08 13:25:02
 *
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
 *
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

#ifndef _NGE_APP_HPP
#define _NGE_APP_HPP

enum{
	APP_QUIT = 0,
	APP_NEXT
};

class CNgeApp
{
public:	
	CNgeApp() { mFps = 60; };
	~CNgeApp() {};
	virtual	int Init() = 0;
	virtual	int Mainloop() = 0;
	virtual	int Fini() = 0;
public:
	int mFps;
};

extern "C" void nge_registerApp(CNgeApp *app);

#endif /* _NGE_APP_HPP */
