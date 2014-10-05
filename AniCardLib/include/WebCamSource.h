#pragma once
#include <dshow.h>
#include <vector>
#include <AniCardLibExportHeader.h>
#include <PictureFetcher.h>
class ANICARDLIB_SHARED WebCamSource
{
	PictureFetcher* fetcher;
	IBaseFilter* selectedCamera;
	IMediaControl* media;
public:
	WebCamSource();
	HRESULT getListOfCameras( IEnumMoniker** theListToPopulate );
	HRESULT selectCamera( IMoniker& camera );
	HRESULT initialize();
};