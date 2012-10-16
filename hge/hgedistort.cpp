/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeDistortionMesh helper class implementation
*/

#include "hgedistort.h"
#include "nge_graphics.h"

hgeDistortionMesh::hgeDistortionMesh(int cols, int rows)
{
	int i;

	nRows=rows;
	nCols=cols;
	cellw=cellh=0;
	quad.tex=NULL;
	disp_array=new vertexf[rows*cols];

	for(i=0;i<rows*cols;i++)
	{
		disp_array[i].x=0.0f;
		disp_array[i].y=0.0f;
		disp_array[i].u=0.0f;
		disp_array[i].v=0.0f;
		
		disp_array[i].z=0.0f;
		disp_array[i].color=0xFFFFFFFF;
	}
}

hgeDistortionMesh::hgeDistortionMesh(const hgeDistortionMesh &dm)
{
	nRows=dm.nRows;
	nCols=dm.nCols;
	cellw=dm.cellw;
	cellh=dm.cellh;
	tx=dm.tx;
	ty=dm.ty;
	width=dm.width;
	height=dm.height;
	quad=dm.quad;

	disp_array=new vertexf[nRows*nCols];
	memcpy(disp_array, dm.disp_array, sizeof(vertexf)*nRows*nCols);
}

hgeDistortionMesh::~hgeDistortionMesh()
{
	delete[] disp_array;
}

hgeDistortionMesh& hgeDistortionMesh::operator= (const hgeDistortionMesh &dm)
{
	if(this!=&dm)
	{
		nRows=dm.nRows;
		nCols=dm.nCols;
		cellw=dm.cellw;
		cellh=dm.cellh;
		tx=dm.tx;
		ty=dm.ty;
		width=dm.width;
		height=dm.height;
		quad=dm.quad;

		delete[] disp_array;
		disp_array=new vertexf[nRows*nCols];
		memcpy(disp_array, dm.disp_array, sizeof(vertexf)*nRows*nCols);
	}

	return *this;
	
}

void hgeDistortionMesh::SetTexture(image_p tex)
{
	quad.tex=tex;
}

void hgeDistortionMesh::SetTextureRect(float x, float y, float w, float h)
{
	int i,j;
	float tw,th;

	tx=x; ty=y; width=w; height=h;

	if (quad.tex)
	{
		tw=(float)(quad.tex)->texw;
		th=(float)(quad.tex)->texh;
	}
	else
	{
		tw = w;
		th = h;
	}

	cellw=w/(nCols-1);
	cellh=h/(nRows-1);

	for(j=0; j<nRows; j++)
		for(i=0; i<nCols; i++)
		{
			disp_array[j*nCols+i].u=(x+i*cellw)/tw;
			disp_array[j*nCols+i].v=(y+j*cellh)/th;

			disp_array[j*nCols+i].x=i*cellw;
			disp_array[j*nCols+i].y=j*cellh;
		}
}

void hgeDistortionMesh::Clear(uint32_t col, float z)
{
	int i,j;

	for(j=0; j<nRows; j++)
		for(i=0; i<nCols; i++)
		{
			disp_array[j*nCols+i].x=i*cellw;
			disp_array[j*nCols+i].y=j*cellh;
			disp_array[j*nCols+i].color=col;
			disp_array[j*nCols+i].z=z;
		}
}

void hgeDistortionMesh::Render(float x, float y)
{
	int i,j,idx;

	for(j=0; j<nRows-1; j++)
		for(i=0; i<nCols-1; i++)
		{
			idx=j*nCols+i;

			quad.v[0].u=disp_array[idx].u;
			quad.v[0].v=disp_array[idx].v;
			quad.v[0].x=x+disp_array[idx].x;
			quad.v[0].y=y+disp_array[idx].y;
			quad.v[0].z=0;
			quad.v[0].color=disp_array[idx].color;

			quad.v[1].u=disp_array[idx+nCols].u;
			quad.v[1].v=disp_array[idx+nCols].v;
			quad.v[1].x=x+disp_array[idx+nCols].x;
			quad.v[1].y=y+disp_array[idx+nCols].y;
			quad.v[1].z=0;
			quad.v[1].color=disp_array[idx+nCols].color;

			quad.v[2].u=disp_array[idx+nCols+1].u;
			quad.v[2].v=disp_array[idx+nCols+1].v;
			quad.v[2].x=x+disp_array[idx+nCols+1].x;
			quad.v[2].y=y+disp_array[idx+nCols+1].y;
			quad.v[2].z=0;
			quad.v[2].color=disp_array[idx+nCols+1].color;

			quad.v[3].u=disp_array[idx+1].u;
			quad.v[3].v=disp_array[idx+1].v;
			quad.v[3].x=x+disp_array[idx+1].x;
			quad.v[3].y=y+disp_array[idx+1].y;
			quad.v[3].z=0;
			quad.v[3].color=disp_array[idx+1].color;

			RealRenderQuad(quad);
		}
}

void hgeDistortionMesh::SetColor(int col, int row, uint32_t color)
{
	if(row<nRows && col<nCols) disp_array[row*nCols+col].color=color;
}

void hgeDistortionMesh::SetDisplacement(int col, int row, float dx, float dy, int ref)
{
	if(row<nRows && col<nCols)
	{
		switch(ref)
		{
			case HGEDISP_NODE:		dx+=col*cellw; dy+=row*cellh; break;
			case HGEDISP_CENTER:	dx+=cellw*(nCols-1)/2;dy+=cellh*(nRows-1)/2; break;
			case HGEDISP_TOPLEFT:	break;
		}

		disp_array[row*nCols+col].x=dx;
		disp_array[row*nCols+col].y=dy;
	}
}

uint32_t hgeDistortionMesh::GetColor(int col, int row) const
{
	if(row<nRows && col<nCols) return disp_array[row*nCols+col].color;
	else return 0;
}

void hgeDistortionMesh::GetDisplacement(int col, int row, float *dx, float *dy, int ref) const
{
	if(row<nRows && col<nCols)
	{
		switch(ref)
		{
			case HGEDISP_NODE:		*dx=disp_array[row*nCols+col].x-col*cellw;
									*dy=disp_array[row*nCols+col].y-row*cellh;
									break;

			case HGEDISP_CENTER:	*dx=disp_array[row*nCols+col].x-cellw*(nCols-1)/2;
									*dy=disp_array[row*nCols+col].x-cellh*(nRows-1)/2;
									break;

			case HGEDISP_TOPLEFT:	*dx=disp_array[row*nCols+col].x;
									*dy=disp_array[row*nCols+col].y;
									break;
		}
	}
}

