#include <WebCamHelpers.h>
#include <qedit.h>
#include <dshow.h>
std::vector<CameraItem> WebCamHelpers::getListOfCameras()
{
	std::vector<CameraItem> cameras;
	HRESULT result;
	IEnumMoniker* theListToPopulate = 0;
	ICreateDevEnum* deviceEnum = 0;
	result = CoCreateInstance( CLSID_SystemDeviceEnum , NULL , CLSCTX_INPROC_SERVER , IID_PPV_ARGS( &deviceEnum ) );
	if ( SUCCEEDED( result ) && deviceEnum )
	{
		result = deviceEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory ,&theListToPopulate , 0 );
		if ( result == S_FALSE ) result = VFW_E_NOT_FOUND;
		deviceEnum->Release();
	}
	if ( result != VFW_E_NOT_FOUND )
	{
		IMoniker* selected = 0;
		while ( theListToPopulate->Next( 1 , &selected , NULL ) == S_OK )
		{
			IPropertyBag* properties;
			if ( FAILED( selected->BindToStorage( 0 , 0 , IID_PPV_ARGS( &properties ) ) ) )
			{
				selected->Release();
				continue;
			}

			VARIANT var;
			VariantInit( &var );

			if ( SUCCEEDED( properties->Read( L"FriendlyName" , &var , 0 ) ) )
			{
				{
					IBaseFilter* test = 0;
					HRESULT result = selected->BindToObject( 0 , 0 , IID_IBaseFilter , reinterpret_cast<void**>( &test ) );
					if ( SUCCEEDED( result ) )
					{
						CameraItem item;
						item.camera = test;

						size_t origsize = wcslen( var.bstrVal );
						size_t numConverted = 0;
						const size_t newsize = origsize * 2;
						char* nstring = new char[newsize];
						wcstombs_s( &numConverted , nstring ,newsize, var.bstrVal , _TRUNCATE );
						//wcstombs( nstring , var.bstrVal , origsize );

						item.name = std::string( nstring );
						delete[] nstring;


						IPin* cameraPin;
						IEnumPins* enumPins;
						test->EnumPins( &enumPins );
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
								if ( SUCCEEDED( configuration->GetStreamCaps( i , &type , ( BYTE* ) &con ) ) )
								{
									CameraMode mode;
									mode.caps = con;
									mode.name = std::to_string( con.MaxOutputSize.cx ) + " x " + std::to_string( con.MaxOutputSize.cy );
									item.modes.push_back( mode );
								}
							}
						}

						cameras.push_back( item );
					}
				}
			}
			else
			{
				selected->Release();
			}
		}
	}
	return cameras;
}