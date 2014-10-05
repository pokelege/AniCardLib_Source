#include <WebCamSource.h>

void main()
{
	CoInitialize(0);
	WebCamSource source;
	IEnumMoniker* list;
	source.getListOfCameras( &list );
	IMoniker* selected = 0;
	list->Next( 1 , &selected, NULL );
	source.selectCamera( *selected );
	source.initialize();
	while ( true );
	CoUninitialize();
}