#include <AniCardLibFileNamesInfo.h>
#include <fstream>
AniCardLibFileNamesInfo::AniCardLibFileNamesInfo() : numCards(0) , numModels(0) , numTextures(0), sizeOfTextInfo(0) , cardNameIndices(0) , modelNameIndices(0) , textureNameIndices(0) , textInfo(0)
{
	
}
AniCardLibFileNamesInfo::~AniCardLibFileNamesInfo()
{
	if ( cardNameIndices ) delete[] cardNameIndices;
	if ( modelNameIndices ) delete[] modelNameIndices;
	if ( textureNameIndices ) delete[] textureNameIndices;
	if ( textInfo ) delete[] textInfo;
}
std::string AniCardLibFileNamesInfo::getCardName( const unsigned int& index )
{
	if ( !cardNameIndices || index >= numCards ) return "";
	else
	{
		RangeInfo* theRange = &cardNameIndices[index];
		return std::string( &textInfo[theRange->start] , theRange->end + 1 - theRange->start );
	}
}
unsigned int AniCardLibFileNamesInfo::getNumCards()
{
	return numCards;
}
std::string AniCardLibFileNamesInfo::getModelName( const unsigned int& index )
{
	if ( !modelNameIndices || index >= numModels ) return "";
	else
	{
		RangeInfo* theRange = &modelNameIndices[index];
		return std::string( &textInfo[theRange->start] , theRange->end + 1 - theRange->start );
	}
}
unsigned int AniCardLibFileNamesInfo::getNumModels()
{
	return numModels;
}
std::string AniCardLibFileNamesInfo::getTextureName( const unsigned int& index )
{
	if ( !textureNameIndices || index >= numTextures ) return "";
	else
	{
		RangeInfo* theRange = &textureNameIndices[index];
		return std::string( &textInfo[theRange->start] , theRange->end + 1 - theRange->start );
	}
}
unsigned int AniCardLibFileNamesInfo::getNumTextures()
{
	return numTextures;
}
void AniCardLibFileNamesInfo::load( const char* filename )
{
	if ( cardNameIndices ) delete[] cardNameIndices;
	if ( modelNameIndices ) delete[] modelNameIndices;
	if ( textureNameIndices ) delete[] textureNameIndices;
	if ( textInfo ) delete[] textInfo;

	std::ifstream theFile( filename , std::ios::ios_base::binary | std::ios::ios_base::in );
	theFile.read( (char*)this , sizeof( unsigned int ) * 4 );
	if ( numCards)
	{
		cardNameIndices = new RangeInfo[numCards];
		theFile.read( ( char* ) cardNameIndices , sizeof( RangeInfo ) * numCards );
	}
	else cardNameIndices = 0;
	if ( numModels )
	{
		modelNameIndices = new RangeInfo[numModels];
		theFile.read( ( char* ) modelNameIndices , sizeof( RangeInfo ) * numModels );
	}
	else modelNameIndices = 0;
	if ( numTextures )
	{
		textureNameIndices = new RangeInfo[numTextures];
		theFile.read( ( char* ) textureNameIndices , sizeof( RangeInfo ) * numTextures );
	}
	else textureNameIndices = 0;
	if ( sizeOfTextInfo )
	{
		textInfo = new char[sizeOfTextInfo];
		theFile.read( textInfo , sizeOfTextInfo );
	}
	else textInfo = 0;
	theFile.close();
}
void AniCardLibFileNamesInfo::save( const char* filename , const std::vector<std::string>& cards , const std::vector<std::string>& models , const std::vector<std::string>& textures )
{
	std::fstream names( filename , std::ios_base::binary | std::ios_base::out | std::ios_base::trunc );

	RangeInfo* theCards = new RangeInfo[cards.size()];
	RangeInfo* theModels = new RangeInfo[models.size()];
	RangeInfo* theTextures = new RangeInfo[textures.size()];
	unsigned int sizeOfText = 0;

	for ( unsigned int i = 0; i < cards.size(); ++i )
	{
		theCards[i].start = sizeOfText;
		sizeOfText += cards[i].size();
		theCards[i].end = sizeOfText - 1;
	}

	for ( unsigned int i = 0; i < models.size(); ++i )
	{
		theModels[i].start = sizeOfText;
		sizeOfText += models[i].size();
		theModels[i].end = sizeOfText - 1;
	}

	for ( unsigned int i = 0; i < textures.size(); ++i )
	{
		theTextures[i].start = sizeOfText;
		sizeOfText += textures[i].size();
		theTextures[i].end = sizeOfText - 1;
	}

	unsigned int sizeofCards = cards.size();
	unsigned int sizeofModels = models.size();
	unsigned int sizeofTextures = textures.size();

	names.write( ( const char* ) &sizeofCards , sizeof( sizeofCards ) );
	names.write( ( const char* ) &sizeofModels , sizeof( sizeofModels ) );
	names.write( ( const char* ) &sizeofTextures , sizeof( sizeofTextures ) );
	names.write( ( const char* ) &sizeOfText , sizeof( sizeOfText ) );
	names.write( ( const char* ) theCards , sizeof( RangeInfo ) * sizeofCards );
	names.write( ( const char* ) theModels , sizeof( RangeInfo ) * sizeofModels );
	names.write( ( const char* ) theTextures , sizeof( RangeInfo ) * sizeofTextures );

	for ( unsigned int i = 0; i < cards.size(); ++i )
	{
		names.write( cards[i].c_str() , cards[i].size() );
	}

	for ( unsigned int i = 0; i < models.size(); ++i )
	{
		names.write( models[i].c_str() , models[i].size() );
	}

	for ( unsigned int i = 0; i < textures.size(); ++i )
	{
		names.write( textures[i].c_str() , textures[i].size() );
	}
	delete[] theCards;
	delete[] theModels;
	delete[] theTextures;
	names.close();
}