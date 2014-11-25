#include <WebCamSource.h>
#include <qedit.h>
#include <wmcodecdsp.h>
#include <dmodshow.h>
#include <Dmoreg.h>
#include <iostream>
#include <vidcap.h>
#include <ksmedia.h>
#include <comdef.h>
#include <WebCamHelpers.h>
WebCamSource::WebCamSource()
{

}
WebCamSource::~WebCamSource()
{
	destroy();
}

int WebCamSource::initialize( CameraItem& camera , CameraMode& mode )
{
	CoCreateInstance( CLSID_FilterGraph , NULL , CLSCTX_INPROC_SERVER , IID_PPV_ARGS( &graph ) );
	graph->AddFilter( camera.camera , L"Camera" );

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
	camera.camera->EnumPins( &enumPins );
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
			if ( SUCCEEDED( configuration->GetStreamCaps( i, &type , ( BYTE* ) &con ) ) )
			{
				if ( VIDEO_STREAM_CONFIG_CAPS_Equals( con , mode.caps ))
				{
					configuration->SetFormat( type );
				}
			}
		}
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
	unsigned char* test = 0;
	while ( !fetcher->getPicture( &test , 0 , 0 ) )
	{
		fetcher->finishedUsing();
	}
	fetcher->finishedUsing();
	return 0;
}

int WebCamSource::destroy()
{
	media->Stop();
	if(fetcher) delete fetcher;
	return S_OK;
}

bool VIDEO_STREAM_CONFIG_CAPS_Equals( VIDEO_STREAM_CONFIG_CAPS& a , VIDEO_STREAM_CONFIG_CAPS& b )
{
	return (
		a.guid == b.guid &&
		a.MaxOutputSize.cx == b.MaxOutputSize.cx &&
		a.MaxOutputSize.cy == b.MaxOutputSize.cy &&
		a.VideoStandard == b.VideoStandard &&
		a.InputSize.cx == b.InputSize.cx &&
		a.InputSize.cx == b.InputSize.cx &&
		a.MaxFrameInterval == b.MaxFrameInterval
		);
}