#pragma once
#include <AniCardLibExportHeader.h>
#include <vector>
#include <string>
class ANICARDLIB_SHARED AniCardLibFileNamesInfo
{
	struct RangeInfo
	{
		unsigned int start;
		unsigned int end;
	};

	unsigned int numCards;
	unsigned int numModels;
	unsigned int numTextures;
	unsigned int sizeOfTextInfo;
	RangeInfo* cardNameIndices;
	RangeInfo* modelNameIndices;
	RangeInfo* textureNameIndices;

	char* textInfo;
public:
	AniCardLibFileNamesInfo();
	~AniCardLibFileNamesInfo();
	std::string getCardName( const unsigned int& index );
	unsigned int getNumCards();
	std::string getModelName( const unsigned int& index );
	unsigned int getNumModels();
	std::string getTextureName( const unsigned int& index );
	unsigned int getNumTextures();
	void load( const char* filename );
	void save( const char* filename , const std::vector<std::string>& cards , const std::vector<std::string>& models , const std::vector<std::string>& textures );
};