#pragma once
#include <vector>
#include <AniCardLibExportHeader.h>
#include <ARMarkerDetector.h>
#include <glm.hpp>
struct Marker
{
	unsigned char* bytes;
	int width;
	int height;
};

class ANICARDLIB_SHARED MarkerPack
{
	struct CompareWithMarkerInfo
	{
		unsigned int marker;
		const unsigned char* picture;
		long pictureWidth;
		long pictureHeight;
		glm::vec2 pos[4];
	};

	std::vector<Marker> markers;
	unsigned char* debugPicture;
	long width , height;
	unsigned int numUsing;
	bool canGrab;
	unsigned long compareWithMarker( CompareWithMarkerInfo info);
public:
	MarkerPack() : debugPicture( 0 ) {}
	void addMarker( const char* file );
	bool matchMarker( Quad& quad, const unsigned char* picture, long pictureWidth, long pictureHeight );
	bool getPicture( unsigned char** bytes , long* width = 0 , long* height = 0 );
	bool finishedUsing();
	static MarkerPack global;
};