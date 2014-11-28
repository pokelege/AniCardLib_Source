#pragma once
#include <AniCardLibExportHeader.h>
#include <WebCamHelpers.h>
#include <ARMarkerDetector.h>
class MarkerPack;
class WebCamSource;
struct GeometryInfo;
struct TextureInfo;
class ANICARDLIB_SHARED AniCardLibCommonGame
{
	
	
	WebCamSource* webCam;
public:
	MarkerPack* markerPack;
	ARMarkerDetector* markerDetector;
	AniCardLibCommonGame();
	~AniCardLibCommonGame();
	void setMarkerPack( const char* filename );
	void update();
	std::vector<FoundMarkerInfo> queryResultList();
	std::vector<CameraItem> getAvailableCameras();
	void setCamera( CameraItem& camera , CameraMode& mode );
	unsigned int getGeometryListSize();
	GeometryInfo* getGeometry(unsigned int& id);
	GeometryInfo* getCardGeometry( unsigned int& id );
	TextureInfo* getCardTexture( unsigned int& id );
	bool copyPicture(TextureInfo* texture , long* width = 0 , long* height = 0 );
};