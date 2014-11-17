#pragma once
#include <dshow.h>
#include <vector>
#include <AniCardLibExportHeader.h>
#include <PictureFetcher.h>
struct CameraItem;
struct CameraMode;
class ANICARDLIB_SHARED WebCamSource
{
	IMediaControl* media;
	IGraphBuilder* graph;
public:
	PictureFetcher* fetcher;
	WebCamSource();
	~WebCamSource();
	int initialize( CameraItem& camera , CameraMode& mode );
	int destroy();
};