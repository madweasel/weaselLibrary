/*********************************************************************
	BitmapLoader.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "BitmapLoader.h"

//-----------------------------------------------------------------------------
// Name: LoadBitmapRGBA()
// Desc: Achtung: Bei 2 und 16-Farben Bitmap Problem bei krummen widths.
//-----------------------------------------------------------------------------
unsigned int *LoadBitmapARGB(const char *filename, unsigned int *width, unsigned int *height)
{
	// Locals
	BYTE			*	bData	= NULL;
	DWORD			*	dwData	= NULL;
	HANDLE				hFile	= NULL;
	DWORD				dwInfoSize;
	DWORD				dwNumColors;
	DWORD				dwBytesPerLine;
	BITMAPFILEHEADER	bmfh;				// Hat jede Bitmap
	BITMAPCOREHEADER	bmch;				// Simple 0815
	BITMAPINFOHEADER	bmih;				// Win NT 3.51 and earlier
	RGBQUAD			*	rgbQuad   = NULL;
	RGBTRIPLE		*	rgbTriple = NULL;
	DWORD				a, b, bytesRead;

	// Nullen
	*width			= 0;
	*height			= 0;
	dwNumColors		= 0;
	dwBytesPerLine	= 0;

	// Filename korrekt ?
	if (filename == NULL) return NULL;
	
	// Öffne Datei
	hFile = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// Handle gültig ?
	if (hFile == INVALID_HANDLE_VALUE) return NULL;

	// Header einlesen
	ReadFile(hFile, &bmfh, sizeof(bmfh), &bytesRead, NULL);

	// Bitmap ?
	if (bmfh.bfType != 0x4D42)goto closeAndQuit;

	// Nächstes DWORD entscheidet, welche Header es ist
	ReadFile(hFile, &dwInfoSize, sizeof(dwInfoSize), &bytesRead, NULL);

	// Wieder zurückspringen
	SetFilePointer(hFile, -1 * (long) sizeof(dwInfoSize), NULL, FILE_CURRENT);

	// Welche Header ?
	switch (dwInfoSize)
	{
	case sizeof(BITMAPCOREHEADER):
		ReadFile(hFile, &bmch, sizeof(bmch), &bytesRead, NULL);

		switch (bmch.bcBitCount)
		{
		case 8:		dwNumColors	=        256; dwBytesPerLine = bmch.bcWidth * 1;	break;
		case 16:	dwNumColors	=      65536; dwBytesPerLine = bmch.bcWidth * 2;	break;
		case 24:	dwNumColors	=   16777216; dwBytesPerLine = bmch.bcWidth * 3;	break;
		case 32:	dwNumColors	=		   0; dwBytesPerLine = bmch.bcWidth * 4;	break;
		default:    goto closeAndQuit;
		}

		if (bmch.bcBitCount < 24)
		{
			// Lese Farbtabelle ein
			rgbTriple = new RGBTRIPLE[dwNumColors];
			ReadFile(hFile, rgbTriple, dwNumColors * sizeof(RGBTRIPLE), &bytesRead, NULL);
		
			// Copy Triples to Quads
			rgbQuad = new RGBQUAD[dwNumColors];
			for (a = 0; a < dwNumColors; a++) {
				rgbQuad[a].rgbBlue		= rgbTriple[a].rgbtBlue;
				rgbQuad[a].rgbGreen		= rgbTriple[a].rgbtGreen;
				rgbQuad[a].rgbRed		= rgbTriple[a].rgbtRed;
				rgbQuad[a].rgbReserved	= 0;
			}
		} else {
			rgbTriple	= NULL;
			rgbQuad		= NULL;
		}

		// Übernehme Ausmaße
		*width  = (DWORD) bmch.bcWidth;
		*height = (DWORD) bmch.bcHeight;

		break;
	case sizeof(BITMAPINFOHEADER):
		ReadFile(hFile, &bmih, sizeof(bmih), &bytesRead, NULL);

		// Mit Compression kann ich nix anfangen
		if (bmih.biCompression != BI_RGB)
			goto closeAndQuit;

		switch (bmih.biBitCount)
		{
		case 8:		dwNumColors	=        256; dwBytesPerLine = bmih.biWidth * 1;	break;
		case 16:	dwNumColors	=      65536; dwBytesPerLine = bmih.biWidth * 2;	break;
		case 24:	dwNumColors	=   16777216; dwBytesPerLine = bmih.biWidth * 3;	break;
		case 32:	dwNumColors	=		   0; dwBytesPerLine = bmih.biWidth * 4;	break;
		default:    goto closeAndQuit;
		}
	
		if (bmih.biBitCount < 24) {
			// Lese Farbtabelle ein
			rgbQuad = new RGBQUAD[dwNumColors];
			ReadFile(hFile, rgbQuad, dwNumColors * sizeof(RGBQUAD), &bytesRead, NULL);
		} else {
			rgbTriple	= NULL;
			rgbQuad		= NULL;
		}

		// Übernehme Ausmaße
		*width  = (DWORD) bmih.biWidth;
		*height = (DWORD) bmih.biHeight;

		break;
	default: 
		goto closeAndQuit;
	}

    // add padding bytes
    dwBytesPerLine += (4-(dwBytesPerLine%4))%4;

	// Lese Bitmap ein
	if (0xFFFFFFFF == SetFilePointer(hFile, (long) bmfh.bfOffBits, NULL, FILE_BEGIN)) goto closeAndQuit;

	// Pixel für Pixel
	WORD color;
	bData  = new BYTE[dwBytesPerLine];
	dwData = new DWORD[*height * *width];
	for (a = 0; a < *height; a++)  {

		// Lese Zeile ein
		ReadFile(hFile, bData, dwBytesPerLine, &bytesRead, NULL);
        
		for (b = 0; b < *width; b++) {

			// Farbtabelle benutzen ?
			if (rgbQuad != NULL) {
				switch (dwNumColors)
				{
				case   256:	color = bData[b];									break;
				case 65536:	color = bData[2 * b + 0]  + 16 * bData[2 * b + 1];	break;
				}
				dwData[b * *height + a]  = (rgbQuad[color].rgbBlue  <<  0)
				                         + (rgbQuad[color].rgbGreen <<  8)
									     + (rgbQuad[color].rgbRed   << 16);
			
            } else if (dwNumColors == 16777216) {
				dwData[b * *height + a]  = (bData[3 * b + 2] << 16)		// Red
									     + (bData[3 * b + 1] <<  8)		// Green 
									     + (bData[3 * b + 0] <<  0);	// Blue
			
            } else if (dwNumColors > 16777216) {
				dwData[b * *height + a]  = (bData[4 * b + 3] << 24)		// Alpha
										 + (bData[4 * b + 2] << 16)		// Red
									     + (bData[4 * b + 1] <<  8)		// Green 
									     + (bData[4 * b + 0] <<  0);	// Blue
			}
		}
	}

	// File Schliessen
	CloseHandle(hFile);
	BMP_SAFE_DELETE_ARRAY(bData);
	BMP_SAFE_DELETE_ARRAY(rgbTriple);
	BMP_SAFE_DELETE_ARRAY(rgbQuad);

	return (unsigned int *) dwData;

closeAndQuit:
	CloseHandle(hFile);
	BMP_SAFE_DELETE_ARRAY(bData);
	BMP_SAFE_DELETE_ARRAY(rgbTriple);
	BMP_SAFE_DELETE_ARRAY(rgbQuad);
	return NULL;
}

//-----------------------------------------------------------------------------
// Name: WriteBitmap()
// Desc: 
//-----------------------------------------------------------------------------
bool WriteBitmap(const char *filename, RGBTRIPLE *bitmap, unsigned int width, unsigned int height)
{
	// Parameter Überprüfen
	if (filename  == NULL)	return false;
	if (bitmap	  == NULL)	return false;

	// Locals
    DWORD               dwBytesPerLine  = width*3;
	HANDLE				hFile           = NULL;
	BITMAPFILEHEADER	bmfh;
	BITMAPINFOHEADER	bmih;
	DWORD				a, b, bytesWritten;
	RGBTRIPLE			*line;
	
    // add padding bytes
    dwBytesPerLine += (4-(dwBytesPerLine%4))%4;

	// Datei erstellen
	hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	// Erfolgreich geöffnet ?
	if (hFile == INVALID_HANDLE_VALUE)	return false;

	// Create File Header
	bmfh.bfType			= 0x4D42;
	bmfh.bfSize			= sizeof(bmfh) + sizeof(bmih) + sizeof(RGBTRIPLE) * width * height;
	bmfh.bfReserved1	= 0;
	bmfh.bfReserved2	= 0;
	bmfh.bfOffBits		= sizeof(bmfh) + sizeof(bmih);
	
	// Schreibe Header
	WriteFile(hFile, &bmfh, sizeof(bmfh), &bytesWritten, NULL);

	// Create Info Header
	bmih.biSize			= sizeof(bmih);
	bmih.biWidth		= width;
	bmih.biHeight		= height;
	bmih.biPlanes		= 1;
	bmih.biBitCount		= 24;
	bmih.biCompression	= BI_RGB;
	bmih.biSizeImage	= height*dwBytesPerLine;
	bmih.biXPelsPerMeter= 1000;
	bmih.biYPelsPerMeter= 1000;
	bmih.biClrUsed		= 0;
	bmih.biClrImportant = 0;
	
	// Schreibe Info Header
	WriteFile(hFile, &bmih, sizeof(bmih), &bytesWritten, NULL);

	// Speicher für Zeile
	line = new RGBTRIPLE[width+1];

    // padding byte
    line[width].rgbtBlue	= 0;
    line[width].rgbtGreen	= 0;
    line[width].rgbtRed		= 0;

	// Bitmapdaten schreiben (mit drehen)
	for (a=0; a<height; a++)
	{
		// Line zusammenstellen
		for (b=0; b<width; b++)
		{
			line[b]	 = bitmap[a * width + b];
		}
		WriteFile(hFile, line, dwBytesPerLine, &bytesWritten, NULL);
	}

	// File Schliessen
	CloseHandle(hFile);
	delete [] line;

	return true;
}

//-----------------------------------------------------------------------------
// Name: colorPalette()
// Desc: 
//-----------------------------------------------------------------------------
colorPalette::colorPalette(const char *filename)
{
	LoadPalette(filename);
}

//-----------------------------------------------------------------------------
// Name: WriteBitmap()
// Desc: 
//-----------------------------------------------------------------------------
bool colorPalette::LoadPalette(const char *filename)
{
	// locals
	ifstream		is(filename);
	unsigned int	numColors;
	unsigned int	curColor = 0;
	unsigned int	curValue;

	// file found?
	if (!is.is_open()) {
		cout << "ERROR: Could not open palette file: " << filename << endl;
		while (true);
		return false; 
	}

	// read number of colors
	is >> numColors;
	palette.resize(numColors);
	
	// loop while extraction from file is possible
	while (is.good() && curColor < numColors) {
		is >> curValue; palette[curColor].rgbtRed	= curValue;
		is >> curValue; palette[curColor].rgbtGreen	= curValue;
		is >> curValue; palette[curColor].rgbtBlue	= curValue;
		curColor++;
	}

	if (curColor!=numColors) {
		//cout << "ERROR: colorPalette::LoadPalette(): curColor!=numColors" << endl;
		return false;
	}

	is.close();
	
	return true;
}

//-----------------------------------------------------------------------------
// Name: getColor()
// Desc: 
//-----------------------------------------------------------------------------
void colorPalette::getColor(double valueFromZeroToOne, RGBTRIPLE &color)
{
	// limit parameter
	if (valueFromZeroToOne>0.999999) valueFromZeroToOne = 0.999999;
	if (valueFromZeroToOne<0.000000) valueFromZeroToOne = 0.000000;

	// locals
	double			fIndex		= (double) valueFromZeroToOne * (palette.size()-1);
	unsigned int	index		= (unsigned int) (fIndex);

	color.rgbtRed	= interpolateValue(index, index+1, palette[index].rgbtRed	, palette[index+1].rgbtRed	, fIndex);
	color.rgbtGreen	= interpolateValue(index, index+1, palette[index].rgbtGreen	, palette[index+1].rgbtGreen, fIndex);
	color.rgbtBlue	= interpolateValue(index, index+1, palette[index].rgbtBlue	, palette[index+1].rgbtBlue	, fIndex);
}


//-----------------------------------------------------------------------------
// Name: interpolateValue()
// Desc: 
//-----------------------------------------------------------------------------
BYTE colorPalette::interpolateValue(unsigned int x1, unsigned x2, BYTE y1, BYTE y2, double &x)
{
	return (BYTE) ((x-x1) / (x2-x1) * (y2-y1) + y1);
}
