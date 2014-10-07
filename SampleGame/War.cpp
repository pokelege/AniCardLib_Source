#include "War.h"
#include <Graphics\CommonGraphicsCommands.h>
#include <Core\GameObjectManger.h>
#include <Graphics\GraphicsCameraManager.h>
#include <Core\WindowInfo.h>
#include <Misc\Clock.h>
#include <Input\MouseInput.h>
#include <iostream>
#include <Graphics\ShaderInfo.h>
#include <Graphics\GraphicsShaderManager.h>
#include <Misc\FileReader.h>
#include <Graphics\GeometryInfo.h>
#include <Graphics\GraphicsGeometryManager.h>
#include <Graphics\VertexInfo.h>
#include <Graphics\GraphicsBufferManager.h>
#include <Graphics\GraphicsRenderingManager.h>
#include <Graphics\RenderableInfo.h>
#include <Graphics\GraphicsSharedUniformManager.h>
#include <Graphics\GraphicsTextureManager.h>
#include <Core\GameObject.h>
#include <Graphics\Camera.h>
#include <WebCamSource.h>
#include <Qt\qtimer.h>
War::War() :cameraSource(0)
{
	
}
void War::initializeGL()
{
	CommonGraphicsCommands::initializeGlobalGraphics();
	GameObjectManager::globalGameObjectManager.initialize();

	std::string errors;
	std::string vert = FileReader( "Shaders/SelfIllumDiffuseVertex.glsl" );
	std::string frag = FileReader( "Shaders/SelfIllumDiffuseFragment.glsl" );
	ShaderInfo* shader = GraphicsShaderManager::globalShaderManager.createShaderInfo( vert.c_str() , frag.c_str(), &errors );
	std::cout << errors.c_str() << std::endl;

	GeometryInfo* geometry = GraphicsGeometryManager::globalGeometryManager.addPMDGeometry( "Models/plane.pmd" , GraphicsBufferManager::globalBufferManager );
	geometry->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
	geometry->addShaderStreamedParameter( 3 , PT_VEC2 , VertexInfo::STRIDE , VertexInfo::UV_OFFSET );

	planeTexture = GraphicsTextureManager::globalTextureManager.addTexture( 0,0,0,0 );

	Renderable* renderable = GraphicsRenderingManager::globalRenderingManager.addRenderable();
	renderable->initialize( 10 , 1 );
	renderable->sharedUniforms = &GraphicsSharedUniformManager::globalSharedUniformManager;
	renderable->geometryInfo = geometry;
	renderable->shaderInfo = shader;
	renderable->alphaBlendingEnabled = false;
	renderable->culling = CT_NONE;
	renderable->addTexture( planeTexture );

	plane = GameObjectManager::globalGameObjectManager.addGameObject();
	plane->addComponent( renderable );
	plane->scale = glm::vec3( 10 , 10 , 10 );

	Camera* camera = GraphicsCameraManager::globalCameraManager.addCamera();
	camera->initializeRenderManagers();
	camera->addRenderList( &GraphicsRenderingManager::globalRenderingManager );
	camera->FOV = 60.0f;
	camera->nearestObject = 0.01f;
	GameObject* player = GameObjectManager::globalGameObjectManager.addGameObject();
	player->translate = glm::vec3( 0 , 0 , 10 );
	camera->direction = glm::vec3( 0 , 0 , -1 );
	player->addComponent( camera );

	//texture = 0;

	//GraphicsSharedUniformManager::globalSharedUniformManager.setSharedUniform()

	QTimer* timer = new QTimer();
	connect( timer , SIGNAL( timeout() ) , this , SLOT( update() ) );
	timer->start( 0 );
}

void War::update()
{
	WindowInfo::width = width();
	WindowInfo::height = height();
	Clock::update();
	QPoint point = cursor().pos();
	MouseInput::globalMouseInput.updateMousePosition( glm::vec2( point.x() , point.y() ) );
	GameObjectManager::globalGameObjectManager.earlyUpdateParents();
	GameObjectManager::globalGameObjectManager.updateParents();
	GameObjectManager::globalGameObjectManager.lateUpdateParents();
	repaint();
}


void War::paintGL()
{
	if ( cameraSource )
	{
		if ( cameraSource->fetcher->canGrab )
		{
			GraphicsTextureManager::globalTextureManager.editTexture( planeTexture , ( char* ) cameraSource->fetcher->picture , cameraSource->fetcher->width , cameraSource->fetcher->height , 0 );
			plane->scale = 0.01f * glm::vec3( cameraSource->fetcher->width , cameraSource->fetcher->height , 1 );
			delete[] cameraSource->fetcher->picture;
			cameraSource->fetcher->picture = 0;
			cameraSource->fetcher->canGrab = false;
		}
	}
	GraphicsCameraManager::globalCameraManager.drawAllCameras();
}
void War::setCameraSource(WebCamSource* webcam)
{
	cameraSource = webcam;
}