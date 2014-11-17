#pragma once
#include <AniCardLibExportHeader.h>
#include <vector>
#include <string>
#include <dshow.h>
struct CameraMode
{
	VIDEO_STREAM_CONFIG_CAPS caps;
	std::string name;
};
struct CameraItem
{
	IBaseFilter* camera;
	std::string name;
	std::vector<CameraMode> modes;
};

class ANICARDLIB_SHARED WebCamHelpers
{
	std::vector<CameraItem> getListOfCameras();
};