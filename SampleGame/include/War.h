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
struct RenderableInfo;
class War : public QGLWidget
{
	Q_OBJECT;

	WebCamSource* cameraSource;
	TextureInfo* planeTexture, *planeDebugTexture;
	GameObject* plane;
	AnimationRenderingInfo* animation, *animation2;
	QTimer* timer;
	GameObject* player1, *player2;
	RenderableInfo* renderable1, *renderable2;
	void mouseMoveEvent( QMouseEvent* e );
	int maxFails;
	int player1Fails , player2Fails;
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

