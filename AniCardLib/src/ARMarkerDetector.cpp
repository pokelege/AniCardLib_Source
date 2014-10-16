#include <ARMarkerDetector.h>
#include <PictureFetcher.h>
void ARMarkerDetector::findCard( PictureFetcher* thePhoto )
{
	if ( !thePhoto ) return;


}

void ARMarkerDetector::findEdgels( PictureFetcher* thePhoto )
{
	if ( !thePhoto ) return;
	unsigned char* bytes;
	long width , height;
	thePhoto->getPicture( &bytes , &width , &height );

	//int edgeDir[240][320];
	//float gradient[240][320];

	//long row , col;
	//unsigned long i;
	//int upperThreshold = 60;
	//int lowerThreshold = 30;
	//unsigned long iOffset;
	//int rowOffset,colOffest;
	//int rowTotal = 0 , colTotal = 0;
	//int Gx , Gy;
	//float angle;
	//bool edgeEnd;
	//int GxMask[3][3];
	//int GyMask[3][3];
	//int newPixel;
	//int gaussianMask[5][5];

}