#include <AniCardLibCommonEditor.h>
#include <MarkerPack.h>
#include <Graphics\TextureInfo.h>
#include <Marker.h>
AniCardLibCommonEditor::AniCardLibCommonEditor() :markerPack( 0 )
{
	markerPack = new MarkerPack;
}

AniCardLibCommonEditor::~AniCardLibCommonEditor()
{
	if ( markerPack ) delete markerPack;
}

void AniCardLibCommonEditor::load( const char* filename )
{
	if ( markerPack ) delete markerPack;
	markerPack = new MarkerPack();
	markerPack->load( filename );
}

bool AniCardLibCommonEditor::save( const char* filename )
{
	if ( !markerPack ) return false;
	return markerPack->save( filename );
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