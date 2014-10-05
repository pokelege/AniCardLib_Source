#include <WebCamSource.h>
#include <qedit.h>
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

HRESULT WebCamSource::initialize()
{
	IGraphBuilder* graph;
	CoCreateInstance( CLSID_FilterGraph , NULL , CLSCTX_INPROC_SERVER , IID_PPV_ARGS( &graph ) );
	graph->AddFilter( selectedCamera , L"Camera" );
	IBaseFilter* grabber;
	CoCreateInstance( CLSID_SampleGrabber , NULL , CLSCTX_INPROC_SERVER , IID_IBaseFilter, (void**)&grabber);
	graph->AddFilter( grabber , L"Grabber" );

	IBaseFilter* nullRenderer;
	CoCreateInstance( CLSID_NullRenderer , NULL , CLSCTX_INPROC_SERVER , IID_IBaseFilter , ( void** ) &nullRenderer );
	graph->AddFilter( nullRenderer , L"NullRender" );

	IPin *cameraPin , *grabberPin,*grabberPin2 , *rendererPin;

	IEnumPins* enumPins;
	selectedCamera->EnumPins( &enumPins );
	while ( S_OK == enumPins->Next( 1 , &cameraPin , NULL ) )
	{
		PIN_DIRECTION pinDir;
		cameraPin->QueryDirection( &pinDir );
		if ( pinDir == PINDIR_OUTPUT ) break;
	}

	grabber->EnumPins( &enumPins );
	while ( S_OK == enumPins->Next( 1 , &grabberPin , NULL ) )
	{
		PIN_DIRECTION pinDir;
		grabberPin->QueryDirection( &pinDir );
		if ( pinDir == PINDIR_INPUT ) break;
	}

	grabber->EnumPins( &enumPins );
	while ( S_OK == enumPins->Next( 1 , &grabberPin2 , NULL ) )
	{
		PIN_DIRECTION pinDir;
		grabberPin2->QueryDirection( &pinDir );
		if ( pinDir == PINDIR_OUTPUT ) break;
	}

	nullRenderer->EnumPins( &enumPins );
	while ( S_OK == enumPins->Next( 1 , &rendererPin , NULL ) )
	{
		PIN_DIRECTION pinDir;
		rendererPin->QueryDirection( &pinDir );
		if ( pinDir == PINDIR_INPUT ) break;
	}

	graph->Connect( cameraPin , grabberPin );
	graph->Connect( grabberPin2 , rendererPin );

	ISampleGrabber* sampleGrabber;

	grabber->QueryInterface( IID_ISampleGrabber , ( void** ) &sampleGrabber );
	sampleGrabber->SetOneShot( FALSE );
	sampleGrabber->SetBufferSamples( TRUE );
	AM_MEDIA_TYPE mediaType;
	sampleGrabber->GetConnectedMediaType( &mediaType );
	fetcher = new PictureFetcher( mediaType );
	sampleGrabber->SetCallback( fetcher , NULL );
	sampleGrabber->Release();
	graph->QueryInterface( IID_PPV_ARGS( &media ) );
	media->Run();
	return 0;
}