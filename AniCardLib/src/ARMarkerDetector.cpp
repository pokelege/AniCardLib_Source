#define GLM_FORCE_RADIANS
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
#include <Misc\ExtraFunctions.h>
#include <thread>
#include <queue>
using namespace AniCardLib;

ARMarkerDetector::~ARMarkerDetector()
{
	if (runningThread )
	{
		while ( runningThread->wait_for( std::chrono::milliseconds( 0 ) ) != std::future_status::ready );
		delete runningThread;
	}

	if ( copiedPictureInstance ) delete[] copiedPictureInstance;
	if ( grayscaleImage ) delete[] grayscaleImage;
	if ( gradientDirection ) delete[] gradientDirection;
	if ( gradientIntensity ) delete[] gradientIntensity;
	runningThread = 0;
}

bool ARMarkerDetector::getPicture( unsigned char** bytes , long* width , long* height )
{
	if ( canGrab && bytes )
	{
		++numUsing;
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
	canGrab = false;
	while ( numUsing ) canGrab = false;
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

	for ( long row = 2; row < height - 2; ++row )
	{
		for ( long col = 2; col < width - 2; ++col )
		{
			float newPixel = 0;
			for ( long rowOffset = -2; rowOffset <= 2; ++rowOffset )
			{
				for ( long colOffset = -2; colOffset <= 2; ++colOffset )
				{
					
					long rowTotal = row + rowOffset;
					long colTotal = col + colOffset;
					
					unsigned long iOffset = ( unsigned long ) ( (rowTotal * this->width) + (colTotal ) );
					newPixel += grayscaleImage[iOffset] * gaussianMask[2 + rowOffset][2 + colOffset];
				}
			}

			unsigned long i = ( unsigned long ) ( ( row ) * this->width + ( col ) );
			
			grayscaleImage[i] = ( unsigned char ) glm::clamp( newPixel / 159 , 0.0f , ( float ) UCHAR_MAX );
		}
	}

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
			gradientDirection[i] = glm::degrees( atan2f( Gy , Gx ) ) + 90;
			if ( gradientDirection[i] < 0 ) gradientDirection[i] += 360;
			if ( gradientDirection[i] >= 360 ) gradientDirection[i] -= 360;
			gradientIntensity[i] = sqrt( ( Gx * Gx ) + ( Gy * Gy ) );
		}
	}
	findLines( theLines );
	float angleThreshold = 10;
	float upperThreshold = 100.0f;

	for ( unsigned int i = 0; i < theLines.size(); ++i )
	{
		Line* lineToConnect = &theLines[i];
		for ( unsigned int j = i + 1; j < theLines.size(); ++j )
		{
			Line* lineEnd = &theLines[j];
			if ( abs(differenceBetweenAngles(lineEnd->angle,lineToConnect->angle) ) > angleThreshold ) continue;
			Line* lineStart = lineToConnect;
			if ( glm::length( glm::vec2( lineEnd->end ) - glm::vec2( lineStart->start ) ) < glm::length( glm::vec2( lineStart->end ) - glm::vec2( lineEnd->start ) ) )
			{
				Line* tempLine = lineEnd;
				lineEnd = lineStart;
				lineStart = tempLine;
			}

			glm::vec2 connectingLineTest = glm::vec2( lineEnd->start ) - glm::vec2( lineStart->end );
			glm::vec2 normalizedConnectingLineTest = glm::normalize( connectingLineTest );
			float connectingLineAngleTest = glm::degrees( atan2f( -connectingLineTest.y , connectingLineTest.x ) );
			if ( connectingLineAngleTest < 0 ) connectingLineAngleTest += 360;
			if ( connectingLineAngleTest >= 360 ) connectingLineAngleTest -= 360;
			float angleAverage = ( lineStart->angle + lineEnd->angle ) / 2;
			if ( angleAverage < 0 ) angleAverage += 360;
			if ( angleAverage >= 360 ) angleAverage -= 360;
			if ( abs( differenceBetweenAngles( connectingLineAngleTest, angleAverage) ) > angleThreshold ) continue;

			glm::vec2 currentPosToCheck = glm::vec2( lineStart->end );
			glm::ivec2 lastPosToCheck = lineStart->end;
			bool compatible = true;
			while ( glm::length( glm::vec2( lastPosToCheck ) - glm::vec2( lineStart->end ) ) < glm::length( glm::vec2( connectingLineTest ) ) && compatible )
			{
				currentPosToCheck += normalizedConnectingLineTest;
				if ( glm::ivec2( currentPosToCheck ) == lastPosToCheck ) continue;
				lastPosToCheck = glm::ivec2( currentPosToCheck );
				unsigned long imageIndex = ( unsigned long ) ( ( lastPosToCheck.y  * this->width ) + ( lastPosToCheck.x ) );
				compatible = ( gradientIntensity[imageIndex] > upperThreshold ) && ( abs( differenceBetweenAngles( gradientDirection[imageIndex], angleAverage) ) <= angleThreshold );
			}
			if ( !compatible ) continue;
			glm::vec2 newLineAngleVec = glm::vec2( lineEnd->end ) - glm::vec2( lineStart->start );
			float newLineAngle = glm::degrees( atan2f( -newLineAngleVec.y , newLineAngleVec.x ) );
			if ( newLineAngle < 0 ) newLineAngle += 360;
			if ( newLineAngle >= 360 ) newLineAngle -= 360;
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
			if ( gradientIntensity[imageIndex] > upperThreshold && abs( differenceBetweenAngles( gradientDirection[imageIndex], theLines[i].angle) ) <= angleThreshold )
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
				}
				tempLastPixel = glm::ivec2(tempTestPixel);
				if ( tempLastPixel.x >= 0 && tempLastPixel.x < width && tempLastPixel.y >= 0 && tempLastPixel.y < height )
				{
					unsigned long imageIndex2 = ( unsigned long ) ( ( tempLastPixel.y  * this->width) + ( tempLastPixel.x ) );
					if ( gradientIntensity[imageIndex] > upperThreshold && abs( differenceBetweenAngles( gradientDirection[imageIndex2],theLines[i].angle) ) <= angleThreshold )
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
						if ( gradientIntensity[imageIndex] > upperThreshold && abs( differenceBetweenAngles( gradientDirection[imageIndex2], theLines[i].angle) ) <= angleThreshold )
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
			if ( gradientIntensity[imageIndex] > upperThreshold &&  abs( differenceBetweenAngles( gradientDirection[imageIndex], theLines[i].angle) ) <= angleThreshold )
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
					if ( gradientIntensity[imageIndex] > upperThreshold && abs(differenceBetweenAngles( gradientDirection[imageIndex2],theLines[i].angle) ) <= angleThreshold )
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
						if ( gradientIntensity[imageIndex] > upperThreshold && abs(differenceBetweenAngles ( gradientDirection[imageIndex2], theLines[i].angle) ) <= angleThreshold )
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

			if (  markerPack->matchMarker( quadResult , grayscaleImage , width , height ) )
			{
				quadResults.push_back( quadResult );
			}

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
				else theLines.erase( theLines.begin() + start , theLines.begin() + end + 1 );
			}
		}
	}

	std::sort( quadResults.begin() , quadResults.end() , dissimilarityCompare );
	std::cout << "quad before " << quadResults.size() << std::endl;
	for ( unsigned int i = 0; i < quadResults.size(); ++i )
	{

		float area = 0.5f * glm::length( quadResults.at( i ).pt[2] - quadResults.at( i ).pt[0] ) * glm::length( quadResults.at( i ).pt[3] - quadResults.at( i ).pt[1] );


		int start = -1 , end = -1;
		for ( unsigned int j = quadResults.size() - 1; j >= i && ( start < 0 || end < 0 ); --j )
		{
			float totalArea = FLT_MAX;
			if ( quadResults.at( i ).markerID == quadResults.at( j ).markerID && i != j )
			{
				totalArea = 0;
			}
			else if ( i == j ) totalArea = FLT_MAX;
			else
			{
				totalArea = 0;
				glm::vec2 center;
				for ( unsigned int k = 0; k < 4; ++k )
				{
					center += quadResults[j].pt[k];
				}
				center *= 0.25f;


				for ( unsigned int k = 0; k < 4; ++k )
				{
					glm::vec2 point1 = quadResults.at( i ).pt[k % 4];
					glm::vec2 point2 = quadResults.at( i ).pt[( k + 1 ) % 4];

					glm::vec2 normal = glm::normalize( point2 - point1 );
					glm::vec2 centerOnPoint1 = center - point1;

					float dot = glm::dot( centerOnPoint1 , normal );
					glm::vec2 perp = ( dot * normal ) + point1;
					totalArea += 0.5f * glm::length( point2 - point1 ) * glm::length( center - perp );
					if ( totalArea > area ) break;
				}
			}
			if ( totalArea <= area && end < 0 ) end = j;
			else if ( end >= 0 && totalArea > area )
			{
				start = j + 1;
			}
		}
		if ( end < 0 ) continue;
		else if ( start < 0 && (int)i + 1 == end ) quadResults.erase( quadResults.begin() + end );
		else if ( start < 0 && ( int ) i + 1 < end ) quadResults.erase( quadResults.begin() + i + 1 , quadResults.begin() + end + 1 );
		else if(start >= 0 && end >= 0) quadResults.erase( quadResults.begin() + start , quadResults.begin() + end + 1 );
	}
	std::cout << "quad after " << quadResults.size() << std::endl;
	canGrabMarkerFound = false;
	while ( numUsingMarkerFound ) std::cout << numUsingMarkerFound << std::endl;
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

		FoundMarkerInfo found;
		found.transform = quadResults[i].transform;
		found.center = center;
		found.cardIndex = quadResults[i].markerID;
		found.dissimilarity = quadResults[i].dissimilarity;
		toSend.push_back( found );
	}

	canGrabMarkerFound = true;
}

bool ARMarkerDetector::findQuad( ConstructingQuad& quadToEdit , std::vector<Line>& lines , unsigned int index )
{
	float angleThreshold = 10;
	float threshold = ((float)(width + height) / 2.0f) * 0.015f;
	if ( index == 4 )
	{
		float length = glm::length( glm::vec2(quadToEdit.line[3]->end) - glm::vec2(quadToEdit.line[0]->start) );
		float lineToCompareAngle = quadToEdit.line[3]->angle + 90;
		if ( lineToCompareAngle >= 360 ) lineToCompareAngle -= 360;
		if ( lineToCompareAngle < 0 ) lineToCompareAngle += 360;
		bool found = length <= threshold;
		float angleDifference = abs( differenceBetweenAngles( lineToCompareAngle , quadToEdit.line[0]->angle ) );
	
		if ( !found && angleDifference  <= angleThreshold )
		{
			glm::vec2 d = glm::vec2(quadToEdit.line[3]->start - quadToEdit.line[0]->end);
			glm::vec2 linebDir = glm::normalize( glm::vec2( quadToEdit.line[3]->end - quadToEdit.line[3]->start ) );
			glm::vec2 lineaDir = glm::normalize( glm::vec2( quadToEdit.line[0]->start - quadToEdit.line[0]->end ) );
			float determinant = ( linebDir.x * lineaDir.y ) - ( linebDir.y * lineaDir.x );
			float u = ( ( d.y * linebDir.x ) - ( d.x * linebDir.y ) ) / determinant;
			float v = ( ( d.y * lineaDir.x ) - ( d.x * lineaDir.y ) ) / determinant;
			if ( u > 0 && v > 0 )
			{
				glm::vec2 newPosA = glm::vec2(quadToEdit.line[0]->end) + ( u * lineaDir );
				glm::vec2 newPosB = glm::vec2( quadToEdit.line[3]->start ) + ( v * linebDir );

				if ( newPosA == newPosB )
				{
					quadToEdit.line[0]->start = newPosA;
					quadToEdit.line[3]->end = newPosB;

					found = true;
				}
			}
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
			float lineToCompareAngle = lineToCompare->angle + 90;
			if ( lineToCompareAngle >= 360 ) lineToCompareAngle -= 360;
			if ( lineToCompareAngle < 0 ) lineToCompareAngle += 360;
			float angleError = abs( differenceBetweenAngles( lineToCompareAngle , currentLine->angle ) );
			
			if ( angleError > angleThreshold )
			{
				continue;
			}
			
			quadToEdit.line[index] = currentLine;
			found = findQuad( quadToEdit , lines , index + 1 );
		}
	}
	if ( !found )
	{
		quadToEdit.line[index] = 0;
		
	}
	
	return found;
}

void ARMarkerDetector::findLines( std::vector<Line>& linesToAdd )
{
	std::queue<std::future<std::vector<Line>>> threads;
	
	unsigned int maxThreads = std::thread::hardware_concurrency();
	if ( !maxThreads ) maxThreads = UINT_MAX;

	long y = 0 , x = 0;
	while ( y < height )
	{
		while ( x < width && threads.size() < maxThreads )
		{
			threads.push( std::async( std::launch::async , &ARMarkerDetector::findLinesOnRegion , this , x , y , 20 , 20 ) );
			x += 20;
		}
		if ( x >= width )
		{
			y += 20;
			x = 0;
		}

		if ( threads.size() >= maxThreads )
		{
			threads.front().wait();
			std::vector<Line> theLines = threads.front().get();
			threads.pop();
			for ( unsigned int j = 0; j < theLines.size(); ++j )
			{
				linesToAdd.push_back( theLines[j] );
			}
		}
	}

	while ( threads.size() )
	{
		threads.front().wait();
		std::vector<Line> theLines = threads.front().get();
		threads.pop();
		for ( unsigned int j = 0; j < theLines.size(); ++j )
		{
			linesToAdd.push_back( theLines[j] );
		}
	}
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

	float angleThreshold = 5.0f;
	float maxDistance = ((float)(width + height) / 3.0f);
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

			float angleTest = std::abs(differenceBetweenAngles(e1->edgel.angle, e2->edgel.angle));
			
			found = abs( angleTest ) <= angleThreshold;
		}
		if ( !found ) continue;

		glm::vec2 theSlope = glm::normalize( glm::vec2(e2->edgel.pos) - glm::vec2(e1->edgel.pos) );
		float slopeAngle = glm::degrees(atan2f( -theSlope.y , theSlope.x));
		float oppSlopeAngle = glm::degrees(atan2f( theSlope.y , -theSlope.x ));
		if ( oppSlopeAngle < 0 ) oppSlopeAngle += 360;
		if ( oppSlopeAngle >= 360 ) oppSlopeAngle -= 360;
		if ( slopeAngle < 0 ) slopeAngle += 360;
		if ( slopeAngle >= 360 ) slopeAngle -= 360;

		float edgelAngleAverage = ( e1->edgel.angle + e2->edgel.angle ) / 2;

		if ( abs( differenceBetweenAngles( oppSlopeAngle , edgelAngleAverage ) ) < abs( differenceBetweenAngles( slopeAngle , edgelAngleAverage ) ) )
		{
			EdgelInList* temp = e2;
			e2 = e1;
			e1 = temp;
			slopeAngle = oppSlopeAngle;
		}

		
		std::vector<EdgelInList*> supportedEdgels;
		glm::ivec2 endAtStartSpace = e2->edgel.pos - e1->edgel.pos;
		float lengthOfLine = glm::length( glm::vec2(endAtStartSpace) );
		glm::vec2 lineNormal = glm::normalize( glm::vec2(endAtStartSpace));
		for ( unsigned int i = 0; i < edgels.size(); ++i )
		{
			EdgelInList* test = &edgels[i];
			if ( test == e1 || test == e2 ) continue;
			if ( ( abs( differenceBetweenAngles( test->edgel.angle , slopeAngle ) ) > angleThreshold ) ) continue;
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
			else if ( end < 0 ) edgels.erase( edgels.begin() + start , edgels.end() );
			else if ( start == end || (end < 0 && edgels.begin() + start + 1 == edgels.end()) ) edgels.erase( edgels.begin() + start );
			else edgels.erase( edgels.begin() + start , edgels.begin() + end + 1 );
		}
		tempLines.push_back( line );
	}

	for ( unsigned int i = 0; i < tempLines.size(); ++i )
	{
		Line* lineToConnect = &tempLines[i];
		for ( unsigned int j = i + 1; j < tempLines.size(); ++j )
		{
			Line* lineEnd = &tempLines[j];
			if ( abs( differenceBetweenAngles(lineEnd->angle,lineToConnect->angle) ) > angleThreshold ) continue;
			Line* lineStart = lineToConnect;
			if ( glm::length( glm::vec2(lineEnd->end) - glm::vec2(lineStart->start) ) < glm::length( glm::vec2(lineStart->end) - glm::vec2(lineEnd->start) ) )
			{
				Line* tempLine = lineEnd;
				lineEnd = lineStart;
				lineStart = tempLine;
			}

			glm::vec2 connectingLineTest = glm::vec2(lineEnd->start) - glm::vec2(lineStart->end);
			glm::vec2 normalizedConnectingLineTest = glm::normalize( connectingLineTest );
			float connectingLineAngleTest = glm::degrees( atan2f( -connectingLineTest.y , connectingLineTest.x ) );
			if ( connectingLineAngleTest < 0 ) connectingLineAngleTest += 360;
			if ( connectingLineAngleTest >= 360 ) connectingLineAngleTest -= 360;
			float angleAverage = ( lineStart->angle + lineEnd->angle ) / 2;
			if ( angleAverage < 0 ) angleAverage += 360;
			if ( angleAverage >= 360 ) angleAverage -= 360;
			if ( abs( differenceBetweenAngles(connectingLineAngleTest,angleAverage) ) > angleThreshold ) continue;
			
			glm::vec2 currentPosToCheck = glm::vec2(lineStart->end);
			glm::ivec2 lastPosToCheck = lineStart->end;
			bool compatible = true;
			while ( glm::length( glm::vec2( lastPosToCheck ) - glm::vec2( lineStart->end ) ) < glm::length( glm::vec2( connectingLineTest ) ) && compatible )
			{
				currentPosToCheck += normalizedConnectingLineTest;
				if ( glm::ivec2( currentPosToCheck ) == lastPosToCheck ) continue;
				lastPosToCheck = glm::ivec2( currentPosToCheck );
				unsigned long imageIndex = ( unsigned long ) ( ( lastPosToCheck.y  * this->width) + ( lastPosToCheck.x ) );
				compatible = ( gradientIntensity[imageIndex] > upperThreshold ) && ( abs( differenceBetweenAngles( gradientDirection[imageIndex], angleAverage) ) <= angleThreshold );
			}
			if ( !compatible ) continue;
			glm::vec2 newLineAngleVec = glm::vec2(lineEnd->end) - glm::vec2(lineStart->start);
			float newLineAngle = glm::degrees(atan2f( -newLineAngleVec.y , newLineAngleVec.x ) );
			if ( newLineAngle < 0 ) newLineAngle += 360;
			if ( newLineAngle >= 360 ) newLineAngle -= 360;
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
	return tempLines;
}

bool dissimilarityCompare( Quad& i , Quad& j ) { return ( i.dissimilarity < j.dissimilarity ); }