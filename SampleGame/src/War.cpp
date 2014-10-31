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
	delete diamondAnimation;
	delete spadeAnimation;
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

	GeometryInfo* diamondGeo = GraphicsGeometryManager::globalGeometryManager.addPMDGeometry( "Models/diamond.pmd" , GraphicsBufferManager::globalBufferManager );
	diamondGeo->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
	diamondGeo->addShaderStreamedParameter( 3 , PT_VEC2 , VertexInfo::STRIDE , VertexInfo::UV_OFFSET );
	diamondGeo->addShaderStreamedParameter( 6 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGINDEX_OFFSET );
	diamondGeo->addShaderStreamedParameter( 7 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGWEIGHT_OFFSET );

	TextureInfo* diamondTexture = GraphicsTextureManager::globalTextureManager.addTexture( "Textures/diamond.png" );

	Renderable* diamondRenderable = GraphicsRenderingManager::globalRenderingManager.addRenderable();
	diamondRenderable->initialize( 10 , 1 );
	diamondRenderable->sharedUniforms = &GraphicsSharedUniformManager::globalSharedUniformManager;
	diamondRenderable->geometryInfo = diamondGeo;
	diamondRenderable->shaderInfo = shader2;
	diamondRenderable->alphaBlendingEnabled = false;
	diamondRenderable->culling = CT_NONE;
	diamondRenderable->addTexture( diamondTexture );
	//model1Renderable->setRenderableUniform( "extraModelToWorld" , PT_MAT4 , &transform , 1 );
	diamondAnimation = new AnimationRenderingInfo;
	
	diamond = GameObjectManager::globalGameObjectManager.addGameObject();
	diamond->addComponent( diamondRenderable );
	diamond->addComponent( diamondAnimation );
	diamond->rotate = glm::vec3( 90 , 0 , 0 );
	diamond->active = false;

	GeometryInfo* spadeGeo = GraphicsGeometryManager::globalGeometryManager.addPMDGeometry( "Models/spade.pmd" , GraphicsBufferManager::globalBufferManager );
	spadeGeo->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
	spadeGeo->addShaderStreamedParameter( 3 , PT_VEC2 , VertexInfo::STRIDE , VertexInfo::UV_OFFSET );
	spadeGeo->addShaderStreamedParameter( 6 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGINDEX_OFFSET );
	spadeGeo->addShaderStreamedParameter( 7 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGWEIGHT_OFFSET );

	TextureInfo* spadeTexture = GraphicsTextureManager::globalTextureManager.addTexture( "Textures/spade.png" );

	Renderable* spadeRenderable = GraphicsRenderingManager::globalRenderingManager.addRenderable();
	spadeRenderable->initialize( 10 , 1 );
	spadeRenderable->sharedUniforms = &GraphicsSharedUniformManager::globalSharedUniformManager;
	spadeRenderable->geometryInfo = spadeGeo;
	spadeRenderable->shaderInfo = shader2;
	spadeRenderable->alphaBlendingEnabled = false;
	spadeRenderable->culling = CT_NONE;
	spadeRenderable->addTexture( spadeTexture );
	//model1Renderable->setRenderableUniform( "extraModelToWorld" , PT_MAT4 , &transform , 1 );
	spadeAnimation = new AnimationRenderingInfo;

	spade = GameObjectManager::globalGameObjectManager.addGameObject();
	spade->addComponent( spadeRenderable );
	spade->addComponent( spadeAnimation );
	spade->rotate = glm::vec3( 90 , 0 , 0 );
	spade->active = false;

	
	MarkerPack::global.addMarker( "Textures/Cards/AS.png" );
	MarkerPack::global.addMarker( "Textures/Cards/AD.png" );

	AudioController::globalAudioController.initialize();
	AudioController::globalAudioController.playSound( "Audio/music.mp3" , true );
	timer = new QTimer();
	connect( timer , SIGNAL( timeout() ) , this , SLOT( update() ) );
	timer->start( 0 );
}
static int fails = 0;
static int maxFails = 100;
void War::update()
{
	cameraSource->update();
	WindowInfo::width = width();
	WindowInfo::height = height();
	Clock::update();
	if ( cameraSource )
	{
		ARMarkerDetector::global.findCard( cameraSource->fetcher );
	}
	std::vector<FoundMarkerInfo>* list = 0;
	if ( ARMarkerDetector::global.getMarkerFound( &list ) )
	{
		if ( list && list->size() )
		{
			fails = 0;
			//transform = list->at( 0 );
			//transform *= glm::inverse( transform );
			glm::vec3 characterPos( ( list->at( 0 ).center.x * plane->scale.x ) / 2 , ( list->at( 0 ).center.y * plane->scale.y ) / 2 , 0 );
			//diamond->translate = characterPos;
			diamond->active = false;
			spade->active = false;
			//std::cout << list->size() << std::endl;
			switch ( (int)list->at(0).cardIndex )
			{
				case (0):
					spade->translate = characterPos;
					spade->active = true;
					break;
				case (1):
					diamond->translate = characterPos;
					diamond->active = true;
					break;
			}
		}
		else if ( diamond->active || spade->active ) fails++;
		if ( (diamond->active || spade->active) && fails >= maxFails )
		{
			diamond->active = false;
			spade->active = false;
		}
		ARMarkerDetector::global.finishedUsingMarkerFound();
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
			float toSize = std::min( (float)width , (float)height );
			toSize = 20.0f / toSize;
			//std::cout << toSize * width << std::endl;
			plane->scale = glm::vec3( toSize * width , toSize * height , 1 );
			cameraSource->fetcher->finishedUsing();
		}
	}

	//unsigned char* pictureData = 0;
	//long width;
	//long height;
	//if ( ARMarkerDetector::global.getPicture( &pictureData , &width , &height ) )
	//{
	//	GraphicsTextureManager::globalTextureManager.editTexture( planeDebugTexture , ( char* ) pictureData , width , height , 1, GL_RGB );
	//	ARMarkerDetector::global.finishedUsing();
	//}

	GraphicsCameraManager::globalCameraManager.drawAllCameras();
}
void War::setCameraSource(WebCamSource* webcam)
{
	cameraSource = webcam;
}