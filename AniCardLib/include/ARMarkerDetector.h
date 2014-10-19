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
};
class ANICARDLIB_SHARED ARMarkerDetector
{
	long width , height;
	unsigned char* copiedPictureInstance;
	unsigned char* grayscaleImage;
	float* gradientDirection;
	float* gradientIntensity;
	std::future<void>* runningThread;
	ARMarkerDetector() : copiedPictureInstance(0), grayscaleImage(0), gradientIntensity(0), runningThread(0), gradientDirection(0), width(0), height(0) {}
	void findLines(std::vector<Line>& linesToAdd );
	std::vector<Line> findLinesOnRegion(long x , long y , long width , long height);
	void _findCard();
public:
	void findCard( PictureFetcher* picture );
	static ARMarkerDetector global;
};