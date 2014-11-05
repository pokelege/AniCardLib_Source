#pragma once
#include <QtOpenGL\QGLWidget>
#include <glm.hpp>
#include <Graphics\GraphicsRenderingManager.h>
#include <Graphics\GraphicsCameraManager.h>
#include <QtCore\QTimer>
struct TextureInfo;
class GameObject;
struct RenderableInfo;
struct GeometryInfo;
class Preview : public QGLWidget
{
	Q_OBJECT;
	QTimer updateTimer;
	glm::vec2 cardScale;
	TextureInfo* cardTexture;
	GameObject* cardPlane;
	RenderableInfo* modelRenderable;
	GraphicsRenderingManager cardRenderer;
	GraphicsCameraManager cameras;
	bool drawing,changingCard;
private slots:
	void update();
	void paintGL();

protected:
	void initializeGL();
signals:
void initialized();

public:
	void setCard( const unsigned char* cardImage , const unsigned int& width , const unsigned int& height, GeometryInfo* cardGeo = 0, TextureInfo* cardModelTexture = 0 );
};