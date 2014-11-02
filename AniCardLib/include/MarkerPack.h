#pragma once
#include <vector>
#include <AniCardLibExportHeader.h>
#include <ARMarkerDetector.h>
#include <glm.hpp>
#include <Marker.h>

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

	struct FoundMarkerInfo
	{
		unsigned long dissimilarity;
		unsigned int markerID;
		glm::vec2 points[4];
		float theAs[8];
	};

	std::vector<Marker> markers;
	unsigned char* debugPicture;
	long width , height;
	unsigned int numUsing;
	bool canGrab;
	FoundMarkerInfo getMarkerCornerDissimilarity( CompareWithMarkerInfo info);
	FoundMarkerInfo getMarkerDissimilarity( CompareWithMarkerInfo info );
	FoundMarkerInfo getSmallestDissimilarity( CompareWithMarkerInfo info );
public:
	MarkerPack() : debugPicture( 0 ) {}
	void addMarker( const char* file );
	void loadMarkerPack( const char* file );
	bool matchMarker( Quad& quad, const unsigned char* picture, long pictureWidth, long pictureHeight );
	bool getPicture( unsigned char** bytes , long* width = 0 , long* height = 0 );
	bool finishedUsing();
	static MarkerPack global;
};