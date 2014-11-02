#include <AniCardLibExportHeader.h>
#include <Graphics\GraphicsBufferManager.h>
#include <Graphics\GraphicsGeometryManager.h>
#include <vector>
struct Marker;
class ANICARDLIB_SHARED AniCardLibFileInfo
{
	unsigned int numCards;
	unsigned int numModels;
	unsigned int sizeOfCardImageData;
	unsigned int sizeOfModelData;

	void* cardData;
	void* cardImageData;

	GraphicsBufferManager bufferManager;
	GraphicsGeometryManager geometryManager;
	std::vector<GeometryInfo*> geos;
	AniCardLibFileInfo() : cardData(0) , cardImageData(0) {}
	AniCardLibFileInfo( const char* fileName );
	~AniCardLibFileInfo();

	int addMarker( const char* fileName , const int& linkedModel = -1 );
	int addModel( const char* fileName , const int& cardToLink = -1 );
	Marker* getMarker( const unsigned int& id );
	unsigned int getMarkerListSize();
	GeometryInfo* getGeometry( const unsigned int& id );
	unsigned int getGeometryListSize();

	bool save( const char* fileName );
};