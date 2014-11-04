#pragma once
#include <QtOpenGL\QGLWidget>
class Preview : public QGLWidget
{
	Q_OBJECT;
protected:
	void initializeGL();
signals:
void initialized();
};