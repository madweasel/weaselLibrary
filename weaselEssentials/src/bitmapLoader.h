/*********************************************************************\
	BitmapLoader.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef BITMAPLOADER_H
#define BITMAPLOADER_H

#include <windows.h>
#include <vector>
#include <fstream>   

#include "myUtil.h"

using namespace std;

/*** Konstanten & Strukturen *****************************************/
#define BMP_SAFE_DELETE_ARRAY(p)		{ if(p) { delete [] (p);   (p)=NULL; } }

/*** Funktionen *****************************************/

unsigned int *  LoadBitmapARGB(const char *filename, unsigned int * width, unsigned int *height);
bool            WriteBitmap   (const char *filename, RGBTRIPLE *	bitmap, unsigned int width, unsigned int height);

struct colorPalette
{
	vector<RGBTRIPLE>	palette;

				colorPalette	(const char *filename);
	bool		LoadPalette		(const char *filename);
	void		getColor		(double valueFromZeroToOne, RGBTRIPLE &color);
	BYTE		interpolateValue(unsigned int x1, unsigned x2, BYTE y1, BYTE y2, double &x);
};

#endif
