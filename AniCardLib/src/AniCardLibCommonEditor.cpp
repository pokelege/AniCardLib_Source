#include <AniCardLibCommonEditor.h>
#include <MarkerPack.h>
#include <Graphics\TextureInfo.h>
#include <Marker.h>
#include <AniCardLibFileNamesInfo.h>
AniCardLibCommonEditor::AniCardLibCommonEditor() :markerPack(0) , names(0)
{
	markerPack = new MarkerPack;
}

AniCardLibCommonEditor::~AniCardLibCommonEditor()
{
	if ( markerPack ) delete markerPack;
	if ( names ) delete names;
}

void AniCardLibCommonEditor::load( const char* filename , const bool& loadNames )
{
	if ( markerPack ) delete markerPack;
	markerPack = new MarkerPack();
	markerPack->load( filename );
	if ( loadNames )
	{
		if ( !names ) names = new AniCardLibFileNamesInfo;
		names->load( ( std::string( filename ) + ".aclfn" ).c_str() );
	}
	else if ( names )
	{
		delete names;
		names = 0;
	}
}

bool AniCardLibCommonEditor::save( const char* fileName , const bool& saveNames , const std::vector<std::string>& cards , const std::vector<std::string>& models, const std::vector<std::string>& textures )
{
	if ( !markerPack ) return false;
	bool toReturn = markerPack->save( fileName );
	if ( saveNames )
	{
		if ( toReturn )
		{
			if ( !names ) names = new AniCardLibFileNamesInfo;
			names->save( ( std::string( fileName ) + ".aclfn" ).c_str(), cards, models, textures );
		}
		else if ( names )
		{
			delete names;
			names = 0;
		}
	}
	else if ( names )
	{
		delete names;
		names = 0;
	}
	return toReturn;
}

int AniCardLibCommonEditor::addMarker( const char* fileName , const int& linkedModel, const int& linkedTexture)
{
	if ( !markerPack ) return -1;
	return markerPack->addMarker( fileName , linkedModel , linkedTexture );
}
int AniCardLibCommonEditor::addModel( const char* fileName , const int& cardToLink )
{
	if ( !markerPack ) return -1;
	return markerPack->addModel( fileName , cardToLink );
}
int AniCardLibCommonEditor::addTexture( const char* fileName , const int& cardToLink )
{
	if ( !markerPack ) return -1;
	return markerPack->addTexture( fileName , cardToLink );
}

int AniCardLibCommonEditor::addTexture( const char* data , const unsigned int& width , const unsigned int& height , const int& cardToLink)
{
	if ( !markerPack ) return -1;
	return markerPack->addTexture( data , width , height , cardToLink );
}

Marker* AniCardLibCommonEditor::getMarker( const unsigned int& id )
{
	if ( !markerPack ) return 0;
	return markerPack->getMarker( id );
}

unsigned int AniCardLibCommonEditor::getMarkerListSize()
{
	if ( !markerPack ) return 0;
	return markerPack->getMarkerListSize();
}

unsigned int AniCardLibCommonEditor::getGeometryListSize()
{
	if ( markerPack ) return markerPack->getGeometryListSize();
	else return 0;
}

GeometryInfo* AniCardLibCommonEditor::getGeometry(const unsigned int& id )
{
	if ( markerPack ) return markerPack->getGeometry( id );
	else return 0;
}



GeometryInfo* AniCardLibCommonEditor::getCardGeometry(const unsigned int& id )
{
	if ( markerPack ) return markerPack->getCardGeometry( id );
	else return 0;
}
TextureInfo* AniCardLibCommonEditor::getCardTexture(const unsigned int& id )
{
	if ( markerPack ) return markerPack->getCardTexture( id );
	else return 0;
}

TextureInfo* AniCardLibCommonEditor::getTexture( const unsigned int& id )
{
	if ( !markerPack ) return 0;
	return markerPack->getTexture( id );
}

unsigned int AniCardLibCommonEditor::getTextureListSize()
{
	if ( !markerPack ) return 0;
	return markerPack->getTextureListSize();
}

unsigned char* AniCardLibCommonEditor::getPicturePointer( const unsigned int& id )
{
	if ( !markerPack ) return 0;
	return markerPack->getPicturePointer( id );
}

bool AniCardLibCommonEditor::cardToTexture( TextureInfo* target , const unsigned int& id )
{
	if ( !target ) return false;
	if ( !markerPack ) return false;
	unsigned char* pointer = getPicturePointer( id );
	if ( !pointer ) return false;
	target->editTexture( (char*)pointer , getMarker( id )->width , getMarker( id )->height , 0 );
	return true;
}

std::string AniCardLibCommonEditor::getCardName( const unsigned int& index )
{
	if ( !names ) return std::string();
	return names->getCardName( index );
}
unsigned int AniCardLibCommonEditor::getNumCards()
{
	if ( !names ) return 0;
	return names->getNumCards();
}
std::string AniCardLibCommonEditor::getModelName( const unsigned int& index )
{
	if ( !names ) return std::string();
	return names->getModelName( index );
}
unsigned int AniCardLibCommonEditor::getNumModels()
{
	if ( !names ) return 0;
	return names->getNumModels();
}
std::string AniCardLibCommonEditor::getTextureName( const unsigned int& index )
{
	if ( !names ) return std::string();
	return names->getTextureName( index );
}
unsigned int AniCardLibCommonEditor::getNumTextures()
{
	if ( !names ) return 0;
	return names->getNumTextures();
}