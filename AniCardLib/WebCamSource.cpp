#include "WebCamSource.h"

WebCamSource::WebCamSource() : selectedCamera(0)
{

}

HRESULT WebCamSource::getListOfCameras( IEnumMoniker** theListToPopulate )
{
	HRESULT result;
	if ( theListToPopulate )
	{
		ICreateDevEnum* deviceEnum = 0;
		result = CoCreateInstance( CLSID_SystemDeviceEnum , NULL , CLSCTX_INPROC_SERVER , IID_PPV_ARGS( &deviceEnum ) );
		if ( SUCCEEDED( result ) && deviceEnum )
		{
			result = deviceEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory , theListToPopulate , 0 );
			if ( result == S_FALSE ) result = VFW_E_NOT_FOUND;
			deviceEnum->Release();
		}
	}
	else
	{
		result = E_POINTER;
	}
	return result;
}

HRESULT WebCamSource::selectCamera( IMoniker& camera )
{
	IBaseFilter* test = 0;
	HRESULT result = camera.BindToObject(0,0,IID_IBaseFilter, reinterpret_cast<void**>(&test));
	if ( SUCCEEDED( result ) ) selectedCamera = test;
	return result;
}