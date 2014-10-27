#pragma once
#include <vector>
#include <AniCardLibExportHeader.h>
#include <ARMarkerDetector.h>
struct Marker
{
	unsigned char* bytes;
	int width;
	int height;
	int channels;
};

class ANICARDLIB_SHARED MarkerPack
{
	std::vector<Marker> markers;
	unsigned char* debugPicture;
	long width , height;
	unsigned int numUsing;
	bool canGrab;
public:
	MarkerPack() : debugPicture( 0 ) {}
	void addMarker( const char* file );
	bool matchMarker( Quad& quad, const unsigned char* picture, long pictureWidth, long pictureHeight );
	bool getPicture( unsigned char** bytes , long* width = 0 , long* height = 0 );
	bool finishedUsing();
	static MarkerPack global;
};