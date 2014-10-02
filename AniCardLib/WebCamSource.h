#pragma once
#include <dshow.h>
#include <vector>
class WebCamSource
{
	IBaseFilter* selectedCamera;
public:
	WebCamSource();
	HRESULT getListOfCameras( IEnumMoniker** theListToPopulate );
	HRESULT selectCamera( IMoniker& camera );
};