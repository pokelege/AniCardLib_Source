#pragma once
#include <dshow.h>
#include <vector>
#include <AniCardLibExportHeader.h>
#include <PictureFetcher.h>

struct CameraConfigs
{
	VIDEO_STREAM_CONFIG_CAPS caps;
	int index;
};

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
	HRESULT selectCamera( IMoniker& camera , std::vector<CameraConfigs>& caps );
	void selectResolution( CameraConfigs& config );
	HRESULT initialize();
	HRESULT destroy();
};