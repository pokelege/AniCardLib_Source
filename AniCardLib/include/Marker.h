#pragma once
#include <AniCardLibExportHeader.h>
struct ANICARDLIB_SHARED Marker
{
	unsigned int width;
	unsigned int height;
	int linkedModel;
	int linkedTexture;
	unsigned int imageOffset;
	Marker() : width(0) , height(0) , linkedModel(-1), linkedTexture(-1) , imageOffset(0) {}
};