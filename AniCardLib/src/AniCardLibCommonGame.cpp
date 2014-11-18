#include <AniCardLibCommonGame.h>
#include <MarkerPack.h>
#include <WebCamSource.h>
#include <Graphics\TextureInfo.h>
AniCardLibCommonGame::AniCardLibCommonGame() :markerPack(0) , markerDetector(0) , webCam(0)
{
	markerDetector = new ARMarkerDetector;
}

AniCardLibCommonGame::~AniCardLibCommonGame()
{
	if ( markerDetector ) delete markerDetector;
	if ( markerPack ) delete markerPack;
	if ( webCam ) delete webCam;
}

void AniCardLibCommonGame::setMarkerPack( const char* filename )
{
	if ( markerPack ) delete markerPack;
	markerPack = new MarkerPack();
	markerPack->load( filename );
}

std::vector<FoundMarkerInfo> AniCardLibCommonGame::queryResultList()
{
	if ( !markerDetector ) return std::vector<FoundMarkerInfo>();
	std::vector<FoundMarkerInfo>* temp = 0;
	if ( !markerDetector->getMarkerFound( &temp ) ) return std::vector<FoundMarkerInfo>();
	std::vector<FoundMarkerInfo> toReturn( *temp );
	markerDetector->finishedUsingMarkerFound();
	return toReturn;
}

void AniCardLibCommonGame::update()
{
	if ( markerDetector && webCam && markerPack )
		markerDetector->findCard( webCam->fetcher , markerPack );
}

std::vector<CameraItem> AniCardLibCommonGame::getAvailableCameras()
{
	WebCamHelpers help;
	return help.getListOfCameras();
}

void AniCardLibCommonGame::setCamera( CameraItem& camera , CameraMode& mode )
{
	if ( webCam ) delete webCam;
	webCam = new WebCamSource;
	webCam->initialize( camera , mode );
}

unsigned int AniCardLibCommonGame::getGeometryListSize()
{
	if ( markerPack ) return markerPack->getGeometryListSize();
	else return 0;
}

GeometryInfo* AniCardLibCommonGame::getGeometry( unsigned int& id )
{
	if ( markerPack ) return markerPack->getGeometry( id );
	else return 0;
}

GeometryInfo* AniCardLibCommonGame::getCardGeometry( unsigned int& id )
{
	if ( markerPack ) return markerPack->getCardGeometry( id );
	else return 0;
}
TextureInfo* AniCardLibCommonGame::getCardTexture( unsigned int& id )
{
	if ( markerPack ) return markerPack->getCardTexture( id );
	else return 0;
}

bool AniCardLibCommonGame::copyPicture( TextureInfo* texture , long* width , long* height )
{
	if ( !( webCam && webCam->fetcher ) ) return false;
	unsigned char* pictureData = 0;
	long lewidth;
	long leheight;
	if ( webCam->fetcher->getPicture( &pictureData , &lewidth , &leheight ) )
	{
		texture->editTexture( ( char* ) pictureData , lewidth , leheight , 0 );
		webCam->fetcher->finishedUsing();
		*width = lewidth;
		*height = leheight;
		webCam->fetcher->finishedUsing();
		return true;
	}
	return false;
}