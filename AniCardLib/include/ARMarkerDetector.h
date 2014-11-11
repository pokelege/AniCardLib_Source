#pragma once
#include <CommonWarningDisables.h>
#include <AniCardLibExportHeader.h>
#include <glm.hpp>
#include <vector>
#include <future>
class PictureFetcher;
struct ANICARDLIB_SHARED EdgelInfo
{
	glm::ivec2 pos;
	float gradientIntensity;
	float angle;
};

struct ANICARDLIB_SHARED EdgelInList
{
	EdgelInfo edgel;
	bool deletion;
	EdgelInList() :deletion(false) {}
};

struct ANICARDLIB_SHARED Line
{
	glm::ivec2 start;
	glm::ivec2 end;
	float angle;
	bool deletion;
	Line() : deletion(false) {}
};

struct ANICARDLIB_SHARED ConstructingQuad
{
	Line* line[4];
	ConstructingQuad()
	{
		for ( unsigned int i = 0; i < 4; ++i ) line[i] = 0;
	}
};

struct ANICARDLIB_SHARED Quad
{ 
	glm::vec2 pt[4];
	glm::mat4 transform;
	unsigned long dissimilarity;
	unsigned int markerID;
};

struct ANICARDLIB_SHARED FoundMarkerInfo
{
	glm::mat4 transform;
	glm::vec2 center;
	unsigned long dissimilarity;
	unsigned int cardIndex;
};
class MarkerPack;
class ANICARDLIB_SHARED ARMarkerDetector
{
	unsigned char* grayscaleImage;
	float* gradientDirection;
	float* gradientIntensity;
	std::future<void>* runningThread;
	unsigned int numUsing;
	bool canGrab;

	long width , height;
	unsigned char* copiedPictureInstance;

	bool canGrabMarkerFound;
	unsigned int numUsingMarkerFound;
	std::vector<FoundMarkerInfo> toSend;
	ARMarkerDetector() : copiedPictureInstance( 0 ) , grayscaleImage( 0 ) , gradientIntensity( 0 ) , runningThread( 0 ) , gradientDirection( 0 ) , width( 0 ) , height( 0 ) , canGrab( false ) , numUsing( 0 ) , canGrabMarkerFound( false ) , numUsingMarkerFound( 0 ) {}
	void findLines(std::vector<Line>& linesToAdd );
	std::vector<Line> findLinesOnRegion(long x , long y , long width , long height);
	void _findCard( MarkerPack* markerPack );
	bool findQuad( ConstructingQuad& quadToEdit , std::vector<Line>& lines , unsigned int index );
public:
	void findCard( PictureFetcher* picture, MarkerPack* markerPack );
	bool getPicture( unsigned char** bytes , long* width = 0 , long* height = 0 );
	bool finishedUsing();
	bool getMarkerFound( std::vector<FoundMarkerInfo>** bytes );
	bool finishedUsingMarkerFound();
	static ARMarkerDetector global;
};

bool dissimilarityCompare( FoundMarkerInfo i , FoundMarkerInfo j );