#pragma warning(disable:4127)
#include <Qt\qapplication.h>
#include <Windows.h>
#include "War.h"
#include <WebCamSource.h>
#include "DebugMemory.h"
int main( int argc , char** argv )
{
#ifdef _DEBUG
	//_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF );
#endif
	CoInitialize(0);
	QApplication app(argc, argv);
	WebCamSource source;
	IEnumMoniker* cameraList;
	source.getListOfCameras( &cameraList );
	IMoniker* selected = 0;
	cameraList->Next( 1 , &selected , NULL );
	source.selectCamera( *selected );
	source.initialize();
	War war;
	war.setCameraSource( &source );
	war.show();
	int toReturn = app.exec();
	CoUninitialize();
	return toReturn;
}