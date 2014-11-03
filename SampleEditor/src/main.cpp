#include <Editor.h>
#include <QtGui\QApplication>
int main( int argc , char** argv )
{
	QApplication app( argc , argv );
	Editor edit;
	edit.showMaximized();
	return app.exec();
}