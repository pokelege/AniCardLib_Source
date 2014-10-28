#pragma once
#include <CommonWarningDisables.h>
#include <AniCardLibExportHeader.h>
#include <glm.hpp>
#include <vector>
#include <future>
class PictureFetcher;
struct EdgelInfo
{
	glm::ivec2 pos;
	float gradientIntensity;
	float angle;
};

struct EdgelInList
{
	EdgelInfo edgel;
	bool deletion;
	EdgelInList() :deletion(false) {}
};

struct Line
{
	glm::ivec2 start;
	glm::ivec2 end;
	float angle;
	bool deletion;
	Line() : deletion(false) {}
};

struct ConstructingQuad
{
	Line* line[4];
	ConstructingQuad()
	{
		for ( unsigned int i = 0; i < 4; ++i ) line[i] = 0;
	}
};

struct Quad
{
	glm::vec2 pt[4];
	glm::mat4 transform;
	unsigned int markerID;
};

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

	bool canGrabPositions;
	bool canGrabMatrices;
	unsigned int numUsingPos;
	unsigned int numUsingMat;
	std::vector<glm::vec2> toSend;
	std::vector<glm::mat4> matrices;
	ARMarkerDetector() : copiedPictureInstance(0), grayscaleImage(0), gradientIntensity(0), runningThread(0), gradientDirection(0), width(0), height(0), canGrab(false), numUsing(0), canGrabMatrices(false), canGrabPositions(false), numUsingPos(0), numUsingMat(0) {}
	void findLines(std::vector<Line>& linesToAdd );
	std::vector<Line> findLinesOnRegion(long x , long y , long width , long height);
	void _findCard( );
	bool findQuad( ConstructingQuad& quadToEdit , std::vector<Line>& lines , unsigned int index );
public:
	void findCard( PictureFetcher* picture );
	bool getPicture( unsigned char** bytes , long* width = 0 , long* height = 0 );
	bool finishedUsing();
	bool getPositions( std::vector<glm::vec2>** bytes);
	bool finishedUsingPos();
	bool getMatrices( std::vector<glm::mat4>** bytes );
	bool finishedUsingMat();
	static ARMarkerDetector global;
};