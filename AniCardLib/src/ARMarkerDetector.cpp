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
	if ( numUsing > 0 ) { --numUsing; if ( numUsing < 0 ) std::cout << numUsing << std::endl; return true; }
	else return false;
}

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

	runningThread = new std::future<void>( std::async( std::launch::async , &ARMarkerDetector::_findCard , this ) );
}
void ARMarkerDetector::_findCard( )
{
	std::vector<Line> theLines;
	Clock c;
	c.Start();
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
			gradientDirection[i] = ( atan2f( Gy , Gx ) * 180 ) / ( float ) M_PI;
			if ( gradientDirection[i] < 0 ) gradientDirection[i] += 360;
			//std::cout << gradientDirection[i] << std::endl;
			gradientIntensity[i] = sqrt( ( Gx * Gx ) + ( Gy * Gy ) );
		}
	}
	findLines( theLines );
	float angleThreshold = 15;
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

			glm::ivec2 connectingLineTest = lineEnd->start - lineStart->end;
			float connectingLineAngleTest = ( atan2f( ( float ) -connectingLineTest.y , ( float ) connectingLineTest.x ) * 180 ) / ( float ) M_PI;
			if ( connectingLineAngleTest < 0 ) connectingLineAngleTest += 360;
			if ( abs( connectingLineAngleTest - lineStart->angle ) > angleThreshold || abs( connectingLineAngleTest - lineEnd->angle ) > angleThreshold ) continue;

			//must check if disconnecting later
			glm::vec2 connectingLineTestNormal = glm::normalize( glm::vec2( connectingLineTest ) );
			glm::vec2 currentPosToCheck = glm::vec2( lineStart->end );
			glm::ivec2 lastPosToCheck = glm::ivec2( currentPosToCheck );
			bool compatible = true;
			while ( glm::length( glm::vec2( lineEnd->start ) - glm::vec2( lastPosToCheck ) ) < glm::length( glm::vec2( connectingLineTest ) ) && compatible )
			{
				currentPosToCheck += connectingLineTestNormal;
				if ( glm::ivec2( currentPosToCheck ) == lastPosToCheck ) continue;
				lastPosToCheck = glm::ivec2( currentPosToCheck );
				unsigned long imageIndex = ( unsigned long ) ( ( lastPosToCheck.y ) * this->width + ( lastPosToCheck.x ) );
				compatible = ( abs( gradientDirection[imageIndex] - lineStart->angle ) <= angleThreshold || abs( gradientDirection[imageIndex] - lineEnd->angle ) <= angleThreshold );
			}
			if ( !compatible ) continue;
			glm::ivec2 newLineAngleVec = lineEnd->end - lineStart->start;
			float newLineAngle = ( atan2f( ( float ) -newLineAngleVec.y , ( float ) newLineAngleVec.x ) * 180 ) / ( float ) M_PI;
			if ( newLineAngle <= 0 ) newLineAngle += 360;
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
			if ( abs( gradientDirection[imageIndex] - theLines[i].angle ) <= angleThreshold )
			{
				theLines[i].start = lastPixel;
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
				if ( tempLastPixel.x >= 0 && tempLastPixel.x < width && tempLastPixel.y >= 0 && tempLastPixel.y < height )
				{
					unsigned long imageIndex2 = ( unsigned long ) ( ( tempLastPixel.y ) * this->width + ( tempLastPixel.x ) );
					if ( abs( gradientDirection[imageIndex2] - theLines[i].angle ) <= angleThreshold )
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
					if ( tempLastPixel.x >= 0 && tempLastPixel.x < width && tempLastPixel.y >= 0 && tempLastPixel.y < height )
					{
						unsigned long imageIndex2 = ( unsigned long ) ( ( tempLastPixel.y ) * this->width + ( tempLastPixel.x ) );
						if ( abs( gradientDirection[imageIndex2] - theLines[i].angle ) <= angleThreshold )
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
			unsigned long imageIndex = ( unsigned long ) ( ( lastPixel.y ) * this->width + ( lastPixel.x ) );
			if ( abs( gradientDirection[imageIndex] - theLines[i].angle ) <= angleThreshold )
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
				if ( tempLastPixel.x >= 0 && tempLastPixel.x < width && tempLastPixel.y >= 0 && tempLastPixel.y < height )
				{
					unsigned long imageIndex2 = ( unsigned long ) ( ( tempLastPixel.y ) * this->width + ( tempLastPixel.x ) );
					if ( abs( gradientDirection[imageIndex2] - theLines[i].angle ) <= angleThreshold )
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
					if ( tempLastPixel.x >= 0 && tempLastPixel.x < width && tempLastPixel.y >= 0 && tempLastPixel.y < height )
					{
						unsigned long imageIndex2 = ( unsigned long ) ( ( tempLastPixel.y ) * this->width + ( tempLastPixel.x ) );
						if ( abs( gradientDirection[imageIndex2] - theLines[i].angle ) <= angleThreshold )
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
				testPixel += normalized;
			}
			lastPixel = glm::ivec2( testPixel );
		}
	}

	////debug
	//{
	//	while ( numUsing ) std::cout << numUsing << std::endl;
	//	canGrab = false;

	//	for ( unsigned int i = 0; i < theLines.size(); ++i )
	//	{
	//		glm::vec2 normalized = glm::normalize( glm::vec2( theLines[i].end ) - glm::vec2( theLines[i].start ) );
	//		bool extendable = true;
	//		glm::vec2 testPixel = glm::vec2( theLines[i].start );
	//		glm::ivec2 lastPixel = theLines[i].start;
	//		//unsigned long iOffset = ( unsigned long ) ( ( lastPixel.y * 4 * this->width ) + ( lastPixel.x * 4 ) );
	//		//std::cout << lastPixel.y << std::endl;
	//		//std::cout << lastPixel.x << std::endl;
	//		//copiedPictureInstance[iOffset] = ( unsigned char ) 255;
	//		//copiedPictureInstance[iOffset+1] = ( unsigned char ) 0;
	//		//copiedPictureInstance[iOffset+2] = ( unsigned char ) 0;
	//		while ( glm::ivec2( testPixel ) == lastPixel )
	//		{
	//			testPixel -= normalized;
	//		}
	//		lastPixel = glm::ivec2( testPixel );
	//		while ( extendable && glm::length( testPixel - glm::vec2( theLines[i].start ) ) <= glm::length( glm::vec2( theLines[i].end ) - glm::vec2( theLines[i].start ) ) && lastPixel.x >= 0 && lastPixel.x < width && lastPixel.y >= 0 && lastPixel.y < height )
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
			quadResult.pt1 = quad.line[0]->end;
			quadResult.pt2 = quad.line[1]->end;
			quadResult.pt3 = quad.line[2]->end;
			quadResult.pt4 = quad.line[3]->end;
			quadResults.push_back( quadResult );

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

	//std::cout << "num lines " << theLines.size() << std::endl;
	//std::cout << "num quads " << quadResults.size() << std::endl;
	//std::cout << "full algoTime " << c.Stop() << std::endl;
}

bool ARMarkerDetector::findQuad( ConstructingQuad& quadToEdit , std::vector<Line>& lines , unsigned int index )
{
	if ( index > 1 ) std::cout << "highindex " << index << std::endl;
	//float angleThreshold = 15;
	float threshold = ((float)(width + height) / 2.0f) * 0.01f;
	//std::cout << threshold << std::endl;
	if ( index == 4 )
	{
		float length = glm::length( glm::vec2(quadToEdit.line[3]->end) - glm::vec2(quadToEdit.line[0]->start) );
		std::cout << (length ) << std::endl;
		return length <= threshold;
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
			std::cout << ( lineToCompare->angle + 90 ) - (currentLine->angle) << std::endl; 
			//std::cout << lineToCompare->angle << std::endl;
			//std::cout << currentLine->angle << std::endl;
			//if ( abs( ( lineToCompare->angle + 90 ) - currentLine->angle ) > angleThreshold ) continue;
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
	for ( long y = 0; y < height; y += 10 )
	{
		for ( long x = 0; x < width; x +=10 )
		{
			threads.push_back( std::async( std::launch::async , &ARMarkerDetector::findLinesOnRegion , this , x , y , 10 , 10 ) );
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
	{
		while ( numUsing ) std::cout << numUsing << std::endl;
		canGrab = false;

		for ( unsigned int i = 0; i < edgels.size(); ++i )
		{
			std::cout << edgels[i].edgel.pos.y << std::endl;
			std::cout << edgels[i].edgel.pos.x << std::endl;
			unsigned long iOffset = ( unsigned long ) ( ( edgels[i].edgel.pos.y * 4 * this->width ) + ( edgels[i].edgel.pos.x * 4 ) );
				copiedPictureInstance[iOffset] = ( unsigned char ) 255;
				copiedPictureInstance[iOffset + 1] = ( unsigned char ) 0;
				copiedPictureInstance[iOffset + 2] = ( unsigned char ) 0;
		}
		canGrab = true;
	}

	float angleThreshold = 1.0f;
	float maxDistance = 3;
	unsigned int maxIterations = (unsigned int)(1.75f * (height * width));
	unsigned int minNumEdgels = 5;

	std::vector<Line> tempLines;

	for ( unsigned int currentIteration = 0; edgels.size() >= minNumEdgels && currentIteration < maxIterations; ++currentIteration )
	{
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

		glm::vec2 theSlope = glm::normalize( glm::vec2(e2->edgel.pos) - glm::vec2(e1->edgel.pos) );
		theSlope.y *= -1;
		float slopeAngle = ( atan2f( theSlope.y , theSlope.x ) * 180 ) / (float)M_PI , oppSlopeAngle = (atan2f( (-theSlope).y , (-theSlope).x ) * 180) / (float)M_PI;
		if ( slopeAngle < 0 ) slopeAngle += 360;
		if ( oppSlopeAngle < 0 ) oppSlopeAngle += 360;
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
		//if(slopeAngle < 0) std::cout << slopeAngle << std::endl;
		//make better deletion
		e1->deletion = true;
		e2->deletion = true;
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
			float connectingLineAngleTest = ( atan2f( ( float ) -connectingLineTest.y , ( float ) connectingLineTest.x ) * 180 ) / ( float ) M_PI;
			if ( connectingLineAngleTest < 0 ) connectingLineAngleTest += 360;
			if ( abs( connectingLineAngleTest - lineStart->angle ) > angleThreshold || abs( connectingLineAngleTest - lineEnd->angle ) > angleThreshold ) continue;
			
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
				compatible = ( abs( gradientDirection[imageIndex] - lineStart->angle ) <= angleThreshold || abs( gradientDirection[imageIndex] - lineEnd->angle ) <= angleThreshold );
			}
			if ( !compatible ) continue;
			glm::ivec2 newLineAngleVec = lineEnd->end - lineStart->start;
			float newLineAngle = (atan2f( ( float ) -newLineAngleVec.y , ( float ) newLineAngleVec.x ) * 180 ) / ( float ) M_PI;
			if ( newLineAngle <=0 ) newLineAngle += 360;
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

	////debug
	//{
	//	while ( numUsing ) std::cout << numUsing << std::endl;
	//	canGrab = false;

	//	for ( unsigned int i = 0; i < tempLines.size(); ++i )
	//	{
	//		glm::vec2 normalized = glm::normalize( glm::vec2( tempLines[i].end ) - glm::vec2( tempLines[i].start ) );
	//		bool extendable = true;
	//		glm::vec2 testPixel = glm::vec2( tempLines[i].start );
	//		glm::ivec2 lastPixel = tempLines[i].start;
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
	//		while ( extendable && glm::length( testPixel - glm::vec2( tempLines[i].start ) ) <= glm::length( glm::vec2( tempLines[i].end ) - glm::vec2( tempLines[i].start ) ) && lastPixel.x >= 0 && lastPixel.x < width && lastPixel.y >= 0 && lastPixel.y < height )
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
	return tempLines;
}