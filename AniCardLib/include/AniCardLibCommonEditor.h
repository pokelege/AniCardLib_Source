#pragma once
#include <AniCardLibExportHeader.h>
#include <string>
#include <vector>
class MarkerPack;
struct GeometryInfo;
struct TextureInfo;
struct Marker;
class AniCardLibFileNamesInfo;
class ANICARDLIB_SHARED AniCardLibCommonEditor
{
	MarkerPack* markerPack;
	AniCardLibFileNamesInfo* names;
public:
	AniCardLibCommonEditor();
	~AniCardLibCommonEditor();
	int addMarker( const char* fileName , const int& linkedModel = -1 , const int& linkedTexture = -1 );
	int addModel( const char* fileName , const int& cardToLink = -1 );
	int addRawModel( const char* data , const int& cardToLink = -1 );
	int addTexture( const char* fileName , const int& cardToLink = -1 );
	int addTexture( const char* data , const unsigned int& width , const unsigned int& height , const int& cardToLink = -1 );
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
	bool save( const char* fileName , const bool& saveNames = false , const std::vector<std::string>& cards = std::vector<std::string>() , const std::vector<std::string>& models = std::vector<std::string>() , const std::vector<std::string>& textures = std::vector<std::string>() );
	void load( const char* fileName, const bool& loadNames = false );


	std::string getCardName( const unsigned int& index );
	unsigned int getNumCards();
	std::string getModelName( const unsigned int& index );
	unsigned int getNumModels();
	std::string getTextureName( const unsigned int& index );
	unsigned int getNumTextures();
};