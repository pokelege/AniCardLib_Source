#pragma once
#include <dshow.h>
#include <vector>
#include <AniCardLibExportHeader.h>
#include <PictureFetcher.h>
class ANICARDLIB_SHARED WebCamSource
{
	IBaseFilter* selectedCamera;
	IMediaControl* media;
	IGraphBuilder* graph;
public:
	PictureFetcher* fetcher;
	WebCamSource();
	~WebCamSource();
	HRESULT getListOfCameras( IEnumMoniker** theListToPopulate );
	HRESULT selectCamera( IMoniker& camera );
	HRESULT initialize();
	HRESULT destroy();
};