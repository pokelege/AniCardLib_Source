#include <Qt\qapplication.h>
#include <Windows.h>
#include "War.h"
#include <WebCamSource.h>
int main( int argc , char** argv )
{
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