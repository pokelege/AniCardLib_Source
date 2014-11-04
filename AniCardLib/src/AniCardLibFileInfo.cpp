#include <AniCardLibFileInfo.h>
#include <fstream>
#include <Marker.h>
#include <SOIL.h>
#include <TextureDimensions.h>
AniCardLibFileInfo::AniCardLibFileInfo() : cardData(0) , cardImageData(0) , numCards(0) , numModels(0) , numTextures(0) , sizeOfCardImageData(0) , sizeOfModelData(0) , sizeOfTextureData(0)
{
	bufferManager.initialize();
	geometryManager.initialize();
	textureManager.initialize();
}
AniCardLibFileInfo::AniCardLibFileInfo( const char* fileName )
{
	std::ifstream theFile( fileName , std::ios::ios_base::binary | std::ios::ios_base::in );
	theFile.read( (char*)this , sizeof( unsigned int ) * 6 );
	cardData = new Marker[numCards];

	theFile.read( (char*)cardData , sizeof( Marker ) * numCards );

	cardImageData = new char[sizeOfCardImageData];
	theFile.read( ( char* ) cardImageData , sizeOfCardImageData );

	unsigned int* sizeOfModel = new unsigned int[numModels];
	theFile.read( ( char* ) sizeOfModel , sizeof( unsigned int ) * numModels );

	void* modelData = new char[sizeOfModelData];
	theFile.read( ( char* ) modelData , sizeOfModelData );

	TextureDimensions* textureDimension = new TextureDimensions[numTextures];
	theFile.read( ( char* ) textureDimension , sizeof( TextureDimensions ) * numTextures );

	void* textureData = new char[sizeOfTextureData];
	theFile.read( ( char* ) textureData , sizeOfTextureData );

	unsigned int imageOffset = 0;
	for ( unsigned int i = 0; i < numCards; ++i )
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

	textureManager.initialize( numTextures + 10 );
	unsigned int textureDataOffset = 0;
	for ( unsigned int i = 0; i < numTextures; ++i )
	{
		char* textureMap = (char*)( ( unsigned int ) textureData + textureDataOffset );
		textures.push_back(textureManager.addTexture( textureMap , textureDimension[i].width , textureDimension[i].height, 0 ));
		textureDataOffset += textureDimension[i].width * textureDimension[i].height * 4;
	}
	delete[] sizeOfModel;
	delete[] modelData;
	delete[] textureData;
	delete[] textureDimension;
}

AniCardLibFileInfo::~AniCardLibFileInfo()
{
	if(cardData) delete[] cardData;
	if(cardImageData) delete[] cardImageData;
	geos.clear();
	geometryManager.destroy();
	bufferManager.destroy();
}

int AniCardLibFileInfo::addMarker( const char* fileName , const int& linkedModel , const int& linkedTexture)
{
	int channels = 0, width, height;
	unsigned char* bytes = SOIL_load_image( fileName , &width , &height , &channels , SOIL_LOAD_RGBA);

	if ( numCards > 0 )
	{
		void* lastCardImageData = cardImageData;
		cardImageData = new char[sizeOfCardImageData + ( width * height * 4 )];
		memcpy( cardImageData , lastCardImageData , sizeOfCardImageData );
		delete[] lastCardImageData;

		void* lastCardData = cardData;

		cardData = new Marker[numCards + 1];
		memcpy( cardData , lastCardData , sizeof( Marker ) * numCards );
	}
	else
	{
		cardImageData = new char[( width * height * 4 )];
		cardData = new Marker[1];
	}

	unsigned char* pixelLocation = ( unsigned char* ) cardImageData;
	memcpy( &pixelLocation[sizeOfCardImageData] , bytes , width * height * 4 );

	Marker* markers = ( Marker* ) cardData;
	markers[numCards].width = width;
	markers[numCards].height = height;
	markers[numCards].linkedModel = linkedModel;
	markers[numCards].linkedTexture = linkedTexture;
	markers[numCards].imageOffset = sizeOfCardImageData;
	sizeOfCardImageData += width * height * 4;
	SOIL_free_image_data( bytes );
	return numCards++;
}
int AniCardLibFileInfo::addModel( const char* fileName , const int& cardToLink  )
{
	GeometryInfo* theGeo = geometryManager.addPMDGeometry( fileName , bufferManager );
	if ( !theGeo )
	{
		return -1;
	}
	geos.push_back( theGeo );
	if ( cardToLink >= 0 )
	{
		Marker* markers = ( Marker* ) cardData;
		markers[cardToLink].linkedModel = numModels;
	}
	return numModels++;
}

int AniCardLibFileInfo::addTexture( const char* fileName , const int& cardToLink)
{
	TextureInfo* theTexture = textureManager.addTexture( fileName , 0 );
	if ( !theTexture ) return -1;
	textures.push_back( theTexture );
	if ( cardToLink >= 0 )
	{
		Marker* markers = ( Marker* ) cardData;
		markers[cardToLink].linkedTexture = numTextures;
	}
	return numTextures++;
}

Marker* AniCardLibFileInfo::getMarker( const unsigned int& id )
{
	if ( id >= numCards ) return 0;
	Marker* markers = ( Marker* ) cardData;
	return &markers[id];
}
unsigned int AniCardLibFileInfo::getMarkerListSize()
{
	return numCards;
}
GeometryInfo* AniCardLibFileInfo::getGeometry( const unsigned int& id )
{
	if ( id >= geos.size() ) return 0;
	return geos.at( id );
}
unsigned int AniCardLibFileInfo::getGeometryListSize()
{
	return geos.size();
}

TextureInfo* AniCardLibFileInfo::getTexture( const unsigned int& id )
{
	if ( id >= textures.size() ) return 0;
	return textures.at( id );
}
unsigned int AniCardLibFileInfo::getTextureListSize()
{
	return textures.size();
}

unsigned char* AniCardLibFileInfo::getPicturePointer( const unsigned int& id )
{
	if ( id >= numCards ) return 0;
	Marker* markers = ( Marker* ) cardData;
	return ( unsigned char * ) (markers[id].imageOffset + ( unsigned int ) cardImageData);
}

bool AniCardLibFileInfo::save( const char* fileName )
{
	std::fstream stream( fileName , std::ios_base::binary | std::ios_base::out | std::ios_base::trunc );

	numModels = geos.size();
	numTextures = textures.size();

	std::string modelData;
	std::vector<unsigned int> modelDataSize;
	for ( unsigned int i = 0; i < geos.size(); ++i )
	{
		std::string toCopy = geometryManager.saveGeometry( geos[i] );
		modelDataSize.push_back( toCopy.size() );
		modelData += toCopy;
	}
	sizeOfModelData = modelData.size();

	std::string textureData;
	std::vector<TextureDimensions> textureDimensions;
	for ( unsigned int i = 0; i < textures.size(); ++i )
	{
		TextureDimensions texDim;
		int width , height;
		textureData += textureManager.saveRawTexture( textures[i] ,&width , &height );
		texDim.width = width;
		texDim.height = height;
		textureDimensions.push_back( texDim );
	}
	sizeOfTextureData = textureData.size();

	stream.write( (char*)this , sizeof( unsigned int ) * 6 );
	stream.write( ( char* ) cardData , sizeof( Marker ) * numCards );
	stream.write( ( char* ) cardImageData , sizeOfCardImageData );
	for ( unsigned int i = 0; i < modelDataSize.size(); ++i )
	{
		stream.write( ( char* ) &modelDataSize[i] , sizeof( unsigned int ) );
	}
	modelDataSize.clear();

	stream.write( modelData.c_str() , modelData.size() );

	for ( unsigned int i = 0; i < textureDimensions.size(); ++i )
	{
		stream.write( ( char* ) &textureDimensions[i] , sizeof( TextureDimensions ) );
	}
	textureDimensions.clear();

	stream.write( textureData.c_str() , textureData.size() );
	textureData.clear();
	stream.close();
	return true;
}