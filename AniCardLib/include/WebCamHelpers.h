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
public:
	std::vector<CameraItem> getListOfCameras();
};

bool VIDEO_STREAM_CONFIG_CAPS_Equals( VIDEO_STREAM_CONFIG_CAPS& a , VIDEO_STREAM_CONFIG_CAPS& b );