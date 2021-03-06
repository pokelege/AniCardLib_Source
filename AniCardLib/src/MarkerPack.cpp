#include <gtc\matrix_transform.hpp>
#include <MarkerPack.h>
#include <SOIL.h>
#include <MathHelpers.h>

#include <iostream>
#include <future>
#include <Clock.h>
#include <AniCardLibFileInfo.h>
#include <Marker.h>
#include <thread>
#include <queue>
unsigned long MarkerPack::lowestDissimilarity = ULONG_MAX;
MarkerPack::MarkerPack() : debugPicture( 0 ) , cards( 0 )
{
	cards = new AniCardLibFileInfo;
	theAs[0] = 0;
	theAs[1] = 0;
	theAs[2] = 0;
	theAs[3] = 0;
}
MarkerPack::~MarkerPack()
{
	if ( cards ) delete cards;
}

bool MarkerPack::getPicture( unsigned char** bytes , long* width , long* height )
{
	if ( canGrab && bytes )
	{
		++numUsing;
		*bytes = debugPicture;
		if ( width ) *width = this->width;
		if ( height ) *height = this->height;
		return true;
	}
	return false;
}
bool MarkerPack::finishedUsing()
{
	if ( numUsing > 0 ) { --numUsing; return true; }
	else return false;
}

int MarkerPack::addMarker( const char* fileName , const int& linkedModel , const int& linkedTexture )
{
	return cards->addMarker( fileName , linkedModel , linkedTexture );
}
int MarkerPack::addModel( const char* fileName , const int& cardToLink)
{
	return cards->addModel( fileName , cardToLink );
}
int MarkerPack::addRawModel( const char* data , const int& cardToLink )
{
	return cards->addRawModel( data , cardToLink );
}
int MarkerPack::addTexture( const char* fileName , const int& cardToLink )
{
	return cards->addTexture( fileName , cardToLink );
}
int MarkerPack::addTexture( const char* data , const unsigned int& width , const unsigned int& height , const int& cardToLink)
{
	return cards->addTexture( data , width , height , cardToLink );
}

Marker* MarkerPack::getMarker( const unsigned int& id )
{
	return cards->getMarker( id );
}
unsigned int MarkerPack::getMarkerListSize()
{
	return cards->getMarkerListSize();
}
GeometryInfo* MarkerPack::getGeometry( const unsigned int& id )
{
	return cards->getGeometry( id );
}

GeometryInfo* MarkerPack::getCardGeometry( const unsigned int& id )
{
	return cards->getCardGeometry( id );
}

unsigned int MarkerPack::getGeometryListSize()
{
	return cards->getGeometryListSize();
}
TextureInfo* MarkerPack::getTexture( const unsigned int& id )
{
	return cards->getTexture( id );
}
TextureInfo* MarkerPack::getCardTexture( const unsigned int& id )
{
	return cards->getCardTexture( id );
}
unsigned int MarkerPack::getTextureListSize()
{
	return cards->getTextureListSize();
}

unsigned char* MarkerPack::getPicturePointer( const unsigned int& id )
{
	return cards->getPicturePointer( id );
}

bool MarkerPack::save( const char* fileName )
{
	return cards->save( fileName );
}
void MarkerPack::load( const char* fileName )
{
	if ( cards ) delete cards;
	cards = new AniCardLibFileInfo( fileName );
}

bool MarkerPack::matchMarker( Quad& quad , const unsigned char* picture , long pictureWidth , long pictureHeight )
{
	if ( !cards->getMarkerListSize() ) return false;

	bool toReturn = true;

	CompareWithMarkerInfo quadInfo;
	quadInfo.marker = 0;
	quadInfo.picture = picture;
	quadInfo.pictureHeight = pictureHeight;
	quadInfo.pictureWidth = pictureWidth;
	quadInfo.pos[0] = quad.pt[0];
	quadInfo.pos[1] = quad.pt[1];
	quadInfo.pos[2] = quad.pt[2];
	quadInfo.pos[3] = quad.pt[3];

	FoundMarkerInfo resultantMarker = getSmallestDissimilarity( quadInfo );
	//if ( resultantMarker.dissimilarity > lowestDissimilarity ) toReturn = false;

	quad.markerID = resultantMarker.markerID;
	quad.dissimilarity = resultantMarker.dissimilarity;

	for ( unsigned int i = 0; i < 4; ++i )
	{
		if ( !theAs[i] ) continue;
		delete[] theAs[i];
		theAs[i] = 0;
	}
	
	return toReturn;
}

MarkerPack::FoundMarkerInfo MarkerPack::getMarkerCornerDissimilarity( CompareWithMarkerInfo info )
{
	FoundMarkerInfo markerFoundInfo;
	
	float markerWidth = ( float ) cards->getMarker( info.marker )->width;
	float markerHeight = ( float ) cards->getMarker( info.marker )->height;
	markerFoundInfo.markerID = info.marker;
	markerFoundInfo.points[0] = info.pos[0];
	markerFoundInfo.points[1] = info.pos[1];
	markerFoundInfo.points[2] = info.pos[2];
	markerFoundInfo.points[3] = info.pos[3];
	markerFoundInfo.dissimilarity = ULONG_MAX;
	
	if ( theAs[info.AsID] != 0 )
	{
		unsigned long difference = 0;
		for ( unsigned long y = 0; y < markerHeight; ++y )
		{
			for ( unsigned long x = 0; x < markerWidth; ++x )
			{
				float Xi = ( theAs[info.AsID][0] * x + theAs[info.AsID][1] * y + theAs[info.AsID][2] ) /
					( theAs[info.AsID][6] * x + theAs[info.AsID][7] * y + 1 );
				float Yi = ( theAs[info.AsID][3] * x + theAs[info.AsID][4] * y + theAs[info.AsID][5] ) /
					( theAs[info.AsID][6] * x + theAs[info.AsID][7] * y + 1 );
				if ( Xi < 0 || Yi < 0 || Xi >= info.pictureWidth || Yi >= info.pictureHeight ) continue;
				int picturePixel = 0;

				if ( info.picture[( ( long ) Yi * info.pictureWidth ) + ( long ) ( Xi )] >= threshold ) picturePixel = 255;
				unsigned char* thePointer = cards->getPicturePointer( info.marker );
				

				long imageX = cards->getMarker( info.marker )->width - x;
				imageX = cards->getMarker( info.marker )->width + imageX;

				unsigned long iOffset = ( unsigned long ) ( ( y * 4 * cards->getMarker( info.marker )->width ) + ( (unsigned long)imageX * 4 ) );
				float grayPixel = ( ( float ) thePointer[iOffset] + ( float ) thePointer[iOffset + 1] + ( float ) thePointer[iOffset + 2] ) / 3.0f;
				int result = 0;
				if ( grayPixel >= 128 ) result = 255;

				int toAdd = ((int)picturePixel) - ((int)result);
				
				difference += (unsigned long)(toAdd * toAdd);
				if ( difference > lowestDissimilarity )
				{
					difference = ULONG_MAX;
					break;
				}
			}
			if ( difference > lowestDissimilarity )
			{
				difference = ULONG_MAX;
				break;
			}
		}
		
		markerFoundInfo.dissimilarity = difference;
		lowestDissimilarity = min( lowestDissimilarity , difference );		
	}

	//canGrab = false;
	////std::cin.get();
	//while ( numUsing ) std::cout << "using at markerpack" << std::endl;

	//if ( debugPicture && ( ( long ) getMarker( info.marker )->width != width || ( long ) getMarker( info.marker )->height != height ) )
	//{
	//	delete[] debugPicture;
	//	debugPicture = 0;
	//	width = 0;
	//	height = 0;
	//}
	//if ( !debugPicture )
	//{
	//	debugPicture = new unsigned char[( ( int ) getMarker( info.marker )->width * ( int ) getMarker( info.marker )->height ) * 4];
	//	width = ( long ) getMarker( info.marker )->width;
	//	height = ( long ) getMarker( info.marker )->height;
	//}

	//for ( long y = 0; y < height; ++y )
	//{
	//	for ( long x = 0; x < width; ++x )
	//	{
	//		float Xi = ( theAs[info.AsID][0] * x + theAs[info.AsID][1] * y + theAs[info.AsID][2] ) /
	//			( theAs[info.AsID][6] * x + theAs[info.AsID][7] * y + 1 );
	//		float Yi = ( theAs[info.AsID][3] * x + theAs[info.AsID][4] * y + theAs[info.AsID][5] ) /
	//			( theAs[info.AsID][6] * x + theAs[info.AsID][7] * y + 1 );
	//		//debugPicture[( y * 4 * width ) + ( x * 4 )] = markers[resultantMarker.markerID].bytes[(y *markers[resultantMarker.markerID].width) + x];
	//		//debugPicture[( y * 4 * width ) + ( x * 4 ) + 1] = markers[resultantMarker.markerID].bytes[(y *markers[resultantMarker.markerID].width) + x];
	//		//debugPicture[( y * 4 * width ) + ( x * 4 ) + 2] = markers[resultantMarker.markerID].bytes[(y *markers[resultantMarker.markerID].width) + x];
	//		unsigned char value = 0;
	//		if ( info.picture[( ( long ) Yi * info.pictureWidth ) + ( long ) ( Xi )] >= threshold ) value = 255;
	//		debugPicture[( y * 4 * width ) + ( x * 4 )] = value;
	//		debugPicture[( y * 4 * width ) + ( x * 4 ) + 1] = value;
	//		debugPicture[( y * 4 * width ) + ( x * 4 ) + 2] = value;
	//		//unsigned long toAdd = picture[( ( long ) Yi * pictureWidth ) + ( long ) ( Xi )] - markers[resultantMarker.markerID].bytes[( y * markers[resultantMarker.markerID].width ) + x];
	//		//std::cout << toAdd << std::endl;
	//	}
	//}
	//////std::cout << "dissimilarity " << resultantMarker.dissimilarity << std::endl;
	//canGrab = true;

	return markerFoundInfo;
}

MarkerPack::FoundMarkerInfo MarkerPack::getSmallestDissimilarity( CompareWithMarkerInfo info )
{
	float markerWidth = (float)cards->getMarker( 0 )->width;
	float markerHeight = (float)cards->getMarker( 0 )->height;
	for ( unsigned int i = 0; i < 4; ++i )
	{
		glm::vec2 pos[4];

		pos[0] = info.pos[i % 4];
		pos[1] = info.pos[( i + 1 ) % 4];
		pos[2] = info.pos[( i + 2 ) % 4];
		pos[3] = info.pos[( i + 3 ) % 4];

		float* vectorMult = new float[8];
		vectorMult[0] = ( float ) ( pos[0].x );
		vectorMult[1] = ( float ) ( pos[1].x );
		vectorMult[2] = ( float ) ( pos[2].x );
		vectorMult[3] = ( float ) ( pos[3].x );
		vectorMult[4] = ( float ) ( pos[0].y );
		vectorMult[5] = ( float ) ( pos[1].y );
		vectorMult[6] = ( float ) ( pos[2].y );
		vectorMult[7] = ( float ) ( pos[3].y );
		float* matrix = new float[8 * 8];
		{
			float prematrix[8][8] =
			{
				{ 0 , 0 , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[0] ) * 0 , -( float ) ( vectorMult[0] ) * 0 } ,
				{ 0 , markerHeight , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[1] ) * 0 , -( float ) ( vectorMult[1] ) * markerHeight } ,
				{ markerWidth , markerHeight , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[2] ) * markerWidth , -( float ) ( vectorMult[2] ) * markerHeight } ,
				{ markerWidth , 0 , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[3] ) *  markerWidth , -( float ) ( vectorMult[3] ) * 0 } ,

				{ 0 , 0 , 0 , 0 , 0 , 1 , -( float ) ( vectorMult[4] ) * 0 , -( float ) ( vectorMult[4] ) * 0 } ,
				{ 0 , 0 , 0 , 0 , markerHeight , 1 , -( float ) ( vectorMult[5] ) * 0 , -( float ) ( vectorMult[5] ) * markerHeight } ,
				{ 0 , 0 , 0 , markerWidth , markerHeight , 1 , -( float ) ( vectorMult[6] ) * markerWidth , -( float ) ( vectorMult[6] ) * markerHeight } ,
				{ 0 , 0 , 0 , markerWidth , 0 , 1 , -( float ) ( vectorMult[7] ) *  markerWidth , -( float ) ( vectorMult[7] ) * 0 } ,
			};
			for ( unsigned int y = 0; y < 8; ++y )
			{
				for ( unsigned int x = 0; x < 8; ++x )
				{
					matrix[( y * 8 ) + x] = prematrix[y][x];
				}
			}
		}
		float determinant = MathHelpers::Determinant( matrix , 8 );
		if ( determinant != 0 )
		{
			MathHelpers::preAdj( matrix , 8 );

			MathHelpers::Transpose( matrix , 8 );
			MathHelpers::Multiply( matrix , 1.0f / determinant , 8 );

			MathHelpers::MultiplyVector( matrix , vectorMult , 8 );
			theAs[i] = new float[8];
			memcpy( theAs[i] , vectorMult , sizeof( float ) * 8 );
		}
		delete[] matrix;
		delete[] vectorMult;
	}
	
	unsigned char minimum = UCHAR_MAX , maximum = 0;
	for ( unsigned long y = 0; y < markerHeight; ++y )
	{
		for ( unsigned long x = 0; x < markerWidth; ++x )
		{
			float Xi = ( theAs[0][0] * x + theAs[0][1] * y + theAs[0][2] ) /
				( theAs[0][6] * x + theAs[0][7] * y + 1 );
			float Yi = ( theAs[0][3] * x + theAs[0][4] * y + theAs[0][5] ) /
				( theAs[0][6] * x + theAs[0][7] * y + 1 );
			if ( Xi < 0 || Yi < 0 || Xi >= info.pictureWidth || Yi >= info.pictureHeight ) continue;
			unsigned char sample = info.picture[( ( long ) Yi * info.pictureWidth ) + ( long ) ( Xi )];
			minimum = min( sample , minimum );
			maximum = max( sample , maximum );
		}
	}
	if ( minimum > maximum ) minimum = maximum;
	threshold = (( maximum - minimum ) / 2) + minimum;
	lowestDissimilarity = ULONG_MAX;

	std::queue<std::future<FoundMarkerInfo>> closestMarkerCorners;
	unsigned int numThreads = std::thread::hardware_concurrency();
	unsigned int i = 0, j = 0;
	unsigned long smallestDissimilarity = ULONG_MAX;
	FoundMarkerInfo toReturn;
	while ( i < cards->getMarkerListSize() )
	{
		while ( j < 4 && closestMarkerCorners.size() < numThreads )
		{
			CompareWithMarkerInfo compareOrientations;
			compareOrientations.marker = i;
			compareOrientations.picture = info.picture;
			compareOrientations.pictureHeight = info.pictureHeight;
			compareOrientations.pictureWidth = info.pictureWidth;
			compareOrientations.pos[0] = info.pos[j% 4];
			compareOrientations.pos[1] = info.pos[( j + 1 ) % 4];
			compareOrientations.pos[2] = info.pos[( j + 2 ) % 4];
			compareOrientations.pos[3] = info.pos[( j + 3 ) % 4];
			compareOrientations.AsID = j;
			closestMarkerCorners.push( std::async( std::launch::async , &MarkerPack::getMarkerCornerDissimilarity , this , compareOrientations ) );
			++j;
		}
		if ( j >= 4 )
		{
			j = 0;
			++i;
		}
		if ( closestMarkerCorners.size() >= numThreads )
		{
			closestMarkerCorners.front().wait();
			FoundMarkerInfo theResult = closestMarkerCorners.front().get();
			closestMarkerCorners.pop();
			smallestDissimilarity = min( theResult.dissimilarity , smallestDissimilarity );
			if ( smallestDissimilarity == theResult.dissimilarity ) toReturn = theResult;
		}
	}

	while ( closestMarkerCorners.size() )
	{
		closestMarkerCorners.front().wait();
		FoundMarkerInfo theResult = closestMarkerCorners.front().get();
		closestMarkerCorners.pop();
		smallestDissimilarity = min( theResult.dissimilarity , smallestDissimilarity );
		if ( smallestDissimilarity == theResult.dissimilarity ) toReturn = theResult;
	}

	return toReturn;
}