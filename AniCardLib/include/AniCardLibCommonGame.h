#pragma once
#include <AniCardLibExportHeader.h>
class MarkerPack;
class ARMarkerDetector;
class WebCamSource;
class ANICARDLIB_SHARED AniCardLibCommonGame
{
	MarkerPack* markerPack;
	ARMarkerDetector* markerDetector;
	WebCamSource* webCam;
public:
	AniCardLibCommonGame();
	~AniCardLibCommonGame();
	void setMarkerPack( const char* filename );
	void update();
	void queryResultList();

};