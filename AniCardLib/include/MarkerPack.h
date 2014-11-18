#pragma once
#include <vector>
#include <AniCardLibExportHeader.h>
#include <ARMarkerDetector.h>
#include <glm.hpp>
#include <AniCardLibFileInfo.h>
class ANICARDLIB_SHARED MarkerPack
{
	struct CompareWithMarkerInfo
	{
		unsigned int marker;
		const unsigned char* picture;
		long pictureWidth;
		long pictureHeight;
		unsigned int AsID;
		glm::vec2 pos[4];
	};

	struct FoundMarkerInfo
	{
		unsigned long dissimilarity;
		unsigned int markerID;
		glm::vec2 points[4];
	};
	AniCardLibFileInfo* cards;
	unsigned char threshold;
	float* theAs[4];
	unsigned char* debugPicture;
	long width , height;
	unsigned int numUsing;
	bool canGrab;
	FoundMarkerInfo getMarkerCornerDissimilarity( CompareWithMarkerInfo info);
	FoundMarkerInfo getMarkerDissimilarity( CompareWithMarkerInfo info );
	FoundMarkerInfo getSmallestDissimilarity( CompareWithMarkerInfo info );
	static unsigned long highestDissimilarity;
public:
	MarkerPack();
	int addMarker( const char* fileName , const int& linkedModel = -1 , const int& linkedTexture = -1 );
	int addModel( const char* fileName , const int& cardToLink = -1 );
	int addTexture( const char* fileName , const int& cardToLink = -1 );
	Marker* getMarker( const unsigned int& id );
	unsigned int getMarkerListSize();
	GeometryInfo* getGeometry( const unsigned int& id );
	GeometryInfo* getCardGeometry( const unsigned int& id );
	unsigned int getGeometryListSize();
	TextureInfo* getTexture( const unsigned int& id );
	TextureInfo* getCardTexture( const unsigned int& id );
	unsigned int getTextureListSize();

	unsigned char* getPicturePointer( const unsigned int& id );

	bool save( const char* fileName );
	void load( const char* fileName );

	bool matchMarker( Quad& quad, const unsigned char* picture, long pictureWidth, long pictureHeight );
	bool getPicture( unsigned char** bytes , long* width = 0 , long* height = 0 );
	bool finishedUsing();
};