#pragma once
#include <QtOpenGL\QGLWidget>

class WebCamSource;
struct TextureInfo;
class GameObject;
class War : public QGLWidget
{
	Q_OBJECT;

	WebCamSource* cameraSource;
	TextureInfo* planeTexture;
	GameObject* plane;
	//int texture;
public:
	War();
	void initializeGL();
	void paintGL();
	void setCameraSource( WebCamSource* cameraSource );
public slots:
	void update();
};

