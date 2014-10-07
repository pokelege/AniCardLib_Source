#pragma once
#include <dshow.h>
#include <vector>
#include <AniCardLibExportHeader.h>
#include <PictureFetcher.h>
class ANICARDLIB_SHARED WebCamSource
{
	IBaseFilter* selectedCamera;
	IMediaControl* media;
public:
	PictureFetcher* fetcher;
	WebCamSource();
	HRESULT getListOfCameras( IEnumMoniker** theListToPopulate );
	HRESULT selectCamera( IMoniker& camera );
	HRESULT initialize();
};