#include <ARMarkerDetector.h>
#include <PictureFetcher.h>
#include <glm.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <iostream>
#include <Clock.h>
#include <chrono>
#include <MathHelpers.h>
#include <MarkerPack.h>
ARMarkerDetector ARMarkerDetector::global;
using namespace AniCardLib;

bool ARMarkerDetector::getPicture( unsigned char** bytes , long* width , long* height )
{
	if ( canGrab && bytes )
	{
		++numUsing;
		//std::cout << numUsing << std::endl;
		*bytes = copiedPictureInstance;
		if ( width ) *width = this->width;
		if ( height ) *height = this->height;
		return true;
	}
	return false;
}
bool ARMarkerDetector::finishedUsing()
{
	//std::cout << numUsing << std::endl;
	if ( numUsing > 0 ) { --numUsing; return true; }
	else return false;
}

bool ARMarkerDetector::getMarkerFound( std::vector<FoundMarkerInfo>** bytes )
{
	if ( canGrabMarkerFound && bytes )
	{
		++numUsingMarkerFound;
		//std::cout << numUsing << std::endl;
		*bytes = &toSend;
		return true;
	}
	return false;
}
bool ARMarkerDetector::finishedUsingMarkerFound()
{
	if ( numUsingMarkerFound > 0 ) { --numUsingMarkerFound; return true; }
	else return false;
}

void ARMarkerDetector::findCard( PictureFetcher* thePhoto , MarkerPack* markerPack )
{
	if ( !thePhoto ) return;
	if ( runningThread )
	{
		if ( runningThread->wait_for( std::chrono::milliseconds( 0 ) ) != std::future_status::ready ) return;
		delete runningThread;
		runningThread = 0;
	}

	unsigned char* bytes;
	long width , height;
	if ( !thePhoto->getPicture( &bytes , &width , &height ) )
	{
		std::cout << "generatingPhoto" << std::endl;
		return;
	}
	while ( numUsing );// std::cout << numUsing << std::endl;;
	canGrab = false;
	if ( this->width != width || this->height != height )
	{
		if ( copiedPictureInstance ) delete[] copiedPictureInstance;
		if ( grayscaleImage ) delete[] grayscaleImage;
		if ( gradientDirection ) delete[] gradientDirection;
		if ( gradientIntensity ) delete[] gradientIntensity;
		copiedPictureInstance = new unsigned char[width * height * 4];
		grayscaleImage = new unsigned char[width * height];
		gradientDirection = new float[width * height];
		gradientIntensity = new float[width * height];
		this->width = width;
		this->height = height;
	}
	memcpy( copiedPictureInstance , bytes , sizeof( unsigned char ) * ( width * height * 4 ) );
	thePhoto->finishedUsing();
	canGrab = true;

	runningThread = new std::future<void>( std::async( std::launch::async , &ARMarkerDetector::_findCard , this, markerPack ) );
}
void ARMarkerDetector::_findCard( MarkerPack* markerPack )
{
	std::vector<Line> theLines;
	Clock c;
	c.Start();
	int GxMask[3][3] =
	{
		{ -1 , 0 , 1 } ,
		{ -1 , 0 , 1 } ,
		{ -1 , 0 , 1 }
	};
	int GyMask[3][3] =
	{
		{ 1 , 1 , 1 } ,
		{ 0 , 0 , 0 } ,
		{ -1 , -1 , -1 }
	};

	int gaussianMask[5][5] =
	{
		{ 2 , 4 , 5 , 4 , 2 } ,
		{ 4 , 9 , 12 , 9 , 4 } ,
		{ 5 , 12 , 15 , 12 , 5 } ,
		{ 4 , 9 , 12 , 9 , 4 } ,
		{ 2 , 4 , 5 , 4 , 2 }
	};

	//grayscaleImage
	for ( long y = 0; y < height; ++y )
	{
		for ( long x = 0; x < width; ++x )
		{
			unsigned long iOffset = ( unsigned long ) ( ( y * 4 * this->width ) + ( x * 4 ) );
			float grayPixel = ( ( float ) copiedPictureInstance[iOffset] + ( float ) copiedPictureInstance[iOffset + 1] + ( float ) copiedPictureInstance[iOffset + 2] ) / 3.0f;
			unsigned long i = ( unsigned long ) ( ( y ) * this->width + ( x ) );
			grayscaleImage[i] = ( unsigned char ) grayPixel;
		}
	}

	//debug
	//{
	//	while ( numUsing ) std::cout << numUsing << std::endl;;
	//	canGrab = false;
	//	for ( long y = 0; y < height; ++y )
	//	{
	//		for ( long x = 0; x < width; ++x )
	//		{
	//			unsigned long i = ( unsigned long ) ( ( y ) * this->width + ( x ) );
	//			unsigned char grayPixel = grayscaleImage[i];
	//			unsigned long iOffset = ( unsigned long ) ( ( y * 4 * this->width ) + ( x * 4 ) );
	//			copiedPictureInstance[iOffset] = grayPixel;
	//			copiedPictureInstance[iOffset+1] = grayPixel;
	//			copiedPictureInstance[iOffset+2] = grayPixel;
	//		}
	//	}
	//	canGrab = true;
	//}

	//gaussianBlur
	for ( long row = 2; row < height - 2; ++row )
	{
		for ( long col = 2; col < width - 2; ++col )
		{
			float newPixel = 0;
			for ( long rowOffset = -2; rowOffset <= 2; ++rowOffset )
			{
				for ( long colOffset = -2; colOffset <= 2; ++colOffset )
				{
					//this is real row and col
					long rowTotal = row + rowOffset;
					long colTotal = col + colOffset;
					//pixel location on image
					unsigned long iOffset = ( unsigned long ) ( (rowTotal * this->width) + (colTotal ) );
					newPixel += grayscaleImage[iOffset] * gaussianMask[2 + rowOffset][2 + colOffset];
				}
			}
			//real pixel location
			unsigned long i = ( unsigned long ) ( ( row ) * this->width + ( col ) );
			//apply blur
			grayscaleImage[i] = ( unsigned char ) glm::clamp( newPixel / 159 , 0.0f , ( float ) UCHAR_MAX );
			//std::cout << newPixel / 159 << std::endl;
		}
	}

	//gradientChecker
	for ( long row = 1; row < height - 1; ++row )
	{
		for ( long col = 1; col < width - 1; ++col )
		{
			float Gx = 0 , Gy = 0;

			for ( long rowOffset = -1; rowOffset <= 1; ++rowOffset )
			{
				for ( long colOffset = -1; colOffset <= 1; ++colOffset )
				{
					long rowTotal = row + rowOffset;
					long colTotal = col + colOffset;
					unsigned long iOffset = ( unsigned long ) ( (rowTotal* this->width) + colTotal );

					Gx += grayscaleImage[iOffset] * GxMask[rowOffset + 1][colOffset + 1];
					Gy += grayscaleImage[iOffset] * GyMask[rowOffset + 1][colOffset + 1];
				}
			}
			unsigned long i = ( unsigned long ) ( ( row * this->width) + ( col ) );
			gradientDirection[i] = (( atan2f( Gy , Gx ) * 180 ) / ( float ) M_PI) + 90;
			if ( gradientDirection[i] < 0 ) gradientDirection[i] += 360;
			if ( gradientDirection[i] >= 360 ) gradientDirection[i] -= 360;
			//std::cout << gradientDirection[i] << std::endl;
			gradientIntensity[i] = sqrt( ( Gx * Gx ) + ( Gy * Gy ) );
		}
	}
	findLines( theLines );
	float angleThreshold = 15;
	float upperThreshold = 100.0f;
	//std::cout << "before " << theLines.size() <<std::endl;
	for ( unsigned int i = 0; i < theLines.size(); ++i )
	{
		Line* lineToConnect = &theLines[i];
		for ( unsigned int j = i + 1; j < theLines.size(); ++j )
		{
			Line* lineEnd = &theLines[j];
			if ( abs( lineEnd->angle - lineToConnect->angle ) > angleThreshold ) continue;
			Line* lineStart = lineToConnect;
			if ( glm::length( glm::vec2( lineEnd->end ) - glm::vec2( lineStart->start ) ) < glm::length( glm::vec2( lineStart->end ) - glm::vec2( lineEnd->start ) ) )
			{
				Line* tempLine = lineEnd;
				lineEnd = lineStart;
				lineStart = tempLine;
			}

			glm::vec2 connectingLineTest = glm::vec2( lineEnd->start ) - glm::vec2( lineStart->end );
			glm::vec2 normalizedConnectingLineTest = glm::normalize( connectingLineTest );
			float connectingLineAngleTest = ( atan2f( -connectingLineTest.y , connectingLineTest.x ) * 180 ) / ( float ) M_PI;
			if ( connectingLineAngleTest < 0 ) connectingLineAngleTest += 360;
			float angleAverage = ( lineStart->angle + lineEnd->angle ) / 2;
			if ( abs( connectingLineAngleTest - angleAverage ) > angleThreshold ) continue;

			//must check if disconnecting later
			glm::vec2 currentPosToCheck = glm::vec2( lineStart->end );
			glm::ivec2 lastPosToCheck = lineStart->end;
			bool compatible = true;
			while ( glm::length( glm::vec2( lastPosToCheck ) - glm::vec2( lineStart->end ) ) < glm::length( glm::vec2( connectingLineTest ) ) && compatible )
			{
				currentPosToCheck += normalizedConnectingLineTest;
				if ( glm::ivec2( currentPosToCheck ) == lastPosToCheck ) continue;
				lastPosToCheck = glm::ivec2( currentPosToCheck );
				unsigned long imageIndex = ( unsigned long ) ( ( lastPosToCheck.y  * this->width ) + ( lastPosToCheck.x ) );
				compatible = ( gradientIntensity[imageIndex] > upperThreshold ) && ( abs( ( gradientDirection[imageIndex] ) - angleAverage ) <= angleThreshold );
			}
			if ( !compatible ) continue;
			glm::vec2 newLineAngleVec = glm::vec2( lineEnd->end ) - glm::vec2( lineStart->start );
			float newLineAngle = ( atan2f( -newLineAngleVec.y , newLineAngleVec.x ) * 180 ) / ( float ) M_PI;
			if ( newLineAngle < 0 ) newLineAngle += 360;
			lineToConnect->start = lineStart->start;
			lineToConnect->end = lineEnd->end;
			lineToConnect->angle = newLineAngle;

			unsigned int lineToRemove = lineEnd - &theLines[0];
			if ( lineEnd == lineToConnect ) lineToRemove = lineStart - &theLines[0];
			theLines.erase( theLines.begin() + lineToRemove );
			--i;
			break;
		}
	}
	//debug
	//{
	//	while ( numUsing ) std::cout << numUsing << std::endl;
	//	canGrab = false;

	//	for ( unsigned int i = 0; i < theLines.size(); ++i )
	//	{
	//		unsigned short randomColor = rand() % 255;
	//		unsigned short randomColor2 = rand() % 255;
	//		unsigned short randomColor3 = rand() % 255;
	//		glm::vec2 normalized = glm::normalize( glm::vec2( theLines[i].end ) - glm::vec2( theLines[i].start ) );
	//		glm::vec2 testPixel = glm::vec2( theLines[i].start );
	//		glm::ivec2 lastPixel = theLines[i].start;
	//		unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
	//		//std::cout << lastPixel.y << std::endl;
	//		//std::cout << lastPixel.x << std::endl;
	//		copiedPictureInstance[iOffset] = ( unsigned char ) 0;
	//		copiedPictureInstance[iOffset + 1] = ( unsigned char ) 255;
	//		copiedPictureInstance[iOffset + 2] = ( unsigned char) 0;
	//		while ( glm::ivec2( testPixel ) == lastPixel )
	//		{
	//			testPixel += normalized;
	//		}
	//		lastPixel = glm::ivec2( testPixel );
	//		while ( glm::length( testPixel - glm::vec2( theLines[i].start ) ) < glm::length( glm::vec2( theLines[i].end ) - glm::vec2( theLines[i].start ) ) && lastPixel.x >= 0 && lastPixel.x < this->width && lastPixel.y >= 0 && lastPixel.y < this->height )
	//		{
	//			unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
	//			if ( copiedPictureInstance[iOffset + 1] != 255 && copiedPictureInstance[iOffset + 2] != 255 )
	//			{
	//				
	//				copiedPictureInstance[iOffset] = ( unsigned char ) 255;
	//				copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
	//				copiedPictureInstance[iOffset + 2] = ( unsigned char ) 0;
	//			}
	//			while ( glm::ivec2( testPixel ) == lastPixel )
	//			{
	//				testPixel += normalized;
	//			}
	//			lastPixel = glm::ivec2( testPixel );
	//		}
	//		iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
	//		copiedPictureInstance[iOffset] = ( unsigned char ) 0;
	//		copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
	//		copiedPictureInstance[iOffset + 2] = ( unsigned char ) 255;
	//	}
	//	canGrab = true;
	//}
	//std::cout << "after " << theLines.size() << std::endl;
	for ( unsigned int i = 0; i < theLines.size(); ++i )
	{
		glm::vec2 normalized = glm::normalize( glm::vec2( theLines[i].end ) - glm::vec2( theLines[i].start ) );
		bool extendable = true;
		glm::vec2 testPixel = glm::vec2(theLines[i].start);
		glm::ivec2 lastPixel = theLines[i].start;
		while ( glm::ivec2( testPixel ) == lastPixel )
		{
			testPixel -= normalized;
		}
		lastPixel = glm::ivec2( testPixel );
		while ( extendable && lastPixel.x >= 0 && lastPixel.x < width && lastPixel.y >= 0 && lastPixel.y < height )
		{
			unsigned long imageIndex = ( unsigned long ) ( ( lastPixel.y ) * this->width + ( lastPixel.x ) );
			if ( gradientIntensity[imageIndex] > upperThreshold && abs( ( gradientDirection[imageIndex] ) - theLines[i].angle ) <= angleThreshold )
			{
				theLines[i].start = lastPixel;
				//change angle on extension
			}
			else
			{
				extendable = false;

				glm::vec2 tempTestPixel = testPixel;
				glm::ivec2 tempLastPixel = lastPixel;
				glm::vec2 perpNormal( normalized.y , -normalized.x );
				while ( glm::ivec2( tempTestPixel ) == tempLastPixel )
				{
					tempTestPixel += perpNormal;
					//std::cout << normalized.x << " " << normalized.y << std::endl;
				}
				tempLastPixel = glm::ivec2(tempTestPixel);
				if ( tempLastPixel.x >= 0 && tempLastPixel.x < width && tempLastPixel.y >= 0 && tempLastPixel.y < height )
				{
					unsigned long imageIndex2 = ( unsigned long ) ( ( tempLastPixel.y  * this->width) + ( tempLastPixel.x ) );
					if ( gradientIntensity[imageIndex] > upperThreshold && abs( ( gradientDirection[imageIndex2] ) - theLines[i].angle ) <= angleThreshold )
					{
						theLines[i].start = tempLastPixel;
						lastPixel = tempLastPixel;
						testPixel = tempTestPixel;
						extendable = true;
					}
				}
				if ( !extendable )
				{
					tempTestPixel = testPixel;
					tempLastPixel = lastPixel;
					perpNormal = glm::vec2( -normalized.y , normalized.x );
					while ( glm::ivec2( tempTestPixel ) == tempLastPixel )
					{
						tempTestPixel += perpNormal;
					}
					tempLastPixel = glm::ivec2( tempTestPixel );
					if ( tempLastPixel.x >= 0 && tempLastPixel.x < width && tempLastPixel.y >= 0 && tempLastPixel.y < height )
					{
						unsigned long imageIndex2 = ( unsigned long ) ( ( tempLastPixel.y * this->width ) + ( tempLastPixel.x ) );
						if ( gradientIntensity[imageIndex] > upperThreshold && abs( ( gradientDirection[imageIndex2] ) - theLines[i].angle ) <= angleThreshold )
						{
							theLines[i].start = tempLastPixel;
							lastPixel = tempLastPixel;
							testPixel = tempTestPixel;
							extendable = true;
						}
					}
				}
			}
			while ( glm::ivec2( testPixel ) == lastPixel )
			{
				testPixel -= normalized;
			}
			lastPixel = glm::ivec2( testPixel );
		}

		extendable = true;
		testPixel = glm::vec2( theLines[i].end );
		lastPixel = theLines[i].end;
		while ( glm::ivec2( testPixel ) == lastPixel )
		{
			testPixel += normalized;
		}
		lastPixel = glm::ivec2( testPixel );
		while ( extendable && lastPixel.x >= 0 && lastPixel.x < width && lastPixel.y >= 0 && lastPixel.y < height )
		{
			unsigned long imageIndex = ( unsigned long ) ( ( lastPixel.y * this->width) + ( lastPixel.x ) );
			if ( gradientIntensity[imageIndex] > upperThreshold &&  abs( ( gradientDirection[imageIndex] ) - theLines[i].angle ) <= angleThreshold )
			{
				theLines[i].end = lastPixel;
			}
			else
			{
				extendable = false;

				glm::vec2 tempTestPixel = testPixel;
				glm::ivec2 tempLastPixel = lastPixel;
				glm::vec2 perpNormal( normalized.y , -normalized.x );
				while ( glm::ivec2( tempTestPixel ) == tempLastPixel )
				{
					tempTestPixel += perpNormal;
				}
				tempLastPixel = glm::ivec2( tempTestPixel );
				if ( tempLastPixel.x >= 0 && tempLastPixel.x < width && tempLastPixel.y >= 0 && tempLastPixel.y < height )
				{
					unsigned long imageIndex2 = ( unsigned long ) ( ( tempLastPixel.y * this->width) + ( tempLastPixel.x ) );
					if ( gradientIntensity[imageIndex] > upperThreshold && abs( ( gradientDirection[imageIndex2] ) - theLines[i].angle ) <= angleThreshold )
					{
						theLines[i].end = tempLastPixel;
						lastPixel = tempLastPixel;
						testPixel = tempTestPixel;
						extendable = true;
					}
				}
				if ( !extendable )
				{
					tempTestPixel = testPixel;
					tempLastPixel = lastPixel;
					perpNormal = glm::vec2( -normalized.y , normalized.x );
					while ( glm::ivec2( tempTestPixel ) == tempLastPixel )
					{
						tempTestPixel += perpNormal;
					}
					tempLastPixel = glm::ivec2( tempTestPixel );
					if ( tempLastPixel.x >= 0 && tempLastPixel.x < width && tempLastPixel.y >= 0 && tempLastPixel.y < height )
					{
						unsigned long imageIndex2 = ( unsigned long ) ( ( tempLastPixel.y ) * this->width + ( tempLastPixel.x ) );
						if ( gradientIntensity[imageIndex] > upperThreshold && abs( ( gradientDirection[imageIndex2] ) - theLines[i].angle ) <= angleThreshold )
						{
							theLines[i].end = tempLastPixel;
							lastPixel = tempLastPixel;
							testPixel = tempTestPixel;
							extendable = true;
						}
					}
				}
			}
			while ( glm::ivec2( testPixel ) == lastPixel )
			{
				testPixel += normalized;
			}
			lastPixel = glm::ivec2( testPixel );
		}
	}

	//debug
	//{
	//	while ( numUsing ) std::cout << numUsing << std::endl;
	//	canGrab = false;

	//	for ( unsigned int i = 0; i < theLines.size(); ++i )
	//	{
	//		//if ( theLines[i].angle > 260 && theLines[i].angle < 290 )
	//		//{
	//			glm::vec2 normalized = glm::normalize( glm::vec2( theLines[i].end ) - glm::vec2( theLines[i].start ) );
	//			glm::vec2 testPixel = glm::vec2( theLines[i].start );
	//			glm::ivec2 lastPixel = theLines[i].start;
	//			unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
	//			//std::cout << lastPixel.y << std::endl;
	//			//std::cout << lastPixel.x << std::endl;
	//			copiedPictureInstance[iOffset] = ( unsigned char ) 0;
	//			copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
	//			copiedPictureInstance[iOffset + 2] = ( unsigned char ) 255;
	//			while ( glm::ivec2( testPixel ) == lastPixel )
	//			{
	//				testPixel += normalized;
	//			}
	//			lastPixel = glm::ivec2( testPixel );
	//			while ( glm::length( testPixel - glm::vec2( theLines[i].start ) ) < glm::length( glm::vec2( theLines[i].end ) - glm::vec2( theLines[i].start ) ) && lastPixel.x >= 0 && lastPixel.x < width && lastPixel.y >= 0 && lastPixel.y < height )
	//			{
	//				unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
	//				if ( copiedPictureInstance[iOffset + 2] != 255 && copiedPictureInstance[iOffset + 1] != 255 )
	//				{
	//					
	//					copiedPictureInstance[iOffset] = ( unsigned char ) 255;
	//					copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
	//					copiedPictureInstance[iOffset + 2] = ( unsigned char ) 0;
	//				}
	//				
	//				while ( glm::ivec2( testPixel ) == lastPixel )
	//				{
	//					testPixel += normalized;
	//				}
	//				lastPixel = glm::ivec2( testPixel );
	//			}
	//			iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
	//			copiedPictureInstance[iOffset] = ( unsigned char ) 0;
	//			copiedPictureInstance[iOffset + 1] = ( unsigned char ) 255;
	//			copiedPictureInstance[iOffset + 2] = ( unsigned char ) 0;
	//		//}
	//	}
	//	canGrab = true;
	//}

	std::vector<Quad> quadResults;

	bool constructing = true;
	while ( constructing && theLines.size() >=4 )
	{
		ConstructingQuad quad;
		
		bool foundQuad = findQuad( quad , theLines , 0 );
		if ( !foundQuad ) constructing = false;
		else
		{
			
			Quad quadResult;
			glm::vec2 normal = glm::normalize( glm::vec2(quad.line[0]->end - quad.line[0]->start) );
			glm::vec2 lineToDot = glm::vec2( quad.line[1]->start - quad.line[0]->start );
			float theDot = glm::dot( lineToDot , normal );
			quadResult.pt[0] = glm::ivec2(theDot * normal) + quad.line[0]->start;

			normal = glm::normalize( glm::vec2( quad.line[1]->end - quad.line[1]->start ) );
			lineToDot = glm::vec2( quad.line[2]->start - quad.line[1]->start );
			theDot = glm::dot( lineToDot , normal );
			quadResult.pt[1] = glm::ivec2( theDot * normal ) + quad.line[1]->start;

			normal = glm::normalize( glm::vec2( quad.line[2]->end - quad.line[2]->start ) );
			lineToDot = glm::vec2( quad.line[3]->start - quad.line[2]->start );
			theDot = glm::dot( lineToDot , normal );
			quadResult.pt[2] = glm::ivec2( theDot * normal ) + quad.line[2]->start;

			normal = glm::normalize( glm::vec2( quad.line[3]->end - quad.line[3]->start ) );
			lineToDot = glm::vec2( quad.line[0]->start - quad.line[3]->start );
			theDot = glm::dot( lineToDot , normal );
			quadResult.pt[3] = glm::ivec2( theDot * normal ) + quad.line[3]->start;

			markerPack->matchMarker( quadResult , grayscaleImage, width, height );
			//float* vectorMult = new float[8];
			//vectorMult[0] = ( float ) ( quadResult.pt[0].x - 618.776f ) / 893.48f;
			//vectorMult[1] = ( float ) ( quadResult.pt[1].x - 618.776f) / 893.48f ;
			//vectorMult[2] = ( float ) ( quadResult.pt[2].x - 618.776f) / 893.48f ;
			//vectorMult[3] = ( float ) ( quadResult.pt[3].x - 618.776f) / 893.48f ;
			//vectorMult[4] = ( float ) ( quadResult.pt[0].y - 375.085f ) / 897.393f;
			//vectorMult[5] = ( float ) ( quadResult.pt[1].y - 375.085f ) / 897.393f;
			//vectorMult[6] = ( float ) ( quadResult.pt[2].y - 375.085f ) / 897.393f;
			//vectorMult[7] = ( float ) ( quadResult.pt[3].y - 375.085f ) / 897.393f;

			//float* matrix = new float[8 * 8];
			//{
			//	float prematrix[8][8] =
			//	{
			//		{ -0.01f , -0.01f , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[0] ) * -0.01f , -( float ) ( vectorMult[0] ) * -0.01f } ,
			//		{ 0.01f , -0.01f , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[1] ) * 0.01f , -( float ) ( vectorMult[1] ) * -0.01f } ,
			//		{ 0.01f , 0.01f , 1 , 0 , 0 , 0 , -( float ) ( vectorMult[2] ) * 0.01f , -( float ) ( vectorMult[2] ) * 0.01f } ,
			//		{ -0.01f , 0.01f, 1 , 0 , 0 , 0 , -( float ) ( vectorMult[3] ) * -0.01f , -( float ) ( vectorMult[3] ) * 0.01f } ,

			//		{ 0 , 0 , 0 , -0.01f , -0.01f , 1 , -( float ) ( vectorMult[4] ) * -0.01f , -( float ) ( vectorMult[4] ) * -0.01f } ,
			//		{ 0 , 0 , 0 , 0.01f , -0.01f , 1 , -( float ) ( vectorMult[5] ) * 0.01f , -( float ) ( vectorMult[5] ) * -0.01f } ,
			//		{ 0 , 0 , 0 , 0.01f , 0.01f , 1 , -( float ) ( vectorMult[6] ) * 0.01f , -( float ) ( vectorMult[6] ) * 0.01f } ,
			//		{ 0 , 0 , 0 , -0.01f , 0.01f , 1 , -( float ) ( vectorMult[7] ) * -0.01f , -( float ) ( vectorMult[7] ) * 0.01f } ,
			//	};
			//	for ( unsigned int j = 0; j < 8; ++j )
			//	{
			//		for ( unsigned int i = 0; i < 8; ++i )
			//		{
			//			matrix[( j * 8 ) + i] = prematrix[j][i];
			//			//if ( ( j * 8 ) + i >= 8 * 8 ) std::cout << "went out" << std::endl;
			//		}
			//	}
			//}
			//
			////std::cout << quadResult.pt[0].x - 595.164f << std::endl;
			//float determinant = MathHelpers::Determinant( matrix , 8 );
			////std::cout << determinant << std::endl;
			//if ( determinant != 0 )
			//{
			//	MathHelpers::preAdj( matrix , 8 );
			//	MathHelpers::Transpose( matrix , 8 );
			//	MathHelpers::Multiply( matrix , 1.0f / determinant , 8 );
			//	MathHelpers::MultiplyVector( matrix , vectorMult , 8 );
			//	//std::cout << "{" << vectorMult[0] << "," << vectorMult[4] << "}" << std::endl;
			//	//std::cout << "{" << vectorMult[1] << "," << vectorMult[5] << "}" << std::endl;
			//	//std::cout << "{" << vectorMult[2] << "," << vectorMult[6] << "}" << std::endl;
			//	//std::cout << "{" << vectorMult[3] << "," << vectorMult[7] << "}" << std::endl;

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
			//	glm::vec4 row0( -vectorMult[0] * translateVector.z , -vectorMult[2] * translateVector.z , vectorMult[4] * translateVector.z, translateVector.x );
			//	glm::vec4 row1( -vectorMult[1] * translateVector.z , -vectorMult[3] * translateVector.z , vectorMult[5] * translateVector.z,translateVector.y );
			//	glm::vec4 row2 = glm::vec4( glm::cross( glm::vec3( row0 ) , glm::vec3( row1 ) ) , translateVector.z );
			//	glm::mat4 theResultingMatrix( row0, row1 , row2 , glm::vec4(0,0,0,1) );
			//	quadResult.transform = glm::transpose( theResultingMatrix );
			//	glm::vec4 translated = glm::transpose( theResultingMatrix ) * glm::vec4( 0 , 0 , 0 , 1 );
			//	//translated /= translated.w;
			//	//std::cout << "{" << translateVector.x << "," << translateVector.y << "," << translateVector.z << "}" << std::endl;
			//}
			//delete[] matrix;
			//delete[] vectorMult;


			quadResults.push_back( quadResult );
			//debug
			//{
			//	while ( numUsing ) std::cout << numUsing << std::endl;
			//	canGrab = false;
			//	unsigned short randomColor = rand() % 255;
			//	unsigned short randomColor2 = rand() % 255;
			//	unsigned short randomColor3 = rand() % 255;
			//	for ( unsigned int i = 0; i < 4; ++i )
			//	{
			//		
			//		glm::vec2* start = &quadResult.pt[i];
			//		glm::vec2* end = 0;
			//		if ( i >= 3 )
			//		{
			//			end = &quadResult.pt[0];
			//		}
			//		else
			//		{
			//			end = &quadResult.pt[i + 1];
			//		}
			//		glm::vec2 normalized = glm::normalize( glm::vec2( *end ) - glm::vec2( *start ) );
			//		glm::vec2 testPixel = glm::vec2( *start);
			//		glm::ivec2 lastPixel = glm::ivec2(*start);
			//		unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
			//		//std::cout << lastPixel.y << std::endl;
			//		//std::cout << lastPixel.x << std::endl;
			//		if ( lastPixel.x >= 0 && lastPixel.x < width && lastPixel.y >= 0 && lastPixel.y < height )
			//		{
			//			copiedPictureInstance[iOffset] = ( unsigned char ) randomColor;
			//			copiedPictureInstance[iOffset + 1] = ( unsigned char ) randomColor2;
			//			copiedPictureInstance[iOffset + 2] = ( unsigned char ) randomColor3;
			//		}
			//		while ( glm::ivec2( testPixel ) == lastPixel )
			//		{
			//			testPixel += normalized;
			//		}
			//		lastPixel = glm::ivec2( testPixel );
			//		while ( glm::length( testPixel - glm::vec2( *start ) ) <= glm::length( glm::vec2( *end ) - glm::vec2( *start ) ) && lastPixel.x >= 0 && lastPixel.x < width && lastPixel.y >= 0 && lastPixel.y < height )
			//		{
			//			unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
			//			copiedPictureInstance[iOffset] = ( unsigned char ) randomColor;
			//			copiedPictureInstance[iOffset + 1] = ( unsigned char ) randomColor2;
			//			copiedPictureInstance[iOffset + 2] = ( unsigned char ) randomColor3;
			//			while ( glm::ivec2( testPixel ) == lastPixel )
			//			{
			//				testPixel += normalized;
			//			}
			//			lastPixel = glm::ivec2( testPixel );
			//		}
			//	}
			//	canGrab = true;
			//}

			for ( unsigned int i = 0; i < 4; ++i )
			{
				quad.line[i]->deletion = true;
			}

			bool cleaned = false;
			while ( !cleaned )
			{
				int start = -1 , end = -1;
				for ( unsigned int i = 0; i < theLines.size(); ++i )
				{
					if ( theLines[i].deletion && start < 0 )
					{
						start = ( int ) i;
					}
					else if ( start >= 0 && !theLines[i].deletion )
					{
						end = ( int ) i - 1;
						break;
					}
				}
				if ( start < 0 ) cleaned = true;
				else if ( end < 0 ) theLines.erase( theLines.begin() + start , theLines.end() );
				else if ( start == end ) theLines.erase( theLines.begin() + start );
				else theLines.erase( theLines.begin() + start , theLines.begin() + end );
			}
		}
	}

	canGrabMarkerFound = false;
	while ( numUsingMarkerFound );
	toSend.clear();
	for ( unsigned int i = 0; i < quadResults.size(); ++i )
	{
		glm::vec2 center;
		for ( unsigned int j = 0; j < 4; ++j )
		{
			center += quadResults[i].pt[j];
		}
		center /= 4;
		center.x = ( center.x / width);
		center.x = ((center.x) * 2) - 1;
		center.y = ( center.y / height);
		center.y = ((center.y) * 2) - 1;
		//std::cout << center.y << std::endl;

		FoundMarkerInfo found;
		found.transform = quadResults[i].transform;
		found.center = center;
		found.cardIndex = quadResults[i].markerID;

		toSend.push_back( found );
	}
	canGrabMarkerFound = true;

	//std::cout << "num lines " << theLines.size() << std::endl;
	//std::cout << "num quads " << quadResults.size() << std::endl;
	//std::cout << "full algoTime " << c.Stop() << std::endl;
	//std::cin.get();
}

bool ARMarkerDetector::findQuad( ConstructingQuad& quadToEdit , std::vector<Line>& lines , unsigned int index )
{
	if ( index > 1 )
	{
		//std::cout << "highindex " << index << std::endl;
	}
	float angleThreshold = 5;
	float threshold = ((float)(width + height) / 2.0f) * 0.015f;
	//std::cout << threshold << std::endl;
	if ( index == 4 )
	{
		float length = glm::length( glm::vec2(quadToEdit.line[3]->end) - glm::vec2(quadToEdit.line[0]->start) );
		//std::cout << (length ) << std::endl;
		float lineToCompareAngle = quadToEdit.line[3]->angle + 90;
		if ( lineToCompareAngle >= 360 ) lineToCompareAngle -= 360;
		if ( lineToCompareAngle < 0 ) lineToCompareAngle += 360;
		bool found = length <= threshold;//&& abs( ( lineToCompareAngle ) -quadToEdit.line[0]->angle ) <= angleThreshold;
		// guess 3 corners
		if ( !found && abs( ( lineToCompareAngle ) -quadToEdit.line[0]->angle ) <= angleThreshold )
		{
			glm::vec2 normal = glm::normalize( glm::vec2( quadToEdit.line[3]->end - quadToEdit.line[3]->start ) );
			glm::vec2 lineToDot = glm::vec2( quadToEdit.line[0]->start - quadToEdit.line[3]->start );
			float theDot = glm::dot( lineToDot , normal );
			quadToEdit.line[3]->end = glm::ivec2( theDot * normal ) + quadToEdit.line[3]->start;

			normal = glm::normalize( glm::vec2( quadToEdit.line[0]->start - quadToEdit.line[0]->end ) );
			lineToDot = glm::vec2( quadToEdit.line[3]->end - quadToEdit.line[0]->end );
			theDot = glm::dot( lineToDot , normal );
			quadToEdit.line[0]->start = glm::ivec2( theDot * normal ) + quadToEdit.line[0]->end;
			//quadToEdit.line[0]->start = quadToEdit.line[3]->end;
			return true;
		}
		return found;
	}
	bool found = false;
	for ( unsigned int i = 0; i < lines.size() && !found; ++i )
	{
		quadToEdit.line[index] = 0;
		bool inLine = false;
		for ( unsigned int j = 0; j < 4 && !inLine; ++j )
		{
			inLine = &lines[i] == quadToEdit.line[j];
		}
		if ( inLine ) continue;

		Line* lineToCompare = 0;
		if ( index > 0 ) lineToCompare = quadToEdit.line[index - 1];

		if ( !lineToCompare )
		{
			quadToEdit.line[index] = &lines[i];
			found = findQuad( quadToEdit , lines , index + 1 );
		}
		else
		{
			Line* currentLine = &lines[i];
			if ( glm::length( glm::vec2( currentLine->start ) - glm::vec2( lineToCompare->end ) ) > threshold ) continue;
			//std::cout << "trueangle " << trueAngle << std::endl;
			//std::cout << "trueangle2 " << trueAngle2 << std::endl;
			//std::cout << glm::length( glm::vec2( currentLine->start ) - glm::vec2( lineToCompare->end ) ) << std::endl;
			//std::cout << ( lineToCompare->angle + 90 ) - (currentLine->angle) << std::endl; 
			//std::cout << lineToCompare->angle << std::endl;
			//std::cout << currentLine->angle << std::endl;
			float lineToCompareAngle = lineToCompare->angle + 90;
			if ( lineToCompareAngle >= 360 ) lineToCompareAngle -= 360;
			if ( lineToCompareAngle < 0 ) lineToCompareAngle += 360;
			if ( abs( ( lineToCompareAngle ) - currentLine->angle ) > angleThreshold )
			{
				//std::cout << ( lineToCompareAngle ) << std::endl;
				continue;
			}

			quadToEdit.line[index] = currentLine;
			found = findQuad( quadToEdit , lines , index + 1 );
		}
	}
	if ( !found ) quadToEdit.line[index] = 0;
	//std::cout << index << std::endl;
	return found;
}

void ARMarkerDetector::findLines( std::vector<Line>& linesToAdd )
{
	std::vector<std::future<std::vector<Line>>> threads;
	for ( long y = 0; y < height; y += 20 )
	{
		for ( long x = 0; x < width; x += 20 )
		{
			threads.push_back( std::async( std::launch::async , &ARMarkerDetector::findLinesOnRegion , this , x , y , 20 , 20 ) );
		}
	}
	for ( unsigned int i = 0; i < threads.size(); ++i )
	{
		threads[i].wait();
		std::vector<Line> theLines = threads[i].get();
		for ( unsigned int j = 0; j < theLines.size(); ++j )
		{
			linesToAdd.push_back( theLines[j] );
			//if(abs(theLines[j].angle) >= 100) std::cout << (int)theLines[j].angle << std::endl;
		}
		//std::cout << "num lines " << linesToAdd.size() << std::endl;
	}

	////debug
	//{
	//	while ( numUsing ) std::cout << numUsing << std::endl;
	//	canGrab = false;

	//	for ( unsigned int i = 0; i < linesToAdd.size(); ++i )
	//	{
	//		glm::vec2 normalized = glm::normalize( glm::vec2( linesToAdd[i].end ) - glm::vec2( linesToAdd[i].start ) );
	//		bool extendable = true;
	//		glm::vec2 testPixel = glm::vec2( linesToAdd[i].start );
	//		glm::ivec2 lastPixel = linesToAdd[i].start;
	//		//unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
	//		//std::cout << lastPixel.y << std::endl;
	//		//std::cout << lastPixel.x << std::endl;
	//		//copiedPictureInstance[iOffset] = ( unsigned char ) 255;
	//		//copiedPictureInstance[iOffset+1] = ( unsigned char ) 0;
	//		//copiedPictureInstance[iOffset+2] = ( unsigned char ) 0;
	//		while ( glm::ivec2( testPixel ) == lastPixel )
	//		{
	//			testPixel += normalized;
	//		}
	//		lastPixel = glm::ivec2( testPixel );
	//		while ( extendable && glm::length( testPixel - glm::vec2( linesToAdd[i].start ) ) <= glm::length( glm::vec2( linesToAdd[i].end ) - glm::vec2( linesToAdd[i].start ) ) && lastPixel.x >= 0 && lastPixel.x < width && lastPixel.y >= 0 && lastPixel.y < height )
	//		{
	//			unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
	//			copiedPictureInstance[iOffset] = ( unsigned char ) 255;
	//			copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
	//			copiedPictureInstance[iOffset + 2] = ( unsigned char ) 0;
	//			while ( glm::ivec2( testPixel ) == lastPixel )
	//			{
	//				testPixel += normalized;
	//			}
	//			lastPixel = glm::ivec2( testPixel );
	//		}
	//	}
	//	canGrab = true;
	//}
}

std::vector<Line> ARMarkerDetector::findLinesOnRegion( long x , long y , long width , long height )
{
	std::vector<Line> lines;

	std::vector<EdgelInList> edgels;

	float upperThreshold = 100.0f;

	for ( long row = y; row < ( y + height )&& row < this->height; ++row )
	{
		for ( long col = x; col < ( x + width ) && col < this->width; ++col )
		{
			unsigned long i = ( unsigned long ) ( ( row * this->width) + ( col ) );
			float gradientIntensity = this->gradientIntensity[i];
			if ( gradientIntensity < upperThreshold ) continue;
			//std::cout << *( gradientDirection + i ) << std::endl;
			EdgelInList edgel;
			edgel.edgel.pos = glm::vec2( col , row );
			edgel.edgel.gradientIntensity = gradientIntensity;
			edgel.edgel.angle = this->gradientDirection[i];
			edgels.push_back( edgel );
		}
	}

	////debug
	//{
	//	while ( numUsing ) std::cout << numUsing << std::endl;
	//	canGrab = false;

	//	for ( unsigned int i = 0; i < edgels.size(); ++i )
	//	{
	//		//if ( edgels[i].edgel.angle > 215 && edgels[i].edgel.angle < 300 )
	//		//{
	//			unsigned long iOffset = ( unsigned long ) ( ( edgels[i].edgel.pos.y * 4 * this->width ) + ( edgels[i].edgel.pos.x * 4 ) );
	//			copiedPictureInstance[iOffset] = ( unsigned char ) 255;
	//			copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
	//			copiedPictureInstance[iOffset + 2] = ( unsigned char ) 0;
	//		//}
	//		//std::cout << edgels[i].edgel.pos.y << std::endl;
	//		//std::cout << edgels[i].edgel.pos.x << std::endl;

	//	}
	//	canGrab = true;
	//}

	float angleThreshold = 15.0f;
	float maxDistance = ((float)(width + height) / 2.0f) * 3.0f;
	unsigned int maxIterations = (unsigned int)(2 * (height * width));
	unsigned int minNumEdgels = 5;

	std::vector<Line> tempLines;

	for ( unsigned int currentIteration = 0; edgels.size() >= minNumEdgels && currentIteration < maxIterations; ++currentIteration )
	{
		int maxSubIteration = ( unsigned int ) ( 2 * ( height * width ));
		bool found = false;
		EdgelInList* e1 = 0 , *e2 = 0;
		for ( int currentSubIteration = 0; currentSubIteration < maxSubIteration && !found; ++currentSubIteration )
		{
			e1 = &edgels[( rand() % edgels.size() )];
			e2 = &edgels[( rand() % edgels.size() )];
			if ( e1 == e2 ) continue;

			float angleTest = e1->edgel.angle - e2->edgel.angle;
			found = abs( angleTest ) <= angleThreshold;
		}
		if ( !found ) continue;

		glm::vec2 theSlope = glm::normalize( glm::vec2(e2->edgel.pos) - glm::vec2(e1->edgel.pos) );
		float slopeAngle = ( atan2f( -theSlope.y , theSlope.x ) * 180 ) / ( float ) M_PI;
		float oppSlopeAngle = ( atan2f( theSlope.y , -theSlope.x ) * 180 ) / ( float ) M_PI;
		if ( oppSlopeAngle < 0 ) oppSlopeAngle += 360;
		if ( oppSlopeAngle >= 360 ) oppSlopeAngle -= 360;
		if ( slopeAngle < 0 ) slopeAngle += 360;
		if ( slopeAngle >= 360 ) slopeAngle -= 360;

		float edgelAngleAverage = ( e1->edgel.angle + e2->edgel.angle ) / 2;

		if ( abs( oppSlopeAngle - ( edgelAngleAverage ) ) < abs( slopeAngle - ( edgelAngleAverage ) ) )
		{
			EdgelInList* temp = e2;
			e2 = e1;
			e1 = temp;
			slopeAngle = oppSlopeAngle;
		}

		//copy of slopes, must optimize later
		std::vector<EdgelInList*> supportedEdgels;
		glm::ivec2 endAtStartSpace = e2->edgel.pos - e1->edgel.pos;
		float lengthOfLine = glm::length( glm::vec2(endAtStartSpace) );
		glm::vec2 lineNormal = glm::normalize( glm::vec2(endAtStartSpace));
		for ( unsigned int i = 0; i < edgels.size(); ++i )
		{
			EdgelInList* test = &edgels[i];
			if ( test == e1 || test == e2 ) continue;
			if ( ( abs( (test->edgel.angle) - slopeAngle ) > angleThreshold ) ) continue;
			glm::ivec2 testAtStartSpace = test->edgel.pos - e1->edgel.pos;
			float locationInLineNormal = glm::dot(glm::vec2(testAtStartSpace), lineNormal);
			//std::cout << locationInLineNormal << std::endl;
			if ( locationInLineNormal < 0 || locationInLineNormal > lengthOfLine )continue;
			//std::cout << locationInLineNormal << std::endl;
			float distanceFromLine = glm::length( glm::vec2(testAtStartSpace) - ( locationInLineNormal * lineNormal ) );
			if ( distanceFromLine > maxDistance ) continue;
			//std::cout << distanceFromLine << std::endl;
			supportedEdgels.push_back( test );
		}
		//debug
		//{
		//	while ( numUsing ) std::cout << numUsing << std::endl;
		//	canGrab = false;

		//	for ( unsigned int i = 0; i < supportedEdgels.size(); ++i )
		//	{
		//		//if ( supportedEdgels[i]->edgel.angle > 300 || supportedEdgels[i]->edgel.angle < 45 )
		//		//{
		//			//std::cout << supportedEdgels[i]->edgel.pos.y << std::endl;
		//			//std::cout << supportedEdgels[i]->edgel.pos.x << std::endl;
		//			unsigned long iOffset = ( unsigned long ) ( ( supportedEdgels[i]->edgel.pos.y * 4 * this->width ) + ( supportedEdgels[i]->edgel.pos.x * 4 ) );
		//			copiedPictureInstance[iOffset] = ( unsigned char ) 255;
		//			copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
		//			copiedPictureInstance[iOffset + 2] = ( unsigned char ) 0;
		//		//}
		//	}
		//	canGrab = true;
		//}
		if ( supportedEdgels.size() < minNumEdgels ) continue;
		
		Line line;
		line.start = e1->edgel.pos;
		line.end = e2->edgel.pos;
		line.angle = slopeAngle;

		////debug
		//{
		//	while ( numUsing ) std::cout << numUsing << std::endl;
		//	canGrab = false;
		//	//if ( line.angle > 215 && line.angle < 300 )
		//	//{
		//	unsigned short randColor = rand() % 255;
		//	unsigned short randColor2 = rand() % 255;
		//	unsigned short randColor3 = rand() % 255;
		//		glm::vec2 normalized = glm::normalize( glm::vec2( line.end ) - glm::vec2( line.start ) );
		//		glm::vec2 testPixel = glm::vec2( line.start );
		//		glm::ivec2 lastPixel = line.start;
		//		unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
		//		std::cout << lastPixel.y << std::endl;
		//		std::cout << lastPixel.x << std::endl;
		//		copiedPictureInstance[iOffset] = ( unsigned char ) 0;
		//		copiedPictureInstance[iOffset + 1] = ( unsigned char ) 255;
		//		copiedPictureInstance[iOffset + 2] = ( unsigned char ) 0;
		//		while ( glm::ivec2( testPixel ) == lastPixel )
		//		{
		//			testPixel += normalized;
		//		}
		//		lastPixel = glm::ivec2( testPixel );
		//		while ( glm::length( testPixel - glm::vec2( line.start ) ) < glm::length( glm::vec2( line.end ) - glm::vec2( line.start ) ) && lastPixel.x >= 0 && lastPixel.x < this->width && lastPixel.y >= 0 && lastPixel.y < this->height )
		//		{
		//			unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
		//			if ( copiedPictureInstance[iOffset + 1] = !255 && copiedPictureInstance[iOffset + 2] != 255 )
		//			{
		//				copiedPictureInstance[iOffset] = ( unsigned char ) 255;
		//				copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
		//				copiedPictureInstance[iOffset + 2] = ( unsigned char ) 0;
		//			}
		//			
		//			while ( glm::ivec2( testPixel ) == lastPixel )
		//			{
		//				testPixel += normalized;
		//			}
		//			lastPixel = glm::ivec2( testPixel );
		//		}
		//		iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
		//		copiedPictureInstance[iOffset] = ( unsigned char ) 0;
		//		copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
		//		copiedPictureInstance[iOffset + 2] = ( unsigned char ) 255;
		//	//}
		//	canGrab = true;
		//}


		//if(slopeAngle < 0) std::cout << slopeAngle << std::endl;
		//make better deletion
		for (unsigned int i = 0; i < supportedEdgels.size(); ++i )
		{
			supportedEdgels[i]->deletion = true;
		}
		bool cleaned = false;
		while ( !cleaned )
		{
			int start = -1 , end = -1;
			for ( unsigned int i = 0; i < edgels.size(); ++i )
			{
				if ( edgels[i].deletion && start < 0 )
				{
					start = (int)i;
				}
				else if ( start >= 0 && !edgels[i].deletion )
				{
					end = (int)i - 1;
					break;
				}
			}
			if ( start < 0 ) cleaned = true;
			else if ( end < 0 && edgels.begin() + start != edgels.end() ) edgels.erase( edgels.begin() + start , edgels.end() );
			else if ( start == end || (end < 0 && edgels.begin() + start == edgels.end()) ) edgels.erase( edgels.begin() + start );
			else edgels.erase( edgels.begin() + start , edgels.begin() + end );
		}
		tempLines.push_back( line );
		//std::cout << glm::length( glm::vec2( line.start ) - glm::vec2( line.end ) ) << std::endl;

		////debug
		//{
		//	while ( numUsing ) std::cout << numUsing << std::endl;
		//	canGrab = false;

		//	for ( unsigned int i = 0; i < tempLines.size(); ++i )
		//	{
		//		glm::vec2 normalized = glm::normalize( glm::vec2( line.end ) - glm::vec2( line.start ) );
		//		bool extendable = true;
		//		glm::vec2 testPixel = glm::vec2( line.start );
		//		glm::ivec2 lastPixel = line.start;
		//		unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
		//		//std::cout << lastPixel.y << std::endl;
		//		//std::cout << lastPixel.x << std::endl;
		//		copiedPictureInstance[iOffset] = ( unsigned char ) 255;
		//		copiedPictureInstance[iOffset+1] = ( unsigned char ) 0;
		//		copiedPictureInstance[iOffset+2] = ( unsigned char ) 0;
		//		while ( glm::ivec2( testPixel ) == lastPixel )
		//		{
		//			testPixel += normalized;
		//		}
		//		lastPixel = glm::ivec2( testPixel );
		//		while ( extendable && glm::length( testPixel - glm::vec2( line.start ) ) <= glm::length( glm::vec2( line.end ) - glm::vec2( line.start ) ) && lastPixel.x >= 0 && lastPixel.x < width && lastPixel.y >= 0 && lastPixel.y < height )
		//		{
		//			unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
		//			copiedPictureInstance[iOffset] = ( unsigned char ) 255;
		//			copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
		//			copiedPictureInstance[iOffset + 2] = ( unsigned char ) 0;
		//			while ( glm::ivec2( testPixel ) == lastPixel )
		//			{
		//				testPixel += normalized;
		//			}
		//			lastPixel = glm::ivec2( testPixel );
		//		}
		//	}
		//	canGrab = true;
		//}
	}

	for ( unsigned int i = 0; i < tempLines.size(); ++i )
	{
		Line* lineToConnect = &tempLines[i];
		for ( unsigned int j = i + 1; j < tempLines.size(); ++j )
		{
			Line* lineEnd = &tempLines[j];
			if ( abs( lineEnd->angle - lineToConnect->angle ) > angleThreshold ) continue;
			Line* lineStart = lineToConnect;
			if ( glm::length( glm::vec2(lineEnd->end) - glm::vec2(lineStart->start) ) < glm::length( glm::vec2(lineStart->end) - glm::vec2(lineEnd->start) ) )
			{
				Line* tempLine = lineEnd;
				lineEnd = lineStart;
				lineStart = tempLine;
			}

			glm::vec2 connectingLineTest = glm::vec2(lineEnd->start) - glm::vec2(lineStart->end);
			glm::vec2 normalizedConnectingLineTest = glm::normalize( connectingLineTest );
			float connectingLineAngleTest = ( atan2f( -connectingLineTest.y , connectingLineTest.x ) * 180 ) / ( float ) M_PI;
			if ( connectingLineAngleTest < 0 ) connectingLineAngleTest += 360;
			float angleAverage = ( lineStart->angle + lineEnd->angle ) / 2;
			if ( abs( connectingLineAngleTest - angleAverage ) > angleThreshold ) continue;
			
			//must check if disconnecting later
			glm::vec2 currentPosToCheck = glm::vec2(lineStart->end);
			glm::ivec2 lastPosToCheck = lineStart->end;
			bool compatible = true;
			while ( glm::length( glm::vec2( lastPosToCheck ) - glm::vec2( lineStart->end ) ) < glm::length( glm::vec2( connectingLineTest ) ) && compatible )
			{
				currentPosToCheck += normalizedConnectingLineTest;
				if ( glm::ivec2( currentPosToCheck ) == lastPosToCheck ) continue;
				lastPosToCheck = glm::ivec2( currentPosToCheck );
				unsigned long imageIndex = ( unsigned long ) ( ( lastPosToCheck.y  * this->width) + ( lastPosToCheck.x ) );
				compatible = ( gradientIntensity[imageIndex] > upperThreshold ) && ( abs( ( gradientDirection[imageIndex] ) - angleAverage ) <= angleThreshold );
			}
			if ( !compatible ) continue;
			glm::vec2 newLineAngleVec = glm::vec2(lineEnd->end) - glm::vec2(lineStart->start);
			float newLineAngle = (atan2f( -newLineAngleVec.y , newLineAngleVec.x ) * 180 ) / ( float ) M_PI;
			if ( newLineAngle < 0 ) newLineAngle += 360;
			lineToConnect->start = lineStart->start;
			lineToConnect->end = lineEnd->end;
			lineToConnect->angle = newLineAngle;

			unsigned int lineToRemove = lineEnd - &tempLines[0];
			if ( lineEnd == lineToConnect ) lineToRemove = lineStart -&tempLines[0];
			tempLines.erase( tempLines.begin() + lineToRemove);
			--i;
			break;
		}
	}
	//if ( tempLines.size() ) std::cout << "after " << tempLines.size() << std::endl;

	////debug
	//{
	//	while ( numUsing ) std::cout << numUsing << std::endl;
	//	canGrab = false;

	//	for ( unsigned int i = 0; i < tempLines.size(); ++i )
	//	{
	//		unsigned short randomColor = rand() % 255;
	//		unsigned short randomColor2 = rand() % 255;
	//		unsigned short randomColor3 = rand() % 255;
	//		glm::vec2 normalized = glm::normalize( glm::vec2( tempLines[i].end ) - glm::vec2( tempLines[i].start ) );
	//		glm::vec2 testPixel = glm::vec2( tempLines[i].start );
	//		glm::ivec2 lastPixel = tempLines[i].start;
	//		unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
	//		//std::cout << lastPixel.y << std::endl;
	//		//std::cout << lastPixel.x << std::endl;
	//		copiedPictureInstance[iOffset] = ( unsigned char ) 0;
	//		copiedPictureInstance[iOffset + 1] = ( unsigned char ) 255;
	//		copiedPictureInstance[iOffset + 2] = ( unsigned char) 0;
	//		while ( glm::ivec2( testPixel ) == lastPixel )
	//		{
	//			testPixel += normalized;
	//		}
	//		lastPixel = glm::ivec2( testPixel );
	//		while ( glm::length( testPixel - glm::vec2( tempLines[i].start ) ) < glm::length( glm::vec2( tempLines[i].end ) - glm::vec2( tempLines[i].start ) ) && lastPixel.x >= 0 && lastPixel.x < this->width && lastPixel.y >= 0 && lastPixel.y < this->height )
	//		{
	//			unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
	//			if ( copiedPictureInstance[iOffset + 1] != 255 && copiedPictureInstance[iOffset + 2] != 255 )
	//			{
	//				
	//				copiedPictureInstance[iOffset] = ( unsigned char ) 255;
	//				copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
	//				copiedPictureInstance[iOffset + 2] = ( unsigned char ) 0;
	//			}
	//			while ( glm::ivec2( testPixel ) == lastPixel )
	//			{
	//				testPixel += normalized;
	//			}
	//			lastPixel = glm::ivec2( testPixel );
	//		}
	//		iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
	//		copiedPictureInstance[iOffset] = ( unsigned char ) 0;
	//		copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
	//		copiedPictureInstance[iOffset + 2] = ( unsigned char ) 255;
	//	}
	//	canGrab = true;
	//}
	return tempLines;
}