#pragma once
#pragma warning(disable:4127)
#include <QtOpenGL\QGLWidget>

class WebCamSource;
struct TextureInfo;
class GameObject;
struct AnimationRenderingInfo;
class QTimer;
class War : public QGLWidget
{
	Q_OBJECT;

	WebCamSource* cameraSource;
	TextureInfo* planeTexture, *planeDebugTexture;
	GameObject* plane;
	AnimationRenderingInfo* model1Animation;
	QTimer* timer;
	GameObject* model1;
	int texture;
public:
	War();
	~War();
	void initializeGL();
	void paintGL();
	void setCameraSource( WebCamSource* cameraSource );
public slots:
	void update();
};

