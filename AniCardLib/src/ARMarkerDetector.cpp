#include <ARMarkerDetector.h>
#include <PictureFetcher.h>
#include <glm.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <iostream>
#include <Clock.h>
#include <chrono>
ARMarkerDetector ARMarkerDetector::global;
using namespace AniCardLib;
void ARMarkerDetector::findCard( PictureFetcher* thePhoto )
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
	if ( this->width != width || this->height != height )
	{
		if ( copiedPictureInstance ) delete[] copiedPictureInstance;
		if ( grayscaleImage ) delete[] grayscaleImage;
		if ( gradientDirection ) delete[] gradientDirection;
		copiedPictureInstance = new unsigned char[width * height * 3];
		grayscaleImage = new unsigned char[width * height];
		gradientDirection = new float[width * height];
		this->width = width;
		this->height = height;
	}
	memcpy( copiedPictureInstance , bytes , sizeof( unsigned char ) * ( width * height * 3 ) );
	thePhoto->finishedUsing();
	runningThread = new std::future<void>(std::async( std::launch::async, &ARMarkerDetector::_findCard , this ));
}
void ARMarkerDetector::_findCard()
{
	std::vector<Line> theLines;
	Clock c;
	c.Start();
	findLines( theLines );
	float angleThreshold = 10;
	for ( unsigned int i = 0; i < theLines.size(); ++i )
	{
		Line* lineToConnect = &theLines[i];
		for ( unsigned int j = i + 1; j < theLines.size(); ++j )
		{
			Line* lineEnd = &theLines[j];
			if ( abs( lineEnd->angle - lineToConnect->angle ) > angleThreshold ) continue;
			Line* lineStart = lineToConnect;
			if ( glm::length( glm::vec2(lineEnd->end) - glm::vec2(lineStart->start )) < glm::length( glm::vec2(lineStart->end) - glm::vec2(lineEnd->start) ) )
			{
				Line* tempLine = lineEnd;
				lineEnd = lineStart;
				lineStart = tempLine;
			}

			glm::ivec2 connectingLineTest = lineEnd->start - lineStart->end;
			if ( ( atan2f( (float)connectingLineTest.y , (float)connectingLineTest.x ) * 180.0f ) / ( float ) M_PI > angleThreshold ) continue;

			//must check if disconnecting later
			glm::vec2 connectingLineTestNormal = glm::normalize( glm::vec2( connectingLineTest ) );
			glm::vec2 currentPosToCheck = glm::vec2( lineStart->end );
			glm::ivec2 lastPosToCheck = glm::ivec2( currentPosToCheck );
			bool compatible = true;
			while ( glm::length( glm::vec2(lineEnd->start) - glm::vec2(lastPosToCheck) ) < glm::length( glm::vec2(connectingLineTest) ) && compatible )
			{
				currentPosToCheck += connectingLineTestNormal;
				if ( glm::ivec2( currentPosToCheck ) == lastPosToCheck ) continue;
				lastPosToCheck = glm::ivec2( currentPosToCheck );
				unsigned long imageIndex = ( unsigned long ) ( ( lastPosToCheck.y ) * this->width + ( lastPosToCheck.x ) );
				compatible = gradientDirection[imageIndex] <= angleThreshold;
			}
			if ( !compatible ) continue;

			glm::ivec2 newLineAngleVec = lineEnd->end - lineStart->start;
			float newLineAngle = atan2f( (float)newLineAngleVec.y , (float)newLineAngleVec.x );

			lineToConnect->start = lineStart->start;
			lineToConnect->end = lineEnd->end;
			lineToConnect->angle = newLineAngle;

			Line* lineToRemove = lineStart;
			if ( lineEnd == lineToConnect ) lineToRemove = lineEnd;
			theLines.erase( theLines.begin() + ( lineToRemove - &theLines[0] ) );
			--i;
			break;
		}
	}

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
			if ( abs( gradientDirection[imageIndex] - theLines[i].angle ) < angleThreshold )
			{
				theLines[i].start = lastPixel;
			}
			else
			{
				extendable = false;
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
			unsigned long imageIndex = ( unsigned long ) ( ( lastPixel.y ) * this->width + ( lastPixel.x ) );
			if ( abs( gradientDirection[imageIndex] - theLines[i].angle ) < angleThreshold )
			{
				theLines[i].end = lastPixel;
			}
			else
			{
				extendable = false;
			}
			while ( glm::ivec2( testPixel ) == lastPixel )
			{
				testPixel += normalized;
			}
			lastPixel = glm::ivec2( testPixel );
		}
	}


	std::cout << "num lines " << theLines.size() << std::endl;
	std::cout << "full algoTime " << c.Stop() << std::endl;
}

void ARMarkerDetector::findLines( std::vector<Line>& linesToAdd )
{
	std::vector<std::future<std::vector<Line>>*> threads;
	for ( long y = 0; y < height; y += 10 )
	{
		for ( long x = 0; x < width; x += 10 )
		{
			threads.push_back( new std::future<std::vector<Line>>( std::async( std::launch::async , &ARMarkerDetector::findLinesOnRegion , this , x , y , 10 , 10 ) ) );
		}
	}
	for ( unsigned int i = 0; i < threads.size(); ++i )
	{
		threads[i]->wait();
		std::vector<Line> theLines = threads[i]->get();
		for ( unsigned int j = 0; j < theLines.size(); ++j )
		{
			linesToAdd.push_back( theLines[j] );
		}
		//std::cout << "num lines " << linesToAdd.size() << std::endl;
		delete threads[i];
	}
}

std::vector<Line> ARMarkerDetector::findLinesOnRegion( long x , long y , long width , long height )
{
	int GxMask[3][3] =
	{
		{ -1 , 0 , 1 } ,
		{ -2 , 0 , 2 } ,
		{ -1 , 0 , 1 }
	};
	int GyMask[3][3] =
	{
		{ 1 , 2 , 1 } ,
		{ 0 , 0 , 0 } ,
		{ -1 , -2 , -1 }
	};

	int gaussianMask[5][5] =
	{
		{ 2 , 4 , 5 , 4 , 2 } ,
		{ 4 , 9 , 12 , 9 , 4 } ,
		{ 5 , 12 , 15 , 12 , 2 } ,
		{ 4 , 9 , 12 , 9 , 4 } ,
		{ 2 , 4 , 5 , 4 , 2 }
	};

	//time_t thisTime = time( 0 );
	//gaussian blur
	for ( long row = y + 2; row < ( y + height ) - 2 && row < this->height - 2; ++row )
	{
		for ( long col = x + 2; col < ( x + width ) - 2 && col < this->width - 2; ++col )
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
					unsigned long iOffset = ( unsigned long ) ( rowTotal * 3 * this->width + colTotal * 3 );
					//do this for all color channels
					float grayPixel = ( ( float ) ( copiedPictureInstance + iOffset )[0] + ( float ) ( copiedPictureInstance + iOffset )[1] + ( float ) ( copiedPictureInstance + iOffset )[2] ) / 3.0f;
					newPixel += grayPixel * gaussianMask[2 + rowOffset][2 + colOffset];
				}
			}
			//real pixel location
			unsigned long i = ( unsigned long ) ( (row) * this->width + (col) );
			//apply blur
			*( grayscaleImage + i ) = (unsigned char)glm::clamp( newPixel / 159, 0.0f, (float)UCHAR_MAX);
		}
	}
	//std::cout << "gaussian blur " << ( time( 0 ) - thisTime ) << std::endl;

	std::vector<Line> lines;

	std::vector<EdgelInList> edgels;

	float upperThreshold = 30.0f;

	for ( long row = y + 1; row < ( y + height ) - 1 && row < this->height - 1; ++row )
	{
		for ( long col = x + 1; col < ( x + width ) - 1 && col < this->width - 1; ++col )
		{
			float Gx = 0, Gy = 0;

			for ( long rowOffset = -1; rowOffset <= 1; ++rowOffset )
			{
				for ( long colOffset = -1; colOffset <= 1; ++colOffset )
				{
					long rowTotal = row + rowOffset;
					long colTotal = col + colOffset;
					unsigned long iOffset = ( unsigned long ) ( rowTotal* this->width + colTotal );

					Gx += grayscaleImage[iOffset] * GxMask[rowOffset + 1][colOffset + 1];
					Gy += grayscaleImage[iOffset] * GyMask[rowOffset + 1][colOffset + 1];
				}
			}
			unsigned long i = ( unsigned long ) ( ( row ) * this->width + ( col ) );
			*( gradientDirection + i ) = ( atan2f( Gy , Gx ) * 180 ) / ( float ) M_PI;
			float gradientIntensity = sqrt( ( Gx * Gx ) + ( Gy * Gy ) );
			//std::cout << gradientIntensity << std::endl;
			if ( gradientIntensity < upperThreshold ) continue;
			EdgelInList edgel;
			edgel.edgel.pos = glm::vec2( col + x , row + y );
			edgel.edgel.gradientIntensity = gradientIntensity;
			edgels.push_back( edgel );
		}
	}

	float angleThreshold = 10;
	float maxDistance = 4;
	unsigned int maxIterations = (unsigned int)(0.75f * (height * width));
	unsigned int minNumEdgels = 3;

	std::vector<Line> tempLines;

	for ( unsigned int currentIteration = 0; edgels.size() >= minNumEdgels && currentIteration < maxIterations; ++currentIteration )
	{
		srand( (unsigned int)time( 0 ) );

		
		int maxSubIteration = edgels.size();
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

		glm::vec2 theSlope = glm::normalize( glm::vec2(e1->edgel.pos) - glm::vec2(e2->edgel.pos) );
		float slopeAngle = ( atan2f( theSlope.y , theSlope.x ) * 180 ) / (float)M_PI , oppSlopeAngle = (atan2f( (-theSlope).y , -(theSlope).x ) * 180) / (float)M_PI;
		if ( abs( oppSlopeAngle - e1->edgel.angle ) < abs( slopeAngle - e1->edgel.angle ) )
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
			if ( ( abs( test->edgel.angle - slopeAngle ) > angleThreshold ) ) continue;
			glm::ivec2 testAtStartSpace = test->edgel.pos - e1->edgel.pos;
			float locationInLineNormal = glm::dot(glm::vec2(testAtStartSpace), lineNormal);
			if ( locationInLineNormal < 0 || locationInLineNormal > lengthOfLine )continue;
			float distanceFromLine = glm::length( glm::vec2(testAtStartSpace) - ( locationInLineNormal * lineNormal ) );
			if ( distanceFromLine > maxDistance ) continue;
			supportedEdgels.push_back( test );
		}

		if ( supportedEdgels.size() < minNumEdgels ) continue;
		
		Line line;
		line.start = e1->edgel.pos;
		line.end = e2->edgel.pos;
		line.angle = slopeAngle;
		
		//make better deletion
		for (unsigned int i = 0; i < supportedEdgels.size(); ++i )
		{
			supportedEdgels[i]->deletion = true;
			e1->deletion = true;
			e2->deletion = true;
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
			else if ( end < 0 ) edgels.erase( edgels.begin() + start , edgels.end() );
			else if(start == end) edgels.erase( edgels.begin() + start);
			else edgels.erase( edgels.begin() + start , edgels.begin() + end );
		}
		tempLines.push_back( line );
	}

	//if(tempLines.size()) std::cout << "before " << tempLines.size() << std::endl;
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

			glm::ivec2 connectingLineTest = lineEnd->start - lineStart->end;
			if ( ( atan2f( (float)connectingLineTest.y , (float)connectingLineTest.x ) * 180 ) / ( float ) M_PI > angleThreshold ) continue;
			
			//must check if disconnecting later
			glm::vec2 connectingLineTestNormal = glm::normalize( glm::vec2(connectingLineTest) );
			glm::vec2 currentPosToCheck = glm::vec2(lineStart->end);
			glm::ivec2 lastPosToCheck = glm::ivec2(currentPosToCheck);
			bool compatible = true;
			while ( glm::length( glm::vec2(lineEnd->start) - glm::vec2(lastPosToCheck) ) < glm::length( glm::vec2(connectingLineTest) ) && compatible )
			{
				currentPosToCheck += connectingLineTestNormal;
				if ( glm::ivec2( currentPosToCheck ) == lastPosToCheck ) continue;
				lastPosToCheck = glm::ivec2( currentPosToCheck );
				unsigned long imageIndex = ( unsigned long ) ( ( lastPosToCheck.y ) * this->width + ( lastPosToCheck.x ) );
				compatible = gradientDirection[imageIndex] <= angleThreshold;
			}
			if ( !compatible ) continue;
			glm::ivec2 newLineAngleVec = lineEnd->end - lineStart->start;
			float newLineAngle = atan2f( (float)newLineAngleVec.y , (float)newLineAngleVec.x );

			lineToConnect->start = lineStart->start;
			lineToConnect->end = lineEnd->end;
			lineToConnect->angle = newLineAngle;

			Line* lineToRemove = lineStart;
			if ( lineEnd == lineToConnect ) lineToRemove = lineEnd;
			tempLines.erase( tempLines.begin() + ( lineToRemove - &tempLines[0] ) );
			--i;
			break;
		}
	}
	//if ( tempLines.size() ) std::cout << "after " << tempLines.size() << std::endl;
	return tempLines;
}