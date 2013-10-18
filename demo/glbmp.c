/*
 *	RollerCoaster2000
 *	Copyright (C) 2003 Plusplus (plusplus@free.fr)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "glbmp.h"

#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gl.h"
//#include "ug.h"


#define BM 19778
#define TGA_RGB 2
#define TGA_RLE 10
#ifndef GL_CLAMP_TO_EDGE
	#define GL_CLAMP_TO_EDGE 0x812f
#endif


int GetRightShiftCount(DWORD dwVal);
int GetNumSet(DWORD dwVal);
int Scale8(int colorValue, int colorBits);


int glBmpLoadTGA32(glBmpImage *img, FILE *tgaFile);
int glBmpLoadTGA24(glBmpImage *img, FILE *tgaFile);
int glBmpLoadCompressedTGA32(glBmpImage *img, FILE *tgaFile);
int glBmpLoadCompressedTGA24(glBmpImage *img, FILE *tgaFile);

int glBmpMemLoadTGA32(glBmpImage *img, char *filePtr);
int glBmpMemLoadTGA24(glBmpImage *img, char *filePtr);
int glBmpMemLoadCompressedTGA32(glBmpImage *img, char *filePtr);
int glBmpMemLoadCompressedTGA24(glBmpImage *img, char *filePtr);

void glBmpLoad32(glBmpImage* img, FILE *imgFile);
void glBmpLoad24(glBmpImage* img, FILE *imgFile);
void glBmpLoad16(glBmpImage* img, FILE *imgFile);
void glBmpLoad8(glBmpImage* img, FILE *imgFile);
void glBmpLoad4(glBmpImage* img, FILE *imgFile);

void glBmpMemLoad32(glBmpImage* img, char *imgPtr);
void glBmpMemLoad24(glBmpImage* img, char *imgPtr);
void glBmpMemLoad16(glBmpImage* img, char *imgPtr);
void glBmpMemLoad8(glBmpImage* img, char *imgPtr);
void glBmpMemLoad4(glBmpImage* img, char *imgPtr);

void glBmpSwapRB(glBmpImage* img);
void glBmpSetAlpha(glBmpImage* img, BYTE alpha);
void glBmpExpandTo32(glBmpImage* img, RGBTRIPLE *image, int pad, int swapRB);

int glBmpGetLineWidth(glBmpImage* img, int bpp);
void glBmpClearMem(glBmpImage* img);


int GetRightShiftCount(DWORD dwVal)
{
	int i;
	for (i=0; i < sizeof(DWORD)*8; i++)
	{
		if (dwVal & 1)
			return i;
		dwVal = dwVal >> 1;
	}
	return -1;
}


int GetNumSet(DWORD dwVal)
{
	int nCount = 0;
	while (dwVal != 0)
	{
		nCount += dwVal & 1;
		dwVal = dwVal >> 1;
	}
	return nCount;
}


int Scale8(int colorValue, int colorBits)
{
	switch (colorBits)
	{
		case 1 : return ((colorValue) ? 255 : 0 );
		case 2 : return (colorValue << 6) | (colorValue << 4) | (colorValue << 2) | colorValue;
		case 3 : return (colorValue << 5) | (colorValue << 2) | (colorValue >> 1);
		case 4 : return (colorValue << 4) | colorValue;
		case 5 : return (colorValue << 3) | (colorValue >> 2);
		case 6 : return (colorValue << 2) | (colorValue >> 4);
		case 7 : return (colorValue << 1) | (colorValue >> 6);
		default : return colorValue;
	}
}


int glBmpGetWidth(glBmpImage* img)
{
	return img->width;
}


int glBmpGetHeight(glBmpImage* img)
{
	return img->height;
}


int glBmpGetColorDepth(glBmpImage* img)
{
	return img->colorDepth;
}


int glBmpGetLineWidth(glBmpImage* img, int bpp)
{
	return ((img->width * bpp + 31) & -32) >> 3;
}


void glBmpClearMem(glBmpImage* img)
{
if (img->pData)
	{
		free(img->pData);
        img->pData = 0;
	}
}


void glBmpInit(glBmpImage* img)
{
	img->width = 0;
	img->height = 0;
	img->colorDepth = 0;
	img->pData = 0;
	img->texID = 0;

	img->texWrapS = GL_CLAMP_TO_EDGE;
	img->texWrapT = GL_CLAMP_TO_EDGE;
	img->minFilter = GL_LINEAR;
	img->magFilter = GL_LINEAR;
}


int glBmpLoadImage(glBmpImage* img, char szFileName[])
{
	char *ext;

	glBmpClearMem(img);

	ext = &szFileName[strlen(szFileName)-3];

	if (!strcmp(ext, "bmp"))
		return glBmpLoadBMP(img, szFileName);

	if (!strcmp(ext, "tga"))
		return glBmpLoadTGA(img, szFileName);

	return 0;
}


int glBmpSaveImage(glBmpImage* img, char szFileName[])
{
	char *ext;

	ext = &szFileName[strlen(szFileName)-3];

	if (!strcmp(ext, "bmp"))
		return glBmpSaveBMP(img, szFileName);


	if (!strcmp(ext, "tga"))
		return glBmpSaveTGA(img, szFileName);

	return 0;
}


int glBmpLoadBMP(glBmpImage* img, char *filename)
{
	BITMAPFILEHEADER fHeader;
	BITMAPINFOHEADER iHeader;
	FILE *bmpFile;
	int palSize;

	if (!(bmpFile = fopen(filename, "rb")))
		return 0;

	fread(&fHeader, 1, sizeof(fHeader), bmpFile);

	if (fHeader.bfType != BM)
	{
		fclose(bmpFile);
		return 0;
	}

	fread(&iHeader, 1, sizeof(iHeader), bmpFile);

	if (iHeader.biSize == sizeof(BITMAPCOREHEADER))
	{
		fclose(bmpFile);
		return 0;
	}

	img->width = iHeader.biWidth;
	img->height = iHeader.biHeight;
	img->colorDepth = iHeader.biBitCount;

	if (iHeader.biCompression != BI_RGB)
	{
		if (iHeader.biCompression == BI_BITFIELDS)
		{
			if (img->colorDepth == 16)
			{
				fread(&img->redMask, 1, sizeof(DWORD), bmpFile);
				fread(&img->greenMask, 1, sizeof(DWORD), bmpFile);
				fread(&img->blueMask, 1, sizeof(DWORD), bmpFile);
			}
		}
		else
		{
			fclose(bmpFile);
			return 0;
		}
	}

	if (img->colorDepth < 16)
	{
		palSize = iHeader.biClrUsed*4;
		fread(&img->colors, 1, palSize, bmpFile);
	}

	fseek(bmpFile, fHeader.bfOffBits, SEEK_SET);

	img->imageSize = iHeader.biSizeImage;

	if (img->imageSize == 0)
		img->imageSize = img->height * glBmpGetLineWidth(img, img->colorDepth);

	switch (img->colorDepth)
	{
		case 32 :
			glBmpLoad32(img, bmpFile);
			break;
		case 24 :
			glBmpLoad24(img, bmpFile);
			break;
		case 16 :
			glBmpLoad16(img, bmpFile);
			break;
		case 8:
			glBmpLoad8(img, bmpFile);
			break;
		case 4:
			glBmpLoad4(img, bmpFile);
			break;
		default :
			break;
	}

	fclose(bmpFile);

	if (img->pData)
		return 1;
	else
		return 0;
}


int glBmpMemLoadBMP(glBmpImage* img, char *filePtr)
{
	BITMAPFILEHEADER fHeader;
	BITMAPINFOHEADER iHeader;
	int palSize;
	char *initialFilePtr;

	initialFilePtr = filePtr;

	if (!filePtr)
		return 0;

	memcpy(&fHeader, filePtr, sizeof(fHeader));
	filePtr += sizeof(fHeader);

	if (fHeader.bfType != BM)
	{
		return 0;
	}

	memcpy(&iHeader, filePtr, sizeof(iHeader));
	filePtr += sizeof(iHeader);

	if (iHeader.biSize == sizeof(BITMAPCOREHEADER))
	{
		return 0;
	}

	img->width = iHeader.biWidth;
	img->height = iHeader.biHeight;
	img->colorDepth = iHeader.biBitCount;

	if (iHeader.biCompression != BI_RGB)
	{
		if (iHeader.biCompression == BI_BITFIELDS)
		{
			if (img->colorDepth == 16)
			{
				memcpy(&img->redMask,filePtr,sizeof(DWORD));
				filePtr += sizeof(DWORD);
				memcpy(&img->greenMask,filePtr,sizeof(DWORD));
				filePtr += sizeof(DWORD);
				memcpy(&img->blueMask,filePtr,sizeof(DWORD));
				filePtr += sizeof(DWORD);
			}
		}
		else
		{
			return 0;
		}
	}

	if (img->colorDepth < 16)
	{
		palSize = iHeader.biClrUsed*4;
		memcpy(&img->colors, filePtr, palSize);
		filePtr += palSize;
	}

	filePtr = &initialFilePtr[fHeader.bfOffBits];

	img->imageSize = iHeader.biSizeImage;

	if (img->imageSize == 0)
		img->imageSize = img->height * glBmpGetLineWidth(img, img->colorDepth);

	switch (img->colorDepth)
	{
		case 32 :
			glBmpMemLoad32(img, filePtr);
			break;
		case 24 :
			glBmpMemLoad24(img, filePtr);
			break;
		case 16 :
			glBmpMemLoad16(img, filePtr);
			break;
		case 8:
			glBmpMemLoad8(img, filePtr);
			break;
		case 4:
			glBmpMemLoad4(img, filePtr);
			break;
		default :
			break;
	}

	if (img->pData)
		return 1;
	else
		return 0;
}


int glBmpLoadTGA(glBmpImage* img, char *filename)
{
	FILE *tgaFile;
	TGAHEADER tHeader;
	int res = 0;

	if (!(tgaFile = fopen(filename, "rb")))
		return 0;

	fread(&tHeader, 1, sizeof(tHeader), tgaFile);

	if (tHeader.tfImageType != TGA_RGB && tHeader.tfImageType != TGA_RLE)
	{
		fclose(tgaFile);
		return 0;
	}

	if (tHeader.tfColorMapType != 0)
	{
		fclose(tgaFile);
		return 0;
	}

	img->width = tHeader.tfWidth[0] + tHeader.tfWidth[1] * 256;
	img->height = tHeader.tfHeight[0] + tHeader.tfHeight[1] * 256;
	img->colorDepth = tHeader.tfBpp;
	img->imageSize = img->width * img->height * (img->colorDepth/8);

	if ((img->colorDepth != 24) && (img->colorDepth != 32))
	{
		fclose(tgaFile);
		return 0;
	}

	switch (img->colorDepth)
	{
		case 32:
			if(tHeader.tfImageType == TGA_RGB)
				res = glBmpLoadTGA32(img,tgaFile);
			if(tHeader.tfImageType == TGA_RLE)
				res = glBmpLoadCompressedTGA32(img,tgaFile);
			if(!res) return 0;
			glBmpSwapRB(img);
			if(tHeader.tfImageDes) /* TGA generated by gimp have tfImageDes = 0x28 and are up side-down */
			{
				glBmpFlipVert(img);
			}
			return res;
		case 24:
			if(tHeader.tfImageType == TGA_RGB)
				res = glBmpLoadTGA24(img,tgaFile);
			if(tHeader.tfImageType == TGA_RLE)
				res = glBmpLoadCompressedTGA24(img,tgaFile);
			if(!res) return 0;
			if(tHeader.tfImageDes) /* TGA generated by gimp have tfImageDes = 0x20 and are up side-down */
			{
				glBmpFlipVert(img);
			}
			return res;
		default :
			fclose(tgaFile);
			return 0;
	}
	fclose(tgaFile);
	return 1;
}


int glBmpMemLoadTGA(glBmpImage* img, char *filePtr)
{
	TGAHEADER tHeader;
	int res = 0;

	if (!filePtr)
		return 0;

	memcpy(&tHeader, filePtr, sizeof(tHeader));
	filePtr += sizeof(tHeader);

	if (tHeader.tfImageType != TGA_RGB && tHeader.tfImageType != TGA_RLE)
	{
		return 0;
	}

	if (tHeader.tfColorMapType != 0)
	{
		return 0;
	}

	img->width = tHeader.tfWidth[0] + tHeader.tfWidth[1] * 256;
	img->height = tHeader.tfHeight[0] + tHeader.tfHeight[1] * 256;
	img->colorDepth = tHeader.tfBpp;
	img->imageSize = img->width * img->height * (img->colorDepth/8);

	if ((img->colorDepth != 24) && (img->colorDepth != 32))
	{
		return 0;
	}

	switch (img->colorDepth)
	{
		case 32:
			if(tHeader.tfImageType == TGA_RGB)
				res = glBmpMemLoadTGA32(img,filePtr);
			if(tHeader.tfImageType == TGA_RLE)
				res = glBmpMemLoadCompressedTGA32(img,filePtr);
			if(!res) return 0;
			glBmpSwapRB(img);
			if(tHeader.tfImageDes) /* TGA generated by gimp have tfImageDes = 0x28 and are up side-down */
			{
				glBmpFlipVert(img);
			}
			return res;
		case 24:
			if(tHeader.tfImageType == TGA_RGB)
				res = glBmpMemLoadTGA24(img,filePtr);
			if(tHeader.tfImageType == TGA_RLE)
				res = glBmpMemLoadCompressedTGA24(img,filePtr);
			if(!res) return 0;
			if(tHeader.tfImageDes) /* TGA generated by gimp have tfImageDes = 0x20 and are up side-down */
			{
				glBmpFlipVert(img);
			}
			return res;
		default :
			return 0;
	}
	return 1;
}


int glBmpLoadTGA32(glBmpImage *img, FILE *tgaFile)
{
	int bytesRead;

	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData) return 0;

	bytesRead = fread(img->pData, 1, img->imageSize, tgaFile);
	if (bytesRead != img->imageSize)
	{
		fclose(tgaFile);
		free(img->pData);
		img->pData = 0;
		return 0;
	}
	return 1;
}


int glBmpMemLoadTGA32(glBmpImage *img, char *filePtr)
{
	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));
	if (!img->pData) return 0;
	memcpy(img->pData, filePtr, img->imageSize);
	glBmpSwapRB(img);
	return 1;
}


int glBmpLoadCompressedTGA32(glBmpImage *img, FILE *tgaFile)
{
	int bytesRead;
	int i, j;
	unsigned char header;

	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData) return 0;

	i = 0;
	while(i < img->width*img->height)
	{
		bytesRead = fread(&header, 1, 1, tgaFile);
		if (bytesRead != 1)
		{
			fclose(tgaFile);
			free(img->pData);
			img->pData = 0;
			return 0;
		}
		if(header < 128)
		{
			header++;
			bytesRead = fread(&img->pData[i],1,header*4,tgaFile);
			if (bytesRead != header*4)
			{
				fclose(tgaFile);
				free(img->pData);
				img->pData = 0;
				return 0;
			}
			i += header;
		}
		else
		{
			header -= 127;
			bytesRead = fread(&img->pData[i],1,4,tgaFile);
			i++;
			if (bytesRead != 4)
			{
				fclose(tgaFile);
				free(img->pData);
				img->pData = 0;
				return 0;
			}
			for(j=1 ; j<header ; j++,i++) img->pData[i] = img->pData[i-1];
		}
	}
	return 1;
}


int glBmpMemLoadCompressedTGA32(glBmpImage *img, char *filePtr)
{
	int i, j;
	unsigned char header;

	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData) return 0;

	i = 0;
	while(i < img->width*img->height)
	{
		header = *((unsigned char*)filePtr);
		filePtr++;
		if(header < 128)
		{
			header++;
			if(i+header > img->width*img->height) return 0;
			memcpy(&img->pData[i],filePtr,header*4);
			filePtr += header*4;
			i += header;
		}
		else
		{
			header -= 127;
			if(i+header > img->width*img->height) return 0;
			memcpy(&img->pData[i],filePtr,4);
			filePtr += 4;
			i++;
			for(j=1 ; j<header ; j++,i++) img->pData[i] = img->pData[i-1];
		}
	}
	return 1;
}


int glBmpLoadTGA24(glBmpImage *img, FILE *tgaFile)
{
	int bytesRead;
	RGBTRIPLE *image;

	image = (RGBTRIPLE*)malloc(img->imageSize*sizeof(unsigned char));
	if (!image) return 0;

	bytesRead = fread(image, 1, img->imageSize, tgaFile);
	if (bytesRead != img->imageSize)
	{
		fclose(tgaFile);
		free(image);
		return 0;
	}

	img->pData = malloc(img->width*img->height*sizeof(GLRGBQUAD));
	if (!img->pData)
	{
		free(image);
		return 0;
	}

	glBmpExpandTo32(img, image, 0, 1);
	free(image);
	return 1;
}


int glBmpMemLoadTGA24(glBmpImage *img, char *filePtr)
{
	RGBTRIPLE *image;

	image = (RGBTRIPLE*)filePtr;
	filePtr += img->imageSize;
	img->pData = malloc(img->width*img->height*sizeof(GLRGBQUAD));
	if (!img->pData) return 0;
	glBmpExpandTo32(img, image, 0, 1);
	return 1;
}


int glBmpLoadCompressedTGA24(glBmpImage *img, FILE *tgaFile)
{
	RGBTRIPLE *image;
	int bytesRead;
	int i, j;
	unsigned char header;

	image = (RGBTRIPLE*)malloc(img->imageSize*sizeof(unsigned char));
	if (!image) return 0;

	i = 0;
	while(i < img->width*img->height)
	{
		bytesRead = fread(&header, 1, 1, tgaFile);
		if (bytesRead != 1)
		{
			fclose(tgaFile);
			free(image);
			return 0;
		}
		if(header < 128)
		{
			header++;
			bytesRead = fread(&image[i],1,header*3,tgaFile);
			if (bytesRead != header*3)
			{
				fclose(tgaFile);
				free(image);
				return 0;
			}
			i += header;
		}
		else
		{
			header -= 127;
			bytesRead = fread(&image[i],1,3,tgaFile);
			i++;
			if (bytesRead != 3)
			{
				fclose(tgaFile);
				free(image);
				return 0;
			}
			for(j=1 ; j<header ; j++,i++) image[i] = image[i-1];
		}
	}

	img->pData = malloc(img->width*img->height*sizeof(GLRGBQUAD));
	if (!img->pData)
	{
		free(image);
		return 0;
	}
	glBmpExpandTo32(img, image, 0, 1);
	free(image);
	return 1;
}


int glBmpMemLoadCompressedTGA24(glBmpImage *img, char *filePtr)
{
	RGBTRIPLE *image;
	int i, j;
	unsigned char header;

	image = (RGBTRIPLE*)malloc(img->imageSize*sizeof(unsigned char));
	if (!image) return 0;

	i = 0;
	while(i < img->width*img->height)
	{
		header = *((unsigned char*)filePtr);
		filePtr++;
		if(header < 128)
		{
			header++;
			if(i+header > img->width*img->height) return 0;
			memcpy(&image[i],filePtr,header*3);
			filePtr += header*3;
			i += header;
		}
		else
		{
			header -= 127;
			if(i+header > img->width*img->height) return 0;
			memcpy(&image[i],filePtr,3);
			filePtr += 3;
			i++;
			for(j=1 ; j<header ; j++,i++) image[i] = image[i-1];
		}
	}

	img->pData = malloc(img->width*img->height*sizeof(GLRGBQUAD));
	if (!img->pData)
	{
		free(image);
		return 0;
	}
	glBmpExpandTo32(img, image, 0, 1);
	free(image);
	return 1;
}


int glBmpSaveBMP(glBmpImage* img, char *filename)
{
	BITMAPFILEHEADER fHeader;
	BITMAPINFOHEADER iHeader;
	FILE *bmpFile;

	int bytesWritten;
	int length = img->width*img->height*4;

	if (!img->pData)
		return 0;

	if (!(bmpFile = fopen(filename, "wb")))
		return 0;

	fHeader.bfType = BM;
	fHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(iHeader) + length;
	fHeader.bfReserved1 = 0;
	fHeader.bfReserved2 = 0;

	// This sets the distance from the start of the file to the start
	// of the bitmaps color data
	fHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(iHeader);

	bytesWritten = fwrite(&fHeader, 1, sizeof(fHeader), bmpFile);
	if (bytesWritten != sizeof(fHeader))
	{
		fclose(bmpFile);
		return 0;
	}

	memset(&iHeader, 0, sizeof(iHeader));

	// Set up the information header
	iHeader.biSize = sizeof(BITMAPINFOHEADER);
	iHeader.biWidth = img->width;         // Current width
	iHeader.biHeight = img->height;       // Current height
	iHeader.biPlanes = 1;                 // Number of planes, must be set to 1
	iHeader.biBitCount = 32;              // Current color depth
	iHeader.biCompression = BI_RGB;       // No compression
	iHeader.biSizeImage = length;         // Number of bytes in bitmap

	bytesWritten = fwrite(&iHeader, 1, sizeof(iHeader), bmpFile);
	if (bytesWritten != sizeof(iHeader))
	{
		fclose(bmpFile);
		return 0;
	}

	glBmpSwapRB(img);

	bytesWritten = fwrite(img->pData, 1, length, bmpFile);
	if (bytesWritten != length)
	{
		fclose(bmpFile);
		return 0;
	}

	fclose(bmpFile);
	return 1;
}


int glBmpSaveTGA(glBmpImage* img, char *filename)
{
	TGAHEADER tHeader;
	FILE *tgaFile;

	int bytesWritten;
	int length = img->width*img->height*4;

	if (!img->pData)
		return 0;

	if (!(tgaFile = fopen(filename, "wb")))
		return 0;

	memset(&tHeader, 0, sizeof(tHeader));

	tHeader.tfImageType = TGA_RGB;
	tHeader.tfWidth[0] = img->width % 256;
	tHeader.tfWidth[1] = img->width / 256;
	tHeader.tfHeight[0] = img->height % 256;
	tHeader.tfHeight[1] = img->height / 256;
	tHeader.tfBpp = 32;

	bytesWritten = fwrite(&tHeader, 1, sizeof(tHeader), tgaFile);
	if (bytesWritten != sizeof(tHeader))
	{
		fclose(tgaFile);
		return 0;
	}

	glBmpSwapRB(img);

	bytesWritten = fwrite(img->pData, 1, length, tgaFile);
	if (bytesWritten != length)
	{
		fclose(tgaFile);
		return 0;
	}
	fclose(tgaFile);
	return 1;
}


int glBmpSaveScreen(glBmpImage* img)
{
	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);

	img->width = viewport[2];
	img->height = viewport[3];
	img->colorDepth = 32;

	glBmpClearMem(img);

	img->pData = malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData)
		return 0;

	glFinish();

	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	//glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	//glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	//glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

	glReadPixels(0, 0, viewport[2], viewport[3], GL_RGBA, GL_UNSIGNED_BYTE, img->pData);

	return 1;
}


void glBmpLoad32(glBmpImage* img, FILE *imgFile)
{
	int bytesRead;

	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData)
		return;

	bytesRead = fread(img->pData, 1, img->imageSize, imgFile);
	if (bytesRead != img->imageSize)
	{
		fclose(imgFile);
		free(img->pData);
		img->pData = 0;
		return;
	}

	glBmpSwapRB(img);
	glBmpSetAlpha(img, 255);
}


void glBmpMemLoad32(glBmpImage* img, char *imgPtr)
{
	if (!imgPtr)
		return;

	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData)
		return;

	memcpy(img->pData, imgPtr, img->imageSize);

	glBmpSwapRB(img);
	glBmpSetAlpha(img, 255);
}


void glBmpLoad24(glBmpImage* img, FILE *imgFile)
{
	RGBTRIPLE *bgrBits;
	int bytesRead;
	int pad;

	pad = glBmpGetLineWidth(img, 24) - (img->width*3);

	bgrBits = (RGBTRIPLE*)malloc(img->imageSize*sizeof(unsigned char));

	if (!bgrBits)
		return;

	bytesRead = fread(bgrBits, 1, img->imageSize, imgFile);
	if (bytesRead != img->imageSize)
	{
		fclose(imgFile);
		free(bgrBits);
		bgrBits = 0;
		return;
	}


	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData)
		return;

	glBmpExpandTo32(img, bgrBits, pad, 1);

	free(bgrBits);
	bgrBits = 0;
	img->colorDepth = 32;
}


void glBmpMemLoad24(glBmpImage* img, char *imgPtr)
{
	RGBTRIPLE *bgrBits;
	int pad;

	pad = glBmpGetLineWidth(img, 24) - (img->width*3);

	bgrBits = (RGBTRIPLE*)imgPtr;

	if (!bgrBits)
		return;

	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData)
		return;

	glBmpExpandTo32(img, bgrBits, pad, 1);

	bgrBits = 0;
	img->colorDepth = 32;
}


void glBmpLoad16(glBmpImage* img, FILE *imgFile)
{
	WORD *bgrBits;
	int bytesRead;
	int pad;

	int redBits, greenBits, blueBits;
	int redShr, greenShr;

	int x, y;

	WORD *source16;
	GLRGBQUAD *dest;

	bgrBits = (WORD*)malloc(img->imageSize*sizeof(unsigned char));

	if (!bgrBits)
		return;

	bytesRead = fread(bgrBits, 1, img->imageSize, imgFile);
	if (bytesRead != img->imageSize)
	{
		fclose(imgFile);
		free(bgrBits);
		bgrBits = 0;
		return;
	}

	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData)
		return;

	pad = glBmpGetLineWidth(img, 16) - (img->width*2);

	redShr = GetRightShiftCount(img->redMask);
	greenShr = GetRightShiftCount(img->greenMask);

	redBits = GetNumSet(img->redMask);
	greenBits = GetNumSet(img->greenMask);
	blueBits = GetNumSet(img->blueMask);

	source16 = bgrBits;
	dest = img->pData;

	for (x = 0; x < img->height; x++)
	{
		for (y = 0; y < img->width; y++)
		{
			dest->red = Scale8((*source16 & (int)img->redMask) >> redShr, redBits);
			dest->green = Scale8((*source16 & (int)img->greenMask) >> greenShr, greenBits);
			dest->blue = Scale8(*source16 & (int)img->blueMask, blueBits);
			dest->alpha = 255;
			source16++;
			dest++;
		}
		source16 = (WORD*)((BYTE*)source16 + pad);
	}

	free(bgrBits);
	bgrBits = 0;
	img->colorDepth = 32;
}


void glBmpMemLoad16(glBmpImage* img, char *imgPtr)
{
	WORD *bgrBits;
	int pad;

	int redBits, greenBits, blueBits;
	int redShr, greenShr;

	int x, y;

	WORD *source16;
	GLRGBQUAD *dest;

	bgrBits = (WORD*)imgPtr;

	if (!bgrBits)
		return;

	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData)
		return;

	pad = glBmpGetLineWidth(img, 16) - (img->width*2);

	redShr = GetRightShiftCount(img->redMask);
	greenShr = GetRightShiftCount(img->greenMask);

	redBits = GetNumSet(img->redMask);
	greenBits = GetNumSet(img->greenMask);
	blueBits = GetNumSet(img->blueMask);

	source16 = bgrBits;
	dest = img->pData;

	for (x = 0; x < img->height; x++)
	{
		for (y = 0; y < img->width; y++)
		{
			dest->red = Scale8((*source16 & (int)img->redMask) >> redShr, redBits);
			dest->green = Scale8((*source16 & (int)img->greenMask) >> greenShr, greenBits);
			dest->blue = Scale8(*source16 & (int)img->blueMask, blueBits);
			dest->alpha = 255;
			source16++;
			dest++;
		}
		source16 = (WORD*)((BYTE*)source16 + pad);
	}

	bgrBits = 0;
	img->colorDepth = 32;
}


void glBmpLoad8(glBmpImage* img, FILE *imgFile)
{
	BYTE *bgrBits;
	int bytesRead;
	BYTE *source8;
	GLRGBQUAD *dest;

	int x, y;
	int pad;

	bgrBits = (BYTE*)malloc(img->imageSize*sizeof(BYTE));

	if (!bgrBits)
		return;

	bytesRead = fread(bgrBits, 1, img->imageSize, imgFile);
	if (bytesRead != img->imageSize)
	{
		fclose(imgFile);
		free(bgrBits);
		bgrBits = 0;
		return;
	}

	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData)
		return;

	pad = glBmpGetLineWidth(img, 8) - img->width;

	source8 = bgrBits;
	dest = img->pData;

	for (x = 0; x < img->height; x++)
	{
		for (y = 0; y < img->width; y++)
		{
			dest->red = img->colors[*source8].rgbRed;
			dest->green = img->colors[*source8].rgbGreen;
			dest->blue = img->colors[*source8].rgbBlue;
			dest->alpha = 255;
			dest++;
			source8++;
		}
		source8 += pad;
	}

	free(bgrBits);
	bgrBits = 0;

	img->colorDepth = 32;
}


void glBmpMemLoad8(glBmpImage* img, char *imgPtr)
{
	BYTE *bgrBits;
	BYTE *source8;
	GLRGBQUAD *dest;

	int x, y;
	int pad;

	bgrBits = (BYTE*)imgPtr;

	if (!bgrBits)
		return;

	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData)
		return;

	pad = glBmpGetLineWidth(img, 8) - img->width;

	source8 = bgrBits;
	dest = img->pData;

	for (x = 0; x < img->height; x++)
	{
		for (y = 0; y < img->width; y++)
		{
			dest->red = img->colors[*source8].rgbRed;
			dest->green = img->colors[*source8].rgbGreen;
			dest->blue = img->colors[*source8].rgbBlue;
			dest->alpha = 255;
			dest++;
			source8++;
		}
		source8 += pad;
	}

	bgrBits = 0;

	img->colorDepth = 32;
}


void glBmpLoad4(glBmpImage* img, FILE *imgFile)
{
	BYTE *bgrBits;
	int bytesRead;
	BYTE *source4;
	GLRGBQUAD *dest;
	int palEntry;

	int x, y;
	int pad;

	bgrBits = (BYTE*)malloc(img->imageSize*sizeof(BYTE));

	if (!bgrBits)
		return;

	bytesRead = fread(bgrBits, 1, img->imageSize, imgFile);
	if (bytesRead != img->imageSize)
	{
		fclose(imgFile);
		free(bgrBits);
		bgrBits = 0;
		return;
	}

	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData)
		return;

	pad = glBmpGetLineWidth(img, 4) - (img->width >> 1);

	source4 = bgrBits;
	dest = img->pData;

	for (x = 0; x < img->height; x++)
	{
		for (y = 0; y < img->width; y++)
		{
			if (y % 2)
				palEntry = (int)(*source4 & 0xF);
			else
				palEntry = (int)((*source4 >> 4) & 0xF);

			dest->red = img->colors[palEntry].rgbRed;
			dest->green = img->colors[palEntry].rgbGreen;
			dest->blue = img->colors[palEntry].rgbBlue;
			dest->alpha = 255;
			dest++;

			if (y % 2)
				source4++;
		}
		source4 += pad;
	}

	free(bgrBits);
	bgrBits = 0;
	img->colorDepth = 32;
}


void glBmpMemLoad4(glBmpImage* img, char *imgPtr)
{
	BYTE *bgrBits;
	BYTE *source4;
	GLRGBQUAD *dest;
	int palEntry;

	int x, y;
	int pad;

	bgrBits = (BYTE*)imgPtr;

	if (!bgrBits)
		return;

	img->pData = (GLRGBQUAD*)malloc(img->width*img->height*sizeof(GLRGBQUAD));

	if (!img->pData)
		return;

	pad = glBmpGetLineWidth(img, 4) - (img->width >> 1);

	source4 = bgrBits;
	dest = img->pData;

	for (x = 0; x < img->height; x++)
	{
		for (y = 0; y < img->width; y++)
		{
			if (y % 2)
				palEntry = (int)(*source4 & 0xF);
			else
				palEntry = (int)((*source4 >> 4) & 0xF);

			dest->red = img->colors[palEntry].rgbRed;
			dest->green = img->colors[palEntry].rgbGreen;
			dest->blue = img->colors[palEntry].rgbBlue;
			dest->alpha = 255;
			dest++;

			if (y % 2)
				source4++;
		}
		source4 += pad;
	}

	bgrBits = 0;
	img->colorDepth = 32;
}


int glBmpFlipVert(glBmpImage* img)
{
	int i;
	GLRGBQUAD *top;
	GLRGBQUAD *bottom;
	GLRGBQUAD *tmpBits;
	int lineWidth = img->width * 4;

	if (!img->pData)
	return 0;

	tmpBits = (GLRGBQUAD*)malloc(img->width*sizeof(GLRGBQUAD));

	if (!tmpBits)
		return 0;

	top = img->pData;
	bottom = (GLRGBQUAD*)((BYTE*)img->pData + lineWidth*(img->height-1));

	for (i = 0; i < (img->height >> 1); i++)
	{
		memcpy(tmpBits, top, lineWidth);
		memcpy(top, bottom, lineWidth);
		memcpy(bottom, tmpBits, lineWidth);

		top = (GLRGBQUAD*)((BYTE*)top + lineWidth);
		bottom = (GLRGBQUAD*)((BYTE*)bottom - lineWidth);
	}

	free(tmpBits);
	tmpBits = 0;

	return 1;
}


int glBmpFlipHorz(glBmpImage* img)
{
	int x, y;
	GLRGBQUAD *front;
	GLRGBQUAD *back;
	GLRGBQUAD tmp;

	int line = img->width * 4;

	if (!img->pData)
		return 0;

	for (y = 0; y < img->height; y++)
	{
		front = (GLRGBQUAD*)((int)img->pData + line*y);
		back = (GLRGBQUAD*)((int)img->pData + (line*y) + (line-4));

		for (x = 0; x < (img->width >> 1)-1; x++)
		{
			tmp = *front;
			*front = *back;
			*back = tmp;

			front++;
			back--;
		}
	}
	return 1;
}


int glBmpRotate180(glBmpImage* img)
{
  if (!glBmpFlipHorz(img))
	return 0;

  if (!glBmpFlipVert(img))
	return 0;

  return 1;
}


void glBmpGenTexture(glBmpImage* img)
{
	if (img->texID) {
		//glDeleteTextures(1, &img->texID);
	}

	glGenTextures(1, &img->texID);
	glBindTexture(GL_TEXTURE_2D, img->texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, img->texWrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, img->texWrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, img->magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, img->minFilter);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pData);
}

gluBuild2DMipmaps(GLenum target, GLint internalFormat, 
			     GLsizei width, GLsizei height, 
			     GLenum format, GLenum type, 
			     GLubyte *texture) {

	int level = 0;

	assert(format == GL_RGBA);
	assert(internalFormat == GL_RGBA);
	assert(type == GL_UNSIGNED_BYTE);

	// OK, for simplicity, assume square bitmap
	glTexImage2D(target, level, internalFormat, width, height, 0, format, type, texture);

	while (width > 1 && height > 1) {
		GLsizei row, col;

		width = width >> 1;
		height = height >> 1;
		++level;

		for(row = 0; row < height; ++row)
		{
			for(col = 0; col < width; ++col)
			{
				const GLubyte * p00 = texture + 4 * (2 * row * (width * 2) + 2 * col);
				const GLubyte * p01 = texture + 4 * (2 * row * (width * 2) + 2 * col + 1);
				const GLubyte * p10 = texture + 4 * ((2 * row + 1) * (width * 2) + 2 * col);
				const GLubyte * p11 = texture + 4 * ((2 * row + 1) * (width * 2) + 2 * col + 1);

				GLubyte * dst = texture + 4 * (row * width + col);

				dst[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
				dst[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
				dst[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
				dst[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
			}
		}

		glTexImage2D(target, level, internalFormat, width, height, 0, format, type, texture);
	}

	while (width > 1) {
		GLsizei row, col;

		width = width >> 1;
		++level;

		for(row = 0; row < height; ++row)
		{
			for(col = 0; col < width; ++col)
			{
				const GLubyte * p00 = texture + 4 * (row * (width * 2) + 2 * col);
				const GLubyte * p01 = texture + 4 * (row * (width * 2) + 2 * col + 1);

				GLubyte * dst = texture + 4 * (row * width + col);

				dst[0] = (p00[0] + p01[0]) / 2;
				dst[1] = (p00[1] + p01[1]) / 2;
				dst[2] = (p00[2] + p01[2]) / 2;
				dst[3] = (p00[3] + p01[3]) / 2;
			}
		}

		glTexImage2D(target, level, internalFormat, width, height, 0, format, type, texture);
	}

	while (height > 1) {
		GLsizei row, col;

		height = height >> 1;
		++level;

		for(row = 0; row < height; ++row)
		{
			for(col = 0; col < width; ++col)
			{
				const GLubyte * p00 = texture + 4 * (2 * row * width + 2 * col);
				const GLubyte * p10 = texture + 4 * ((2 * row + 1) * width + 2 * col);

				GLubyte * dst = texture + 4 * (row * width + col);

				dst[0] = (p00[0] + p10[0]) / 2;
				dst[1] = (p00[1] + p10[1]) / 2;
				dst[2] = (p00[2] + p10[2]) / 2;
				dst[3] = (p00[3] + p10[3]) / 2;
			}
		}

		glTexImage2D(target, level, internalFormat, width, height, 0, format, type, texture);
	}
}

void glBmpGenTextureMipMap(glBmpImage* img)
{
	if (img->texID) {
		//glDeleteTextures(1, &img->texID);
	}

	glGenTextures(1, &img->texID);
	glBindTexture(GL_TEXTURE_2D, img->texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, img->texWrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, img->texWrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, img->magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, img->minFilter);

	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, img->width, img->height, GL_RGBA, GL_UNSIGNED_BYTE, (GLubyte *) img->pData);
}


void glBmpSetFilter(glBmpImage* img, GLint min, GLint mag)
{
	switch (min)
	{
		case GL_NEAREST : img->minFilter = GL_NEAREST; break;
		case GL_LINEAR : img->minFilter = GL_LINEAR; break;
		case GL_NEAREST_MIPMAP_NEAREST : img->minFilter = GL_NEAREST_MIPMAP_NEAREST; break;
		case GL_LINEAR_MIPMAP_NEAREST : img->minFilter = GL_LINEAR_MIPMAP_NEAREST; break;
		case GL_NEAREST_MIPMAP_LINEAR : img->minFilter = GL_NEAREST_MIPMAP_LINEAR; break;
		case GL_LINEAR_MIPMAP_LINEAR : img->minFilter = GL_LINEAR_MIPMAP_LINEAR; break;
		default : img->minFilter = GL_LINEAR;
	}
	switch (mag)
	{
		case GL_NEAREST : img->magFilter = GL_NEAREST; break;
		case GL_LINEAR : img->magFilter = GL_LINEAR; break;
		default : img->magFilter = GL_LINEAR;
	}
}


void glBmpSetTextureWrap(glBmpImage* img, GLint s, GLint t)
{
	switch (s)
	{
		//case GL_CLAMP : img->texWrapS = GL_CLAMP; break;
		case GL_REPEAT : img->texWrapS = GL_REPEAT; break;
		case GL_CLAMP_TO_EDGE : img->texWrapS = GL_CLAMP_TO_EDGE; break;
	}
	switch (t)
	{
		//case GL_CLAMP : img->texWrapT = GL_CLAMP; break;
		case GL_REPEAT : img->texWrapT = GL_REPEAT; break;
		case GL_CLAMP_TO_EDGE : img->texWrapT = GL_CLAMP_TO_EDGE; break;
	}
}


void glBmpBind(glBmpImage* img)
{
	if (img->texID)
		glBindTexture(GL_TEXTURE_2D, img->texID);
}


void glBmpSwapRB(glBmpImage* img)
{
	int i;
	GLRGBQUAD *pixel;
	unsigned char tmp;

	pixel = img->pData;

	for (i = 0; i < (img->width*img->height); i++)
	{
		tmp = pixel->blue;
		pixel->blue = pixel->red;
		pixel->red = tmp;

		pixel++;
	}
}


void glBmpSetAlpha(glBmpImage* img, BYTE alpha)
{
	int i;
	GLRGBQUAD *pixel;

	pixel = img->pData;
	for (i = 0; i < (img->width*img->height); i++)
	{
		pixel->alpha = alpha;
		pixel++;
	}
}


void glBmpExpandTo32(glBmpImage* img, RGBTRIPLE *image, int pad, int swapRB)
{
	int x, y;
	RGBTRIPLE *pixel24;
	GLRGBQUAD *pixel32;

	pixel24 = image;
	pixel32 = img->pData;

	for (x = 0; x < img->height; x++)
	{
		for (y = 0; y < img->width; y++)
		{
			if (swapRB)
			{
				pixel32->red = pixel24->rgbtRed;
				pixel32->blue = pixel24->rgbtBlue;
			}
			else
			{
				pixel32->blue = pixel24->rgbtRed;
				pixel32->red = pixel24->rgbtBlue;
			}

			pixel32->green = pixel24->rgbtGreen;
			pixel32->alpha = 255;

			pixel24++;
			pixel32++;
		}
		pixel24 = (RGBTRIPLE*)((BYTE*)pixel24 + pad);
	}
}


void glBmpFreeTexture(glBmpImage* img)
{
	glBmpClearMem(img);
	if (img->texID)
	{
		//glDeleteTextures(1, &img->texID);
		img->texID = 0;
	}
}
