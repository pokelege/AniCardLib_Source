#include <Preview.h>
#include <Graphics\CommonGraphicsCommands.h>
void Preview::initializeGL()
{
	CommonGraphicsCommands::initializeGlobalGraphics();
	emit initialized();
}