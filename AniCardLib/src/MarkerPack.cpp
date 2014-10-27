#include <MarkerPack.h>
#include <SOIL.h>
#include <MathHelpers.h>
#include <iostream>
#include <future>
#include <Clock.h>
MarkerPack MarkerPack::global;

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

void MarkerPack::addMarker( const char* file )
{
	Marker marker;
	int channels = 0;
	unsigned char* bytes = SOIL_load_image( file , &marker.width , &marker.height , &channels , SOIL_LOAD_RGBA );
	marker.bytes = new unsigned char[marker.height * marker.width];
	for ( int y = 0; y < marker.height; ++y )
	{
		for ( int x = 0; x < marker.width; ++x )
		{
			unsigned long iOffset = ( unsigned long ) ( ( y * 4 * this->width ) + ( x * 4 ) );
			float grayPixel = ( ( float ) bytes[iOffset] + ( float ) bytes[iOffset + 1] + ( float ) bytes[iOffset + 2] ) / 3.0f;
			unsigned long i = ( unsigned long ) ( ( y ) * this->width + ( x ) );
			marker.bytes[i] = ( unsigned char ) grayPixel;
		}
	}
	markers.push_back( marker );
}

bool MarkerPack::matchMarker( Quad& quad , const unsigned char* picture , long pictureWidth , long pictureHeight )
{
	if ( !markers.size() ) return false;
	CompareWithMarkerInfo compareOrientations[4];
	compareOrientations[0].marker = 0;
	compareOrientations[0].picture = picture;
	compareOrientations[0].pictureHeight = pictureHeight;
	compareOrientations[0].pictureWidth = pictureWidth;
	compareOrientations[0].pos[0] = quad.pt[0];
	compareOrientations[0].pos[1] = quad.pt[1];
	compareOrientations[0].pos[2] = quad.pt[2];
	compareOrientations[0].pos[3] = quad.pt[3];

	compareOrientations[1].marker = 0;
	compareOrientations[1].picture = picture;
	compareOrientations[1].pictureHeight = pictureHeight;
	compareOrientations[1].pictureWidth = pictureWidth;
	compareOrientations[1].pos[3] = quad.pt[3];
	compareOrientations[1].pos[0] = quad.pt[0];
	compareOrientations[1].pos[1] = quad.pt[1];
	compareOrientations[1].pos[2] = quad.pt[2];

	compareOrientations[2].marker = 0;
	compareOrientations[2].picture = picture;
	compareOrientations[2].pictureHeight = pictureHeight;
	compareOrientations[2].pictureWidth = pictureWidth;
	compareOrientations[2].pos[2] = quad.pt[2];
	compareOrientations[2].pos[3] = quad.pt[3];
	compareOrientations[2].pos[0] = quad.pt[0];
	compareOrientations[2].pos[1] = quad.pt[1];

	compareOrientations[3].marker = 0;
	compareOrientations[3].picture = picture;
	compareOrientations[3].pictureHeight = pictureHeight;
	compareOrientations[3].pictureWidth = pictureWidth;
	compareOrientations[3].pos[1] = quad.pt[1];
	compareOrientations[3].pos[2] = quad.pt[2];
	compareOrientations[3].pos[3] = quad.pt[3];
	compareOrientations[3].pos[0] = quad.pt[0];

	//AniCardLib::Clock c;
	//c.Start();
	std::vector<std::future<unsigned long>> orientationsToTest;
	orientationsToTest.push_back( std::async( std::launch::async , &MarkerPack::compareWithMarker , this , compareOrientations[0] ));
	orientationsToTest.push_back( std::async( std::launch::async , &MarkerPack::compareWithMarker , this , compareOrientations[1] ) );
	orientationsToTest.push_back( std::async( std::launch::async , &MarkerPack::compareWithMarker , this , compareOrientations[2] ) );
	orientationsToTest.push_back( std::async( std::launch::async , &MarkerPack::compareWithMarker , this , compareOrientations[3] ) );

	unsigned int closest = 0;
	unsigned long smallestDissimilarity = ULONG_MAX;


	for ( unsigned int i = 0; i < 4; ++i )
	{
		orientationsToTest[i].wait();
		unsigned long theResult = orientationsToTest[i].get();
		smallestDissimilarity = min( theResult , smallestDissimilarity );
		if ( smallestDissimilarity == theResult ) closest = i;
	}
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

	//if ( debugPicture && (markerWidth != width || markerHeight != height ))
	//{
	//	delete[] debugPicture;
	//	debugPicture = 0;
	//	width = 0;
	//	height = 0;
	//}
	//if ( !debugPicture )
	//{
	//	debugPicture = new unsigned char[((int)markerWidth * (int)markerHeight) * 4];
	//	width = ( long ) markerWidth;
	//	height = ( long ) markerHeight;
	//}
	//while ( numUsing ) std::cout <<"using at markerpack" << std::endl;
	//canGrab = false;
	//unsigned long difference = 0;
	//for ( unsigned long y = 0; y < height; ++y )
	//{
	//	for ( unsigned long x = 0; x < width; ++x )
	//	{
	//		float Xi = ( vectorMult[0] * x + vectorMult[1] * y + vectorMult[2] ) /
	//			(vectorMult[6] * x + vectorMult[7] * y + 1 );
	//		float Yi = ( vectorMult[3] * x + vectorMult[4] * y + vectorMult[5] ) /
	//			( vectorMult[6] * x + vectorMult[7] * y + 1 );
	//		debugPicture[( y * 4 * width ) + ( x * 4 )] = picture[( (long)Yi * pictureWidth ) + (long)( Xi )];
	//		debugPicture[( y * 4 * width ) + ( x * 4 ) + 1] = picture[( ( long ) Yi * pictureWidth ) + ( long ) ( Xi )];
	//		debugPicture[( y * 4 * width ) + ( x * 4 ) + 2] = picture[( ( long ) Yi * pictureWidth ) + ( long ) ( Xi )];
	//		unsigned long toAdd = picture[( ( long ) Yi * pictureWidth ) + ( long ) ( Xi )] - markers[0].bytes[( y * markers[0].width ) + x];
	//		difference += toAdd * toAdd;
	//	}
	//}
	//std::cout << difference << std::endl;
	//canGrab = true;
	//delete[] matrix;
	//delete[] vectorMult;
	return true;
}

unsigned long MarkerPack::compareWithMarker( CompareWithMarkerInfo info )
{
	float markerWidth = ( float ) markers[info.marker].width;
	float markerHeight = ( float ) markers[info.marker].height;
	float* vectorMult = new float[8];
	vectorMult[0] = ( float ) ( info.pos[0].x );
	vectorMult[1] = ( float ) ( info.pos[1].x );
	vectorMult[2] = ( float ) ( info.pos[2].x );
	vectorMult[3] = ( float ) ( info.pos[3].x );
	vectorMult[4] = ( float ) ( info.pos[0].y );
	vectorMult[5] = ( float ) ( info.pos[1].y );
	vectorMult[6] = ( float ) ( info.pos[2].y );
	vectorMult[7] = ( float ) ( info.pos[3].y );

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
			}
		}
	}

	float determinant = MathHelpers::Determinant( matrix , 8 );
	glm::mat4 transform;
	if ( determinant != 0 )
	{
		AniCardLib::Clock clock;
		
		MathHelpers::preAdj( matrix , 8 );
		
		MathHelpers::Transpose( matrix , 8 );

		MathHelpers::Multiply( matrix , 1.0f / determinant , 8 );
		
		//clock.Start();
		MathHelpers::MultiplyVector( matrix , vectorMult , 8 );
		//std::cout << "clock " << clock.Stop() << std::endl;
		//if ( debugPicture && ( markerWidth != width || markerHeight != height ) )
		//{
		//	delete[] debugPicture;
		//	debugPicture = 0;
		//	width = 0;
		//	height = 0;
		//}
		//if ( !debugPicture )
		//{
		//	debugPicture = new unsigned char[( ( int ) markerWidth * ( int ) markerHeight ) * 4];
		//	width = ( long ) markerWidth;
		//	height = ( long ) markerHeight;
		//}

		//AniCardLib::Clock c;
		//c.Start();
		unsigned long difference = 0;
		for ( unsigned long y = 0; y < height; ++y )
		{
			for ( unsigned long x = 0; x < width; ++x )
			{
				float Xi = ( vectorMult[0] * x + vectorMult[1] * y + vectorMult[2] ) /
					( vectorMult[6] * x + vectorMult[7] * y + 1 );
				float Yi = ( vectorMult[3] * x + vectorMult[4] * y + vectorMult[5] ) /
					( vectorMult[6] * x + vectorMult[7] * y + 1 );
				if ( Xi < 0 || Yi < 0 || Xi >= info.pictureWidth || Yi >= info.pictureHeight ) continue;
				unsigned long toAdd = info.picture[( ( long ) Yi * info.pictureWidth ) + ( long ) ( Xi )] - markers[info.marker].bytes[( y * markers[info.marker].width ) + x];
				difference += toAdd * toAdd;
			}
		}
		//std::cout << "calctimepic " << c.Stop() << std::endl;
		return difference;
	}

	delete[] matrix;
	delete[] vectorMult;
	return ULONG_MAX;
}