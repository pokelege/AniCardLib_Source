#pragma once
#pragma warning(disable:4127)
#pragma warning(disable:4201)
#include <QtOpenGL\QGLWidget>
#include <glm.hpp>
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
	glm::mat4 transform;
	glm::mat4 identity;
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

