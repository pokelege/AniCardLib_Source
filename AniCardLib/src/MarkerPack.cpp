#include <MarkerPack.h>
#include <SOIL.h>
#include <MathHelpers.h>
#include <iostream>
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
	marker.bytes = SOIL_load_image( file , &marker.width , &marker.height , &marker.channels , SOIL_LOAD_AUTO );
	markers.push_back( marker );
}

bool MarkerPack::matchMarker( Quad& quad , const unsigned char* picture , long pictureWidth , long pictureHeight )
{
	if ( !markers.size() ) return false;
	float markerWidth = ( float ) markers[0].width;
	float markerHeight = ( float ) markers[0].height;
	float* vectorMult = new float[8];
	vectorMult[0] = ( float ) ( quad.pt[0].x );
	vectorMult[1] = ( float ) ( quad.pt[1].x );
	vectorMult[2] = ( float ) ( quad.pt[2].x );
	vectorMult[3] = ( float ) ( quad.pt[3].x );
	vectorMult[4] = ( float ) ( quad.pt[0].y );
	vectorMult[5] = ( float ) ( quad.pt[1].y );
	vectorMult[6] = ( float ) ( quad.pt[2].y );
	vectorMult[7] = ( float ) ( quad.pt[3].y );

	float* matrix = new float[8 * 8];
	{
		float prematrix[8][8] =
		{
			{ 0 , 0 , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[0] ) * 0 , -( float ) ( vectorMult[0] ) * 0 } ,
			{ markerWidth , 0 , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[1] ) * markerWidth , -( float ) ( vectorMult[1] ) * 0 } ,
			{ markerWidth , markerHeight , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[2] ) *markerWidth , -( float ) ( vectorMult[2] ) * markerHeight } ,
			{ 0 , markerHeight , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[3] ) * 0 , -( float ) ( vectorMult[3] ) * markerHeight } ,

			{ 0 , 0 , 0 , 0 , 0 , 1 , -( float ) ( vectorMult[4] ) * 0 , -( float ) ( vectorMult[4] ) * 0 } ,
			{ 0 , 0 , 0 , markerWidth ,0 , 1 , -( float ) ( vectorMult[5] ) * markerWidth , -( float ) ( vectorMult[5] ) * 0} ,
			{ 0 , 0 , 0 , markerWidth , markerHeight , 1 , -( float ) ( vectorMult[6] ) * markerWidth , -( float ) ( vectorMult[6] ) * markerHeight } ,
			{ 0 , 0 , 0 , 0 , markerHeight , 1 , -( float ) ( vectorMult[7] ) * 0 , -( float ) ( vectorMult[7] ) * markerHeight } ,
		};
		for ( unsigned int j = 0; j < 8; ++j )
		{
			for ( unsigned int i = 0; i < 8; ++i )
			{
				matrix[( j * 8 ) + i] = prematrix[j][i];
				//if ( ( j * 8 ) + i >= 8 * 8 ) std::cout << "went out" << std::endl;
			}
		}
	}

	//std::cout << quadResult.pt[0].x - 595.164f << std::endl;
	float determinant = MathHelpers::Determinant( matrix , 8 );
	//std::cout << determinant << std::endl;
	glm::mat4 transform;
	if ( determinant != 0 )
	{
		MathHelpers::preAdj( matrix , 8 );
		MathHelpers::Transpose( matrix , 8 );
		MathHelpers::Multiply( matrix , 1.0f / determinant , 8 );
		MathHelpers::MultiplyVector( matrix , vectorMult , 8 );
		//std::cout << "{" << vectorMult[0] << "," << vectorMult[4] << "}" << std::endl;
		//std::cout << "{" << vectorMult[1] << "," << vectorMult[5] << "}" << std::endl;
		//std::cout << "{" << vectorMult[2] << "," << vectorMult[6] << "}" << std::endl;
		//std::cout << "{" << vectorMult[3] << "," << vectorMult[7] << "}" << std::endl;

		float a = ( vectorMult[2] * vectorMult[5] ) - ( vectorMult[4] * vectorMult[3] );
		float b = ( vectorMult[0] * vectorMult[5] ) - ( vectorMult[1] * vectorMult[4] );
		float c = ( vectorMult[0] * vectorMult[3] ) - ( vectorMult[2] * vectorMult[1] );
		//std::cout << "{" << a << "," << b << "," << c << "}" << std::endl;
		glm::vec4 translateVector;
		translateVector.z = -powf( 1.0f / ( ( a * a ) + ( b* b ) + ( c* c ) ) , 0.25f );
		std::cout << translateVector.z << std::endl;
		translateVector.x = -vectorMult[6] * translateVector.z;
		translateVector.y = -vectorMult[7] * translateVector.z;
		translateVector.w = 1;
		glm::vec4 row0( -vectorMult[0] * translateVector.z , -vectorMult[2] * translateVector.z , vectorMult[4] * translateVector.z , translateVector.x );
		glm::vec4 row1( -vectorMult[1] * translateVector.z , -vectorMult[3] * translateVector.z , vectorMult[5] * translateVector.z , translateVector.y );
		glm::vec4 row2 = glm::vec4( glm::cross( glm::vec3( row0 ) , glm::vec3( row1 ) ) , translateVector.z );
		glm::mat4 theResultingMatrix( row0 , row1 , row2 , glm::vec4( 0 , 0 , 0 , 1 ) );
		transform = glm::transpose( theResultingMatrix );
		//translated /= translated.w;
		//std::cout << "{" << translateVector.x << "," << translateVector.y << "," << translateVector.z << "}" << std::endl;
	}

	if ( debugPicture && (markerWidth != width || markerHeight != height ))
	{
		delete[] debugPicture;
		debugPicture = 0;
		width = 0;
		height = 0;
	}
	if ( !debugPicture )
	{
		debugPicture = new unsigned char[((int)markerWidth * (int)markerHeight) * 4];
		width = ( long ) markerWidth;
		height = ( long ) markerHeight;
	}
	while ( numUsing ) std::cout <<"using at markerpack" << std::endl;
	canGrab = false;
	for ( unsigned long y = 0; y < height; ++y )
	{
		for ( unsigned long x = 0; x < width; ++x )
		{
			float Xi = ( vectorMult[0] * x + vectorMult[1] * y + vectorMult[2] ) /
				(vectorMult[6] * x + vectorMult[7] * y + 1 );
			float Yi = ( vectorMult[3] * x + vectorMult[4] * y + vectorMult[5] ) /
				( vectorMult[6] * x + vectorMult[7] * y + 1 );
			debugPicture[( y * 4 * width ) + ( x * 4 )] = picture[( (long)Yi * pictureWidth ) + (long)( Xi )];
			debugPicture[( y * 4 * width ) + ( x * 4 ) + 1] = picture[( ( long ) Yi * pictureWidth ) + ( long ) ( Xi )];
			debugPicture[( y * 4 * width ) + ( x * 4 ) + 2] = picture[( ( long ) Yi * pictureWidth ) + ( long ) ( Xi )];
			//glm::vec4 projectionPos = transform * glm::vec4( x , y , 0 , 1 );
			//std::cout << projectionPos.x << ", " << projectionPos.y  << ", " << projectionPos.z <<" " << projectionPos.w << std::endl;
			//glm::vec3 projectionPos2 = glm::vec3(projectionPos) / projectionPos.w;
			//glm::vec2 realPos = glm::vec2( projectionPos2 ) / projectionPos2.z;
			//std::cout <<Xi << "," << Yi << std::endl;
		}
	}
	canGrab = true;
	delete[] matrix;
	delete[] vectorMult;
	return true;
}