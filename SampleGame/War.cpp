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
#include <Graphics\AnimationRenderingInfo.h>
#include <Graphics\GraphicsTextureManager.h>
#include <Audio\AudioController.h>
#include <ARMarkerDetector.h>
#include <MarkerPack.h>
#include "DebugMemory.h"
War::War() :cameraSource(0)
{
	texture = 1;
}
War::~War()
{
	CommonGraphicsCommands::destroyGlobalGraphics();
	GameObjectManager::globalGameObjectManager.destroy();
	delete model1Animation;
	delete timer;
	delete cameraSource;
	AudioController::globalAudioController.destroy();
}
void War::initializeGL()
{
	CommonGraphicsCommands::initializeGlobalGraphics();
	GameObjectManager::globalGameObjectManager.initialize();
	ShaderInfo* shader;
	{
		std::string errors;
		std::string vert = FileReader( "Shaders/SelfIllumDiffuseVertex.glsl" );
		std::string frag = FileReader( "Shaders/SelfIllumDiffuseFragmentCamera.glsl" );
		shader = GraphicsShaderManager::globalShaderManager.createShaderInfo( vert.c_str() , frag.c_str() , &errors );
		std::cout << errors.c_str() << std::endl;
	}

	ShaderInfo* shader2;
	{
		std::string errors;
		std::string vert = FileReader( "Shaders/SelfIllumDiffuseVertex.glsl" );
		std::string frag = FileReader( "Shaders/SelfIllumDiffuseFragment.glsl" );
		shader2 = GraphicsShaderManager::globalShaderManager.createShaderInfo( vert.c_str() , frag.c_str() , &errors );
		std::cout << errors.c_str() << std::endl;
	}

	GeometryInfo* geometry = GraphicsGeometryManager::globalGeometryManager.addPMDGeometry( "Models/plane.pmd" , GraphicsBufferManager::globalBufferManager );
	geometry->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
	geometry->addShaderStreamedParameter( 3 , PT_VEC2 , VertexInfo::STRIDE , VertexInfo::UV_OFFSET );
	geometry->addShaderStreamedParameter( 6 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGINDEX_OFFSET );
	geometry->addShaderStreamedParameter( 7 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGWEIGHT_OFFSET );
	planeTexture = GraphicsTextureManager::globalTextureManager.addTexture( 0,0,0,0 );
	planeDebugTexture = GraphicsTextureManager::globalTextureManager.addTexture( 0 , 0 , 0 , 1 );;
	Renderable* renderable = GraphicsRenderingManager::globalRenderingManager.addRenderable();
	renderable->initialize( 10 , 2 );
	renderable->sharedUniforms = &GraphicsSharedUniformManager::globalSharedUniformManager;
	renderable->geometryInfo = geometry;
	renderable->shaderInfo = shader;
	renderable->alphaBlendingEnabled = false;
	renderable->culling = CT_NONE;
	renderable->addTexture( planeTexture );
	renderable->addTexture( planeDebugTexture );
	renderable->setRenderableUniform( "debug" , PT_INT , &texture );
	plane = GameObjectManager::globalGameObjectManager.addGameObject();
	plane->addComponent( renderable );
	plane->scale = glm::vec3( 10 , 10 , 10 );
	//renderable->setRenderableUniform( "extraModelToWorld" , PT_MAT4 , &identity , 1 );
	Camera* camera = GraphicsCameraManager::globalCameraManager.addCamera();
	camera->initializeRenderManagers();
	camera->addRenderList( &GraphicsRenderingManager::globalRenderingManager );
	camera->FOV = 60.0f;
	camera->nearestObject = 0.01f;
	GameObject* player = GameObjectManager::globalGameObjectManager.addGameObject();
	player->translate = glm::vec3( 0 , 0 , 25 );
	camera->direction = glm::vec3( 0 , 0 , -1 );
	player->addComponent( camera );

	GeometryInfo* diamond = GraphicsGeometryManager::globalGeometryManager.addPMDGeometry( "Models/diamond.pmd" , GraphicsBufferManager::globalBufferManager );
	diamond->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
	diamond->addShaderStreamedParameter( 3 , PT_VEC2 , VertexInfo::STRIDE , VertexInfo::UV_OFFSET );
	diamond->addShaderStreamedParameter( 6 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGINDEX_OFFSET );
	diamond->addShaderStreamedParameter( 7 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGWEIGHT_OFFSET );

	TextureInfo* diamondTexture = GraphicsTextureManager::globalTextureManager.addTexture( "Textures/diamond.png" );

	Renderable* model1Renderable = GraphicsRenderingManager::globalRenderingManager.addRenderable();
	model1Renderable->initialize( 10 , 1 );
	model1Renderable->sharedUniforms = &GraphicsSharedUniformManager::globalSharedUniformManager;
	model1Renderable->geometryInfo = diamond;
	model1Renderable->shaderInfo = shader2;
	model1Renderable->alphaBlendingEnabled = false;
	model1Renderable->culling = CT_NONE;
	model1Renderable->addTexture( diamondTexture );
	model1Renderable->setRenderableUniform( "extraModelToWorld" , PT_MAT4 , &transform , 1 );
	model1Animation = new AnimationRenderingInfo;
	

	AudioController::globalAudioController.initialize();
	//AudioController::globalAudioController.playSound( "Audio/music.mp3" , true );
	model1 = GameObjectManager::globalGameObjectManager.addGameObject();
	model1->addComponent( model1Renderable );
	model1->addComponent( model1Animation );
	//model1->rotate = glm::vec3( 90 , 90, 0 );
	model1->scale = glm::vec3( 0.1f , 0.1f , 0.1f );
	model1->active = false;
	
	MarkerPack::global.addMarker( "Textures/Cards/AS.png" );
	MarkerPack::global.addMarker( "Textures/Cards/AD.png" );
	timer = new QTimer();
	connect( timer , SIGNAL( timeout() ) , this , SLOT( update() ) );
	timer->start( 0 );
}
static int fails = 0;
static int maxFails = 100;
void War::update()
{
	WindowInfo::width = width();
	WindowInfo::height = height();
	Clock::update();
	if ( cameraSource )
	{
		ARMarkerDetector::global.findCard( cameraSource->fetcher );
	}
	std::vector<glm::mat4>* list = 0;
	if ( ARMarkerDetector::global.getMatrices( &list ) )
	{
		if ( list && list->size() )
		{
			fails = 0;
			transform = list->at( 0 );
			//transform *= glm::inverse( transform );
			//glm::vec3 characterPos( (list->at( 0 ).x * plane->scale.x)  / 2, (list->at( 0 ).y * plane->scale.y) / 2 , 0 );
			//model1->translate = characterPos;
			model1->active = true;
		}
		else if(model1->active) fails++;
		if ( model1->active && fails >= maxFails )
		{
			model1->active = false;
		}
		ARMarkerDetector::global.finishedUsingMat();
	}
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
		unsigned char* pictureData = 0;
		long width;
		long height;
		if ( cameraSource->fetcher->getPicture(&pictureData, &width, &height) )
		{
			GraphicsTextureManager::globalTextureManager.editTexture( planeTexture , ( char* ) pictureData, width , height , 0 );
			plane->scale = 0.025f * glm::vec3( width , height , 1 );
			cameraSource->fetcher->finishedUsing();
		}
	}

	unsigned char* pictureData = 0;
	long width;
	long height;
	if ( MarkerPack::global.getPicture( &pictureData , &width , &height ) )
	{
		GraphicsTextureManager::globalTextureManager.editTexture( planeDebugTexture , ( char* ) pictureData , width , height , 1, GL_RGB );
		MarkerPack::global.finishedUsing();
	}

	GraphicsCameraManager::globalCameraManager.drawAllCameras();
}
void War::setCameraSource(WebCamSource* webcam)
{
	cameraSource = webcam;
}