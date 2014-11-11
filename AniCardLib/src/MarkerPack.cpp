#include <gtc\matrix_transform.hpp>
#include <MarkerPack.h>
#include <SOIL.h>
#include <MathHelpers.h>

#include <iostream>
#include <future>
#include <Clock.h>
#include <AniCardLibFileInfo.h>
#include <Marker.h>
MarkerPack MarkerPack::global;
unsigned long MarkerPack::highestDissimilarity = ULONG_MAX;
MarkerPack::MarkerPack() : debugPicture( 0 ) , cards( 0 )
{
	cards = new AniCardLibFileInfo;
}

bool MarkerPack::getPicture( unsigned char** bytes , long* width , long* height )
{
	if ( canGrab && bytes )
	{
		++numUsing;
		//std::cout << numUsing << std::endl;
		*bytes = debugPicture;
		if ( width ) *width = this->width;
		if ( height ) *height = this->height;
		return true;
	}
	return false;
}
bool MarkerPack::finishedUsing()
{
	//std::cout << numUsing << std::endl;
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
int MarkerPack::addTexture( const char* fileName , const int& cardToLink )
{
	return cards->addTexture( fileName , cardToLink );
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
	if ( resultantMarker.dissimilarity > highestDissimilarity ) return false;
	float markerWidth = ( ( float ) cards->getMarker(resultantMarker.markerID )->width) / pictureWidth;
	float markerHeight = ( ( float ) cards->getMarker( resultantMarker.markerID )->height ) / pictureHeight;
	float* vectorMult = new float[8];
	vectorMult[0] = ( float ) ( resultantMarker.points[0].x - ( pictureWidth / 2 ) ) / 2000.0f;
	vectorMult[1] = ( float ) ( resultantMarker.points[1].x - ( pictureWidth / 2 ) ) / 2000.0f;
	vectorMult[2] = ( float ) ( resultantMarker.points[2].x - ( pictureWidth / 2 ) ) / 2000.0f;
	vectorMult[3] = ( float ) ( resultantMarker.points[3].x - ( pictureWidth / 2 ) ) / 2000.0f;
	vectorMult[4] = ( float ) ( resultantMarker.points[0].y - ( pictureHeight / 2 ) ) / 2000.0f;
	vectorMult[5] = ( float ) ( resultantMarker.points[1].y - ( pictureHeight / 2 ) ) / 2000.0f;
	vectorMult[6] = ( float ) ( resultantMarker.points[2].y - ( pictureHeight / 2 ) ) / 2000.0f;
	vectorMult[7] = ( float ) ( resultantMarker.points[3].y - ( pictureHeight / 2 ) ) / 2000.0f;
	//std::cout << resultantMarker.points[0].x << " - " << pictureWidth / 2 << " = " << ( resultantMarker.points[0].x - ( pictureWidth / 2 ) ) << std::endl;
	float* matrix = new float[8 * 8];
	//std::cout << ( resultantMarker.points[0].x - ( pictureWidth / 2 ) ) << " / " << ( resultantMarker.points[0].x - ( pictureWidth / 2 ) ) / 1000.0f << std::endl;
	{

		float prematrix[8][8] =
		{
			{ 0 , 0 , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[0] ) * 0 , -( float ) ( vectorMult[0] ) * 0 } ,
			{ 0 , markerHeight , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[1] ) * 0 , -( float ) ( vectorMult[1] ) * markerHeight } ,
			{ markerWidth , markerHeight , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[2] ) *markerWidth , -( float ) ( vectorMult[2] ) * markerHeight } ,
			{ markerWidth , 0 , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[3] ) * markerWidth , -( float ) ( vectorMult[3] ) * 0 } ,

			{ 0 , 0 , 0 , 0 , 0 , 1 , -( float ) ( vectorMult[4] ) * 0 , -( float ) ( vectorMult[4] ) * 0 } ,
			{ 0 , 0 , 0 , 0 , markerHeight , 1 , -( float ) ( vectorMult[5] ) * 0 , -( float ) ( vectorMult[5] ) * markerHeight } ,
			{ 0 , 0 , 0 , markerWidth , markerHeight , 1 , -( float ) ( vectorMult[6] ) * markerWidth , -( float ) ( vectorMult[6] ) * markerHeight } ,
			{ 0 , 0 , 0 , markerWidth , 0 , 1 , -( float ) ( vectorMult[7] ) * markerWidth , -( float ) ( vectorMult[7] ) * 0 } ,
		};
		for ( unsigned int j = 0; j < 8; ++j )
		{
			for ( unsigned int i = 0; i < 8; ++i )
			{
				matrix[( j * 8 ) + i] = prematrix[j][i];
			}
		}
	}

	float determinant = MathHelpers::Determinant( matrix , 8 );
	//std::cout << determinant << std::endl;
	glm::mat4 transform;
	if ( determinant != 0 )
	{
		MathHelpers::preAdj( matrix , 8 );
		MathHelpers::Transpose( matrix , 8 );
		MathHelpers::Multiply( matrix , 1.0f / determinant , 8 );
		MathHelpers::MultiplyVector( matrix , vectorMult , 8 );

		float a = ( vectorMult[2] * vectorMult[5] ) - ( vectorMult[4] * vectorMult[3] );
		float b = ( vectorMult[0] * vectorMult[5] ) - ( vectorMult[1] * vectorMult[4] );
		float c = ( vectorMult[0] * vectorMult[3] ) - ( vectorMult[2] * vectorMult[1] );
		//std::cout << "{" << a << "," << b << "," << c << "}" << std::endl;
		//std::cout << ( ( a * a ) + ( b* b ) + ( c* c ) ) << std::endl;
		//std::cout << 1.0f / ( ( a * a ) + ( b* b ) + ( c* c ) ) << std::endl;
		//std::cout << -powf( 1.0f / ( ( a * a ) + ( b* b ) + ( c* c ) ) , 0.25f ) << std::endl;
		glm::vec3 translateVector;
		translateVector.z = -powf( 1.0f / ( ( a * a ) + ( b* b ) + ( c* c ) ) , 0.25f );
		//std::cout << translateVector.z << std::endl;
		//std::cout << vectorMult[6] << std::endl;
		translateVector.x = -vectorMult[6] * translateVector.z;
		translateVector.y = -vectorMult[7] * translateVector.z;
		glm::vec4 row0( -vectorMult[0] * translateVector.z , -vectorMult[2] * translateVector.z , vectorMult[4] * translateVector.z , 0 );
		glm::vec4 row1( -vectorMult[1] * translateVector.z , -vectorMult[3] * translateVector.z , vectorMult[5] * translateVector.z , 0 );
		glm::vec4 row2 = glm::vec4( glm::cross( glm::vec3( row1 ) , glm::vec3( row0 ) ) , 0 );
		glm::mat4 theResultingMatrix( row0 , row1 , row2 , glm::vec4( 0 , 0 , 0 , 1 ) );
		transform = glm::translate( glm::mat4() , translateVector ) * glm::transpose( theResultingMatrix );
		quad.transform = transform;
		//translated /= translated.w;
		//std::cout << "{" << translateVector.x << "," << translateVector.y << "," << translateVector.z << "}" << std::endl;
	}
	//compareOrientations[1].marker = 0;
	//compareOrientations[1].picture = picture;
	//compareOrientations[1].pictureHeight = pictureHeight;
	//compareOrientations[1].pictureWidth = pictureWidth;
	//compareOrientations[1].pos[3] = quad.pt[3];
	//compareOrientations[1].pos[0] = quad.pt[0];
	//compareOrientations[1].pos[1] = quad.pt[1];
	//compareOrientations[1].pos[2] = quad.pt[2];

	//compareOrientations[2].marker = 0;
	//compareOrientations[2].picture = picture;
	//compareOrientations[2].pictureHeight = pictureHeight;
	//compareOrientations[2].pictureWidth = pictureWidth;
	//compareOrientations[2].pos[2] = quad.pt[2];
	//compareOrientations[2].pos[3] = quad.pt[3];
	//compareOrientations[2].pos[0] = quad.pt[0];
	//compareOrientations[2].pos[1] = quad.pt[1];

	//compareOrientations[3].marker = 0;
	//compareOrientations[3].picture = picture;
	//compareOrientations[3].pictureHeight = pictureHeight;
	//compareOrientations[3].pictureWidth = pictureWidth;
	//compareOrientations[3].pos[1] = quad.pt[1];
	//compareOrientations[3].pos[2] = quad.pt[2];
	//compareOrientations[3].pos[3] = quad.pt[3];
	//compareOrientations[3].pos[0] = quad.pt[0];

	////AniCardLib::Clock c;
	////c.Start();
	//std::vector<std::future<FoundMarkerInfo>> orientationsToTest;
	//orientationsToTest.push_back( std::async( std::launch::async , &MarkerPack::getMarkerCornerDissimilarity , this , compareOrientations[0] ));
	//orientationsToTest.push_back( std::async( std::launch::async , &MarkerPack::getMarkerCornerDissimilarity , this , compareOrientations[1] ) );
	//orientationsToTest.push_back( std::async( std::launch::async , &MarkerPack::getMarkerCornerDissimilarity , this , compareOrientations[2] ) );
	//orientationsToTest.push_back( std::async( std::launch::async , &MarkerPack::getMarkerCornerDissimilarity , this , compareOrientations[3] ) );

	//unsigned int closest = 0;
	//unsigned long smallestDissimilarity = ULONG_MAX;


	//for ( unsigned int i = 0; i < 4; ++i )
	//{
	//	orientationsToTest[i].wait();
	//	FoundMarkerInfo theResult = orientationsToTest[i].get();
	//	smallestDissimilarity = min( theResult.dissimilarity , smallestDissimilarity );
	//	if ( smallestDissimilarity == theResult.dissimilarity ) closest = i;
	//}



	//std::cout << "calctime " << c.Stop() << std::endl;
	//float markerWidth = ( float ) markers[0].width;
	//float markerHeight = ( float ) markers[0].height;
	//float* vectorMult = new float[8];
	//vectorMult[0] = ( float ) ( quad.pt[0].x );
	//vectorMult[1] = ( float ) ( quad.pt[1].x );
	//vectorMult[2] = ( float ) ( quad.pt[2].x );
	//vectorMult[3] = ( float ) ( quad.pt[3].x );
	//vectorMult[4] = ( float ) ( quad.pt[0].y );
	//vectorMult[5] = ( float ) ( quad.pt[1].y );
	//vectorMult[6] = ( float ) ( quad.pt[2].y );
	//vectorMult[7] = ( float ) ( quad.pt[3].y );

	//float* matrix = new float[8 * 8];
	//{

	//	float prematrix[8][8] =
	//	{
	//		{ 0 , 0 , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[0] ) * 0 , -( float ) ( vectorMult[0] ) * 0 } ,
	//		{ 0 , markerHeight , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[1] ) * 0 , -( float ) ( vectorMult[1] ) * markerHeight } ,
	//		{ markerWidth , markerHeight , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[2] ) *markerWidth , -( float ) ( vectorMult[2] ) * markerHeight } ,
	//		{ markerWidth , 0 , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[3] ) * markerWidth , -( float ) ( vectorMult[3] ) * 0 } ,

	//		{ 0 , 0 , 0 , 0 , 0 , 1 , -( float ) ( vectorMult[4] ) * 0 , -( float ) ( vectorMult[4] ) * 0 } ,
	//		{ 0 , 0 , 0 , 0 , markerHeight , 1 , -( float ) ( vectorMult[5] ) * 0 , -( float ) ( vectorMult[5] ) * markerHeight } ,
	//		{ 0 , 0 , 0 , markerWidth , markerHeight , 1 , -( float ) ( vectorMult[6] ) * markerWidth , -( float ) ( vectorMult[6] ) * markerHeight } ,
	//		{ 0 , 0 , 0 , markerWidth , 0 , 1 , -( float ) ( vectorMult[7] ) * markerWidth , -( float ) ( vectorMult[7] ) * 0 } ,
	//	};
	//	for ( unsigned int j = 0; j < 8; ++j )
	//	{
	//		for ( unsigned int i = 0; i < 8; ++i )
	//		{
	//			matrix[( j * 8 ) + i] = prematrix[j][i];
	//		}
	//	}
	//}

	//float determinant = MathHelpers::Determinant( matrix , 8 );
	////std::cout << determinant << std::endl;
	//glm::mat4 transform;
	//if ( determinant != 0 )
	//{
	//	MathHelpers::preAdj( matrix , 8 );
	//	MathHelpers::Transpose( matrix , 8 );
	//	MathHelpers::Multiply( matrix , 1.0f / determinant , 8 );
	//	MathHelpers::MultiplyVector( matrix , vectorMult , 8 );

	//	float a = ( vectorMult[2] * vectorMult[5] ) - ( vectorMult[4] * vectorMult[3] );
	//	float b = ( vectorMult[0] * vectorMult[5] ) - ( vectorMult[1] * vectorMult[4] );
	//	float c = ( vectorMult[0] * vectorMult[3] ) - ( vectorMult[2] * vectorMult[1] );
	//	//std::cout << "{" << a << "," << b << "," << c << "}" << std::endl;
	//	glm::vec4 translateVector;
	//	translateVector.z = -powf( 1.0f / ( ( a * a ) + ( b* b ) + ( c* c ) ) , 0.25f );
	//	std::cout << translateVector.z << std::endl;
	//	translateVector.x = -vectorMult[6] * translateVector.z;
	//	translateVector.y = -vectorMult[7] * translateVector.z;
	//	translateVector.w = 1;
	//	glm::vec4 row0( -vectorMult[0] * translateVector.z , -vectorMult[2] * translateVector.z , vectorMult[4] * translateVector.z , translateVector.x );
	//	glm::vec4 row1( -vectorMult[1] * translateVector.z , -vectorMult[3] * translateVector.z , vectorMult[5] * translateVector.z , translateVector.y );
	//	glm::vec4 row2 = glm::vec4( glm::cross( glm::vec3( row0 ) , glm::vec3( row1 ) ) , translateVector.z );
	//	glm::mat4 theResultingMatrix( row0 , row1 , row2 , glm::vec4( 0 , 0 , 0 , 1 ) );
	//	transform = glm::transpose( theResultingMatrix );
	//	//translated /= translated.w;
	//	//std::cout << "{" << translateVector.x << "," << translateVector.y << "," << translateVector.z << "}" << std::endl;
	//}

	//while ( numUsing ) std::cout << "using at markerpack" << std::endl;
	//canGrab = false;
	//if ( debugPicture && ( markers[resultantMarker.markerID].width != width || markers[resultantMarker.markerID].height != height ) )
	//{
	//	delete[] debugPicture;
	//	debugPicture = 0;
	//	width = 0;
	//	height = 0;
	//}
	//if ( !debugPicture )
	//{
	//	debugPicture = new unsigned char[( ( int ) markers[resultantMarker.markerID].width * ( int ) markers[resultantMarker.markerID].height ) * 4];
	//	width = ( long ) markers[resultantMarker.markerID].width;
	//	height = ( long ) markers[resultantMarker.markerID].height;
	//}

	//for ( unsigned long y = 0; y < height; ++y )
	//{
	//	for ( unsigned long x = 0; x < width; ++x )
	//	{
	//		float Xi = ( resultantMarker.theAs[0] * x + resultantMarker.theAs[1] * y + resultantMarker.theAs[2] ) /
	//			( resultantMarker.theAs[6] * x + resultantMarker.theAs[7] * y + 1 );
	//		float Yi = ( resultantMarker.theAs[3] * x + resultantMarker.theAs[4] * y + resultantMarker.theAs[5] ) /
	//			( resultantMarker.theAs[6] * x + resultantMarker.theAs[7] * y + 1 );
	//		//debugPicture[( y * 4 * width ) + ( x * 4 )] = markers[resultantMarker.markerID].bytes[(y *markers[resultantMarker.markerID].width) + x];
	//		//debugPicture[( y * 4 * width ) + ( x * 4 ) + 1] = markers[resultantMarker.markerID].bytes[(y *markers[resultantMarker.markerID].width) + x];
	//		//debugPicture[( y * 4 * width ) + ( x * 4 ) + 2] = markers[resultantMarker.markerID].bytes[(y *markers[resultantMarker.markerID].width) + x];
	//		unsigned char value = 0;
	//		if ( picture[( ( long ) Yi * pictureWidth ) + ( long ) ( Xi )] > 255 / 2 ) value = 255;
	//		debugPicture[( y * 4 * width ) + ( x * 4 )] = value;
	//		debugPicture[( y * 4 * width ) + ( x * 4 ) + 1] = value;
	//		debugPicture[( y * 4 * width ) + ( x * 4 ) + 2] = value;
	//		//unsigned long toAdd = picture[( ( long ) Yi * pictureWidth ) + ( long ) ( Xi )] - markers[resultantMarker.markerID].bytes[( y * markers[resultantMarker.markerID].width ) + x];
	//		//std::cout << toAdd << std::endl;
	//	}
	//}
	////std::cout << "dissimilarity " << resultantMarker.dissimilarity << std::endl;
	//canGrab = true;
	//delete[] matrix;
	//delete[] vectorMult;
	quad.markerID = resultantMarker.markerID;
	quad.dissimilarity = resultantMarker.dissimilarity;
	return true;
}

MarkerPack::FoundMarkerInfo MarkerPack::getMarkerCornerDissimilarity( CompareWithMarkerInfo info )
{
	FoundMarkerInfo markerFoundInfo;
	
	float markerWidth = ( float ) cards->getMarker( info.marker )->width;
	float markerHeight = ( float ) cards->getMarker( info.marker )->height;
	//std::cout << info.marker << " " << cards->getMarker( info.marker )->width << " " << cards->getMarker( info.marker )->height << std::endl;
	float* vectorMult = new float[8];
	vectorMult[0] = ( float ) ( info.pos[0].x );
	vectorMult[1] = ( float ) ( info.pos[1].x );
	vectorMult[2] = ( float ) ( info.pos[2].x );
	vectorMult[3] = ( float ) ( info.pos[3].x );
	vectorMult[4] = ( float ) ( info.pos[0].y );
	vectorMult[5] = ( float ) ( info.pos[1].y );
	vectorMult[6] = ( float ) ( info.pos[2].y );
	vectorMult[7] = ( float ) ( info.pos[3].y );
	markerFoundInfo.markerID = info.marker;
	markerFoundInfo.points[0] = info.pos[0];
	markerFoundInfo.points[1] = info.pos[1];
	markerFoundInfo.points[2] = info.pos[2];
	markerFoundInfo.points[3] = info.pos[3];
	markerFoundInfo.dissimilarity = ULONG_MAX;
	float* matrix = new float[8 * 8];
	{
		float prematrix[8][8] =
		{
			{ 0 , 0 , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[0] ) * 0 , -( float ) ( vectorMult[0] ) * 0 } ,
			{ 0 , markerHeight , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[1] ) * 0 , -( float ) ( vectorMult[1] ) * markerHeight } ,
			{ markerWidth , markerHeight , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[2] ) *markerWidth , -( float ) ( vectorMult[2] ) * markerHeight } ,
			{ markerWidth , 0 , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[3] ) * markerWidth , -( float ) ( vectorMult[3] ) * 0 } ,

			{ 0 , 0 , 0 , 0 , 0 , 1 , -( float ) ( vectorMult[4] ) * 0 , -( float ) ( vectorMult[4] ) * 0 } ,
			{ 0 , 0 , 0 , 0 , markerHeight , 1 , -( float ) ( vectorMult[5] ) * 0 , -( float ) ( vectorMult[5] ) * markerHeight } ,
			{ 0 , 0 , 0 , markerWidth , markerHeight , 1 , -( float ) ( vectorMult[6] ) * markerWidth , -( float ) ( vectorMult[6] ) * markerHeight } ,
			{ 0 , 0 , 0 , markerWidth , 0 , 1 , -( float ) ( vectorMult[7] ) * markerWidth , -( float ) ( vectorMult[7] ) * 0 } ,
		};
		for ( unsigned int j = 0; j < 8; ++j )
		{
			for ( unsigned int i = 0; i < 8; ++i )
			{
				matrix[( j * 8 ) + i] = prematrix[j][i];
				//std::cout << matrix[( j * 8 ) + i] << std::endl;
			}
		}
	}

	//for ( int i = 0; i < 8; ++i )
	//{
	//	std::cout << vectorMult[i] << std::endl;
	//}

	//AniCardLib::Clock clock;
	//clock.Start();
	float determinant = MathHelpers::Determinant( matrix , 8 );
	//std::cout << determinant << std::endl;
	//std::cout << "clock " << clock.Stop() << std::endl;
	glm::mat4 transform;
	if ( determinant != 0 )
	{

		MathHelpers::preAdj( matrix , 8 );

		MathHelpers::Transpose( matrix , 8 );
		MathHelpers::Multiply( matrix , 1.0f / determinant , 8 );

		MathHelpers::MultiplyVector( matrix , vectorMult , 8 );
		markerFoundInfo.theAs[0] = vectorMult[0];
		markerFoundInfo.theAs[1] = vectorMult[1];
		markerFoundInfo.theAs[2] = vectorMult[2];
		markerFoundInfo.theAs[3] = vectorMult[3];
		markerFoundInfo.theAs[4] = vectorMult[4];
		markerFoundInfo.theAs[5] = vectorMult[5];
		markerFoundInfo.theAs[6] = vectorMult[6];
		markerFoundInfo.theAs[7] = vectorMult[7];

		//std::cout << "doing card check" << std::endl;
		//AniCardLib::Clock c;
		//c.Start();
		unsigned long difference = 0;
		for ( unsigned long y = 0; y < markerHeight; ++y )
		{
			for ( unsigned long x = 0; x < markerWidth; ++x )
			{
				float Xi = ( vectorMult[0] * x + vectorMult[1] * y + vectorMult[2] ) /
					( vectorMult[6] * x + vectorMult[7] * y + 1 );
				float Yi = ( vectorMult[3] * x + vectorMult[4] * y + vectorMult[5] ) /
					( vectorMult[6] * x + vectorMult[7] * y + 1 );
				if ( Xi < 0 || Yi < 0 || Xi >= info.pictureWidth || Yi >= info.pictureHeight ) continue;
				unsigned char picturePixel = 0;
				//std::cout << (( ( long ) Yi * info.pictureWidth ) + ( long ) ( Xi )) << " " << (info.pictureWidth * info.pictureHeight) << std::endl;
				//std::cout << Xi << " " << Yi << std::endl;
				if ( info.picture[( ( long ) Yi * info.pictureWidth ) + ( long ) ( Xi )] > 128 ) picturePixel = 255;
				unsigned char* thePointer = cards->getPicturePointer( info.marker );
				//std::cout << "pass picture test" << std::endl;
				unsigned long iOffset = ( unsigned long ) ( ( y * 4 * cards->getMarker( info.marker )->width ) + ( x * 4 ) );
				float grayPixel = ( ( float ) thePointer[iOffset] + ( float ) thePointer[iOffset + 1] + ( float ) thePointer[iOffset + 2] ) / 3.0f;
				unsigned char result = 0;
				if ( grayPixel > 128 ) result = 255;

				unsigned long toAdd = picturePixel - result;
				difference += toAdd * toAdd;
				if ( difference > highestDissimilarity )
				{
					difference = ULONG_MAX;
					break;
				}
				//std::cout << "pass card test" << std::endl;
				//float Xi = ( vectorMult[0] * x + vectorMult[1] * y + vectorMult[2] ) /
				//	( vectorMult[6] * x + vectorMult[7] * y + 1 );
				//float Yi = ( vectorMult[3] * x + vectorMult[4] * y + vectorMult[5] ) /
				//	( vectorMult[6] * x + vectorMult[7] * y + 1 );
				//if ( Xi < 0 || Yi < 0 || Xi >= info.pictureWidth || Yi >= info.pictureHeight ) continue;
				//unsigned char picturePixel = 0;
				//if ( info.picture[( ( long ) Yi * info.pictureWidth ) + ( long ) ( Xi )] > 128 ) picturePixel = 255;
				//unsigned long toAdd = picturePixel - markers[info.marker].bytes[( y * markers[info.marker].width ) + x];
				//difference += toAdd * toAdd;
			}
			if ( difference > highestDissimilarity )
			{
				difference = ULONG_MAX;
				break;
			}
		}
		//std::cout << "calctimepic " << c.Stop() << std::endl;
		markerFoundInfo.dissimilarity = difference;
		//std::cout << "dissimilarity " << difference << std::endl;
		
	}
	//std::cout << info.marker << std::endl;
	delete[] matrix;
	delete[] vectorMult;
	return markerFoundInfo;
}

MarkerPack::FoundMarkerInfo MarkerPack::getMarkerDissimilarity( CompareWithMarkerInfo info )
{
	CompareWithMarkerInfo compareOrientations[4];
	std::vector<std::future<FoundMarkerInfo>> orientationsToTest;
	for ( unsigned int i = 0; i < 4; ++i )
	{
		compareOrientations[i].marker = info.marker;
		compareOrientations[i].picture = info.picture;
		compareOrientations[i].pictureHeight = info.pictureHeight;
		compareOrientations[i].pictureWidth = info.pictureWidth;
		compareOrientations[i].pos[0] = info.pos[i % 4];
		compareOrientations[i].pos[1] = info.pos[( i + 1 ) % 4];
		compareOrientations[i].pos[2] = info.pos[( i + 2 ) % 4];
		compareOrientations[i].pos[3] = info.pos[( i + 3 ) % 4];
		orientationsToTest.push_back( std::async( std::launch::async , &MarkerPack::getMarkerCornerDissimilarity , this , compareOrientations[i] ) );
	}

	//AniCardLib::Clock c;
	//c.Start();

	unsigned int closest = 0;
	unsigned long smallestDissimilarity = ULONG_MAX;

	std::vector<FoundMarkerInfo> foundInfos;
	for ( unsigned int i = 0; i < orientationsToTest.size(); ++i )
	{
		orientationsToTest[i].wait();
		FoundMarkerInfo theResult = orientationsToTest[i].get();
		foundInfos.push_back( theResult );
		smallestDissimilarity = min( theResult.dissimilarity , smallestDissimilarity );
		if ( smallestDissimilarity == theResult.dissimilarity ) closest = i;
		//std::cout << "dissimilarity " << i << " " << theResult.dissimilarity << std::endl;
	}
	//std::cout << "dissimilarity closest " << foundInfos[closest].dissimilarity << std::endl;
	//std::cin.get();
	//std::cout << "mid " << c.Stop() << std::endl;
	return foundInfos[closest];
}

MarkerPack::FoundMarkerInfo MarkerPack::getSmallestDissimilarity( CompareWithMarkerInfo info )
{
	//std::cout << cards->getMarkerListSize() << std::endl;
	std::vector < std::future<FoundMarkerInfo>> closestMarkerCorners;
	for ( unsigned int i = 0; i < cards->getMarkerListSize(); ++i )
	{
		CompareWithMarkerInfo compareInfo;
		compareInfo.marker = i;
		compareInfo.picture = info.picture;
		compareInfo.pictureHeight = info.pictureHeight;
		compareInfo.pictureWidth = info.pictureWidth;
		compareInfo.pos[0] = info.pos[0];
		compareInfo.pos[1] = info.pos[1];
		compareInfo.pos[2] = info.pos[2];
		compareInfo.pos[3] = info.pos[3];
		closestMarkerCorners.push_back( std::async( std::launch::async , &MarkerPack::getMarkerDissimilarity , this , compareInfo ) );
	}

	unsigned int closest = 0;
	unsigned long smallestDissimilarity = ULONG_MAX;
	//AniCardLib::Clock c;
	//c.Start();
	std::vector<FoundMarkerInfo> foundInfos;
	for ( unsigned int i = 0; i < closestMarkerCorners.size(); ++i )
	{
		closestMarkerCorners[i].wait();
		FoundMarkerInfo theResult = closestMarkerCorners[i].get();
		foundInfos.push_back( theResult );
		smallestDissimilarity = min( theResult.dissimilarity , smallestDissimilarity );
		if ( smallestDissimilarity == theResult.dissimilarity ) closest = i;
		//std::cout << "dissimilarity " << i << " " << theResult.dissimilarity << std::endl;
	}
	//std::cout << "dissimilarity closest " << foundInfos[closest].dissimilarity << std::endl;
	//std::cin.get();
	//std::cout << "top " << c.Stop() << std::endl;
	return foundInfos[closest];
}