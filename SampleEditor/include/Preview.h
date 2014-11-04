#pragma once
#include <QtOpenGL\QGLWidget>
#include <glm.hpp>
#include <Graphics\GraphicsRenderingManager.h>
#include <QtCore\QTimer>
struct TextureInfo;
class GameObject;
class Preview : public QGLWidget
{
	Q_OBJECT;
	QTimer updateTimer;
	glm::vec2 cardScale;
	TextureInfo* cardTexture;
	GameObject* cardPlane;
	GraphicsRenderingManager cardRenderer;
	bool drawing,changingTexture;
private slots:
	void update();
	void paintGL();

protected:
	void initializeGL();
signals:
void initialized();

public:
	void setCard( unsigned char* cardImage , const unsigned int& width , const unsigned int& height );
};