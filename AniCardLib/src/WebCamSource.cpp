#include <WebCamSource.h>
#include <qedit.h>
#include <wmcodecdsp.h>
#include <dmodshow.h>
#include <Dmoreg.h>
#include <iostream>
WebCamSource::WebCamSource() : selectedCamera(0)
{

}
WebCamSource::~WebCamSource()
{
	destroy();
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

HRESULT WebCamSource::selectCamera( IMoniker& camera , std::vector<CameraConfigs>& caps )
{
	IBaseFilter* test = 0;
	HRESULT result = camera.BindToObject(0,0,IID_IBaseFilter, reinterpret_cast<void**>(&test));
	if ( SUCCEEDED( result ) ) selectedCamera = test;
	IPin* cameraPin;
	IEnumPins* enumPins;
	selectedCamera->EnumPins( &enumPins );
	while ( S_OK == enumPins->Next( 1 , &cameraPin , NULL ) )
	{
		PIN_DIRECTION pinDir;
		cameraPin->QueryDirection( &pinDir );
		if ( pinDir == PINDIR_OUTPUT ) break;
	}

	IAMStreamConfig* configuration;
	cameraPin->QueryInterface( IID_IAMStreamConfig , ( void** ) &configuration );
	int count , size;
	configuration->GetNumberOfCapabilities( &count , &size );
	if ( size == sizeof( VIDEO_STREAM_CONFIG_CAPS ) )
	{
		for ( int i = 0; i < count; ++i )
		{
			VIDEO_STREAM_CONFIG_CAPS con;
			AM_MEDIA_TYPE* type;
			if (SUCCEEDED(configuration->GetStreamCaps( i , &type , ( BYTE* ) &con ) ))
			{
				CameraConfigs lolz;
				lolz.caps = con;
				lolz.index = i;
				caps.push_back( lolz );
			}

		}
	}

	return result;
}

void WebCamSource::selectResolution( CameraConfigs& config )
{
	IPin* cameraPin;
	IEnumPins* enumPins;
	selectedCamera->EnumPins( &enumPins );
	while ( S_OK == enumPins->Next( 1 , &cameraPin , NULL ) )
	{
		PIN_DIRECTION pinDir;
		cameraPin->QueryDirection( &pinDir );
		if ( pinDir == PINDIR_OUTPUT ) break;
	}

	IAMStreamConfig* configuration;
	cameraPin->QueryInterface( IID_IAMStreamConfig , ( void** ) &configuration );
	int count , size;
	configuration->GetNumberOfCapabilities( &count , &size );
	if ( size == sizeof( VIDEO_STREAM_CONFIG_CAPS ) )
	{
		VIDEO_STREAM_CONFIG_CAPS con;
		AM_MEDIA_TYPE* type;
		if ( SUCCEEDED( configuration->GetStreamCaps( config.index , &type , ( BYTE* ) &con ) ) )
		{
			configuration->SetFormat( type );
		}
	}
}


HRESULT WebCamSource::initialize()
{
	CoCreateInstance( CLSID_FilterGraph , NULL , CLSCTX_INPROC_SERVER , IID_PPV_ARGS( &graph ) );
	graph->AddFilter( selectedCamera , L"Camera" );

	IBaseFilter* converter;
	CoCreateInstance( CLSID_DMOWrapperFilter , NULL , CLSCTX_INPROC_SERVER , IID_IBaseFilter , ( void** ) &converter );

	IDMOWrapperFilter* wrapper;
	converter->QueryInterface( IID_IDMOWrapperFilter , ( void** ) &wrapper );
	wrapper->Init( CLSID_CColorConvertDMO , DMOCATEGORY_VIDEO_EFFECT );
	wrapper->Release();
	graph->AddFilter( converter , L"Converter" );

	IBaseFilter* grabber;
	CoCreateInstance( CLSID_SampleGrabber , NULL , CLSCTX_INPROC_SERVER , IID_IBaseFilter, (void**)&grabber);
	graph->AddFilter( grabber , L"Grabber" );

	IBaseFilter* nullRenderer;
	CoCreateInstance( CLSID_NullRenderer , NULL , CLSCTX_INPROC_SERVER , IID_IBaseFilter , ( void** ) &nullRenderer );
	graph->AddFilter( nullRenderer , L"NullRender" );

	IPin *cameraPin, *converterPin, *converterPin2 , *grabberPin,*grabberPin2 , *rendererPin;

	IEnumPins* enumPins;
	selectedCamera->EnumPins( &enumPins );
	while ( S_OK == enumPins->Next( 1 , &cameraPin , NULL ) )
	{
		PIN_DIRECTION pinDir;
		cameraPin->QueryDirection( &pinDir );
		if ( pinDir == PINDIR_OUTPUT ) break;
	}

	converter->EnumPins( &enumPins );
	while ( S_OK == enumPins->Next( 1 , &converterPin , NULL ) )
	{
		PIN_DIRECTION pinDir;
		converterPin->QueryDirection( &pinDir );
		if ( pinDir == PINDIR_INPUT ) break;
	}

	converter->EnumPins( &enumPins );
	while ( S_OK == enumPins->Next( 1 , &converterPin2 , NULL ) )
	{
		PIN_DIRECTION pinDir;
		converterPin2->QueryDirection( &pinDir );
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

	ISampleGrabber* sampleGrabber;

	grabber->QueryInterface( IID_ISampleGrabber , ( void** ) &sampleGrabber );
	sampleGrabber->SetOneShot( FALSE );
	sampleGrabber->SetBufferSamples( TRUE );
	AM_MEDIA_TYPE mediaType2;
	ZeroMemory( &mediaType2 , sizeof( mediaType2 ) );
	mediaType2.majortype = MEDIATYPE_Video;
	mediaType2.subtype = MEDIASUBTYPE_RGB32;
	sampleGrabber->SetMediaType( &mediaType2 );
	sampleGrabber->Release();

	graph->Connect( cameraPin , converterPin );
	graph->Connect( converterPin2 , grabberPin );
	graph->Connect( grabberPin2 , rendererPin );

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

HRESULT WebCamSource::destroy()
{
	if ( !selectedCamera ) return E_POINTER;
	media->Stop();
	delete fetcher;
	return S_OK;
}