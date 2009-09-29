/*
 * Thanks to Dr.Watson JGE++!
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeRect helper class
*/


#ifndef HGERECT_H
#define HGERECT_H
#include "nge_define.h"

class hgeRect
{
public:
	float	x1, y1, x2, y2;

	hgeRect(float _x1, float _y1, float _x2, float _y2) {x1=_x1; y1=_y1; x2=_x2; y2=_y2; bClean=0; }
	hgeRect() {bClean=1;}

	void    Clear() {bClean=1;}
	BOOL    IsClean() const {return bClean;}
	void	Set(float _x1, float _y1, float _x2, float _y2) { x1=_x1; x2=_x2; y1=_y1; y2=_y2; bClean=0; }
	void	SetRadius(float x, float y, float r) { x1=x-r; x2=x+r; y1=y-r; y2=y+r; bClean=0; }
	void	Encapsulate(float x, float y);
	BOOL	TestPoint(float x, float y) const;
	BOOL	Intersect(const hgeRect *rect) const;

private:
	BOOL	bClean;
};


#endif
