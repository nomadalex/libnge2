/***************************************************************************
 *            nge_app.h
 *
 *  2011/03/25 06:45:09
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

#ifndef _NGE_APP_H
#define _NGE_APP_H

#include "nge_common.h"

enum{
	NGE_APP_NORMAL = 0,
	NGE_APP_QUIT
};

typedef struct nge_app {
	int (*init) (void);
	int (*mainloop) (void);
	int (*fini) (void);
} nge_app_t;

#ifdef __cplusplus
extern "C" {
#endif

	NGE_API void nge_register_app(nge_app_t *app);
	NGE_API nge_app_t* nge_get_app();

#ifdef __cplusplus
}
#endif

#endif /* _NGE_APP_H */
