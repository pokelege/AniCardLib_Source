#pragma once
#pragma warning(disable:4127)
#pragma warning(disable:4201)
#include <QtOpenGL\QGLWidget>
#include <glm.hpp>
#include <ARMarkerDetector.h>
class WebCamSource;
struct TextureInfo;
class GameObject;
struct AnimationRenderingInfo;
class QTimer;
struct RenderableInfo;
enum AnimationState {None, ToFight, EndFight};
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
	FoundMarkerInfo marker1 , marker2;
	glm::vec3 player1OldPos , player2OldPos;
	AnimationState aniState;
	float lerp;
	float speed;
	bool animating;
	void mouseMoveEvent( QMouseEvent* e );
	int maxFails;
	int player1Fails , player2Fails;
	glm::mat4 transform;
	glm::mat4 identity;
	int texture;
	void animationUpdate();
	bool findMarkers();
public:
	War();
	~War();
	void initializeGL();
	void paintGL();
	void setCameraSource( WebCamSource* cameraSource );
public slots:
	void update();
};

