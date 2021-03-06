#pragma once
#include <AniCardLibExportHeader.h>
#include <Graphics\GraphicsBufferManager.h>
#include <Graphics\GraphicsGeometryManager.h>
#include <Graphics\GraphicsTextureManager.h>
#include <vector>
struct Marker;

/*
numCards;
numModels;
numTextures;
sizeOfCardImageData;
sizeOfModelData;
sizeOfTextureData;
cardData;
cardImageData;
sizeOfModel
modelData
textureDimension
textureData
*/
class ANICARDLIB_SHARED AniCardLibFileInfo
{
	unsigned int numCards;
	unsigned int numModels;
	unsigned int numTextures;
	unsigned int sizeOfCardImageData;
	unsigned int sizeOfModelData;
	unsigned int sizeOfTextureData;

	void* cardData;
	void* cardImageData;

	GraphicsBufferManager bufferManager;
	GraphicsGeometryManager geometryManager;
	GraphicsTextureManager textureManager;
	std::vector<GeometryInfo*> geos;
	std::vector<TextureInfo*> textures;
public:
	AniCardLibFileInfo();
	AniCardLibFileInfo( const char* fileName );
	~AniCardLibFileInfo();

	int addMarker( const char* fileName , const int& linkedModel = -1 , const int& linkedTexture = -1 );
	int addModel( const char* fileName , const int& cardToLink = -1 );
	int addRawModel( const char* data , const int& cardToLink = -1 );
	int addTexture( const char* fileName , const int& cardToLink = -1 );
	int addTexture(const char* data, const unsigned int& width, const unsigned int& height, const int& cardToLink = -1);
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
};