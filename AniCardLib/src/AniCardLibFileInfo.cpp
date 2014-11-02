#include <AniCardLibFileInfo.h>
#include <fstream>
#include <Marker.h>
#include <SOIL.h>
AniCardLibFileInfo::AniCardLibFileInfo( const char* fileName )
{
	std::ifstream theFile( fileName , std::ios::ios_base::binary | std::ios::ios_base::in );
	theFile.read( (char*)this , sizeof( unsigned int ) * 4 );
	cardData = new Marker[numCards];

	theFile.read( (char*)cardData , sizeof( Marker ) * numCards );

	unsigned int* sizeOfModel = new unsigned int[numModels];
	theFile.read( ( char* ) sizeOfModel , sizeof( unsigned int ) * numModels );
	
	cardImageData = new char[sizeOfCardImageData];
	theFile.read( ( char* ) cardImageData , sizeOfCardImageData );

	void* modelData = new char[sizeOfModelData];
	theFile.read( ( char* ) modelData , sizeOfModelData );

	unsigned int imageOffset = 0;
	for ( unsigned int i = 0; i < numCards; +i )
	{
		Marker* markers = (Marker*)cardData;
		markers[i].imageOffset = (imageOffset);
		imageOffset += markers[i].width * markers[i].height;
	}

	bufferManager.initialize();
	geometryManager.initialize( numModels + 50 );

	unsigned int modelDataOffset = 0;
	for ( unsigned int i = 0; i < numModels; ++i )
	{
		geos.push_back( geometryManager.addRawGeometry( ( char* ) ( ( unsigned int ) modelData + modelDataOffset ) , bufferManager ));
		modelDataOffset += sizeOfModel[i];
	}
	delete[] sizeOfModel;
	delete[] modelData;
}

AniCardLibFileInfo::~AniCardLibFileInfo()
{
	delete[] cardData;
	delete[] cardImageData;
	geos.clear();
	geometryManager.destroy();
	bufferManager.destroy();
}

int AniCardLibFileInfo::addMarker( const char* fileName , const int& linkedModel = -1 )
{
	int channels = 0, width, height;
	unsigned char* bytes = SOIL_load_image( fileName , &width , &height , &channels , SOIL_LOAD_RGBA );

	void* lastCardImageData = cardImageData;
	cardImageData = new char[sizeOfCardImageData + ( width * height )];
	memcpy( cardImageData , lastCardImageData , sizeOfCardImageData );
	delete[] lastCardImageData;

	void* lastCardData = cardData;
	cardData = new Marker[numCards + 1];
	memcpy( cardData , lastCardData , sizeof( Marker ) * numCards );
	for ( int y = 0; y < height; ++y )
	{
		for ( int x = 0; x < width; ++x )
		{
			unsigned long iOffset = ( unsigned long ) ( ( y * 4 * width ) + ( x * 4 ) );
			float grayPixel = ( ( float ) bytes[iOffset] + ( float ) bytes[iOffset + 1] + ( float ) bytes[iOffset + 2] ) / 3.0f;
			unsigned char result = 0;
			if ( grayPixel > 128 ) result = 255;
			unsigned long i = ( unsigned long ) ( ( y * width ) + ( x ) ) + sizeOfCardImageData;
			unsigned char* pixelLocation = (unsigned char*)cardImageData;
			pixelLocation[i] = ( unsigned char ) result;
		}
	}
	Marker* markers = ( Marker* ) cardData;
	markers[numCards].width = width;
	markers[numCards].height = height;
	markers[numCards].linkedModel = linkedModel;
	markers[numCards].imageOffset = sizeOfCardImageData;
	++numCards;
	sizeOfCardImageData += width * height;
}
int addModel( const char* fileName , const int& cardToLink = -1 );
Marker* getMarker( const unsigned int& id );
unsigned int getMarkerListSize();
GeometryInfo* getGeometry( const unsigned int& id );
unsigned int getGeometryListSize();

bool save( const char* fileName );