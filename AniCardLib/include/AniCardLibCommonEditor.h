#pragma once
#include <AniCardLibExportHeader.h>
class MarkerPack;
struct GeometryInfo;
struct TextureInfo;
struct Marker;
class ANICARDLIB_SHARED AniCardLibCommonEditor
{
	MarkerPack* markerPack;
public:
	AniCardLibCommonEditor();
	~AniCardLibCommonEditor();
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
	bool cardToTexture( TextureInfo* target , const unsigned int& id );
	bool save( const char* fileName );
	void load( const char* fileName );
};