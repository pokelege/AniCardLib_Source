#pragma warning(disable:4127)
#include <Qt\qapplication.h>
#include <Windows.h>
#include <Setup.h>
#include <WebCamSource.h>
#include <DebugMemory.h>
int main( int argc , char** argv )
{
#ifdef _DEBUG
	//_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF );
#endif
	
	CoInitialize(0);
	QApplication app( argc , argv );
	Setup* set = new Setup;
	set->setAttribute( Qt::WA_DeleteOnClose );
	set->show();
	int toReturn = app.exec();
	CoUninitialize();
	return toReturn;
}