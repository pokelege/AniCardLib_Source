#include <Editor.h>
#include <QtGui\QApplication>
#include <DebugMemory.h>
int main( int argc , char** argv )
{
#ifdef _DEBUG
	//_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF );
#endif
	QApplication app( argc , argv );
	Editor edit;
	edit.showMaximized();
	return app.exec();
}