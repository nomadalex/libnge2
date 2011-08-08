/***************************************************************************
 *            nge_app.cpp
 *
 *  2011/03/25 06:55:37
 *  Copyright 2010 TOPOC. All rights reserved.
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
#include "nge_app.hpp"

static nge_app_t s_app;
static CNgeApp *s_App = 0;

static int app_init() {
	return s_App->Init();
}

static int app_mainloop() {
	return s_App->Mainloop();
}

static int app_fini() {
	return s_App->Fini();
}

void nge_registerApp(CNgeApp *app) {
	s_App = app;
	s_app.init = app_init;
	s_app.mainloop = app_mainloop;
	s_app.fini = app_fini;
	nge_register_app(&s_app);
}
