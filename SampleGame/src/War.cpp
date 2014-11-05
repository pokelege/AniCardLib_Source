#include <War.h>
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
#include <DebugMemory.h>
War::War() :cameraSource(0)
{
	texture = 1;
	maxFails = 100;
}
War::~War()
{
	CommonGraphicsCommands::destroyGlobalGraphics();
	GameObjectManager::globalGameObjectManager.destroy();
	delete animation;
	delete animation2;
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
		std::string vert = FileReader( "assets/shaders/SelfIllumDiffuseVertex.glsl" );
		std::string frag = FileReader( "assets/shaders/SelfIllumDiffuseFragmentCamera.glsl" );
		shader = GraphicsShaderManager::globalShaderManager.createShaderInfo( vert.c_str() , frag.c_str() , &errors );
		std::cout << errors.c_str() << std::endl;
	}

	ShaderInfo* shader2;
	{
		std::string errors;
		std::string vert = FileReader( "assets/shaders/SelfIllumDiffuseVertex.glsl" );
		std::string frag = FileReader( "assets/shaders/SelfIllumDiffuseFragment.glsl" );
		shader2 = GraphicsShaderManager::globalShaderManager.createShaderInfo( vert.c_str() , frag.c_str() , &errors );
		std::cout << errors.c_str() << std::endl;
	}

	GeometryInfo* geometry = GraphicsGeometryManager::globalGeometryManager.addPMDGeometry( "assets/models/plane.pmd" , GraphicsBufferManager::globalBufferManager );
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

	renderable1 = GraphicsRenderingManager::globalRenderingManager.addRenderable();
	renderable1->initialize( 10 , 1 );
	renderable1->sharedUniforms = &GraphicsSharedUniformManager::globalSharedUniformManager;
	renderable1->shaderInfo = shader2;
	renderable1->alphaBlendingEnabled = false;
	renderable1->culling = CT_NONE;
	//model1Renderable->setRenderableUniform( "extraModelToWorld" , PT_MAT4 , &transform , 1 );
	animation = new AnimationRenderingInfo;
	
	player1 = GameObjectManager::globalGameObjectManager.addGameObject();
	player1->addComponent( renderable1 );
	player1->addComponent( animation );
	player1->rotate = glm::vec3( 90 , 0 , 0 );
	player1->active = false;

	renderable2 = GraphicsRenderingManager::globalRenderingManager.addRenderable();
	renderable2->initialize( 10 , 1 );
	renderable2->sharedUniforms = &GraphicsSharedUniformManager::globalSharedUniformManager;
	renderable2->shaderInfo = shader2;
	renderable2->alphaBlendingEnabled = false;
	renderable2->culling = CT_NONE;
	//model1Renderable->setRenderableUniform( "extraModelToWorld" , PT_MAT4 , &transform , 1 );
	animation2 = new AnimationRenderingInfo;

	player2 = GameObjectManager::globalGameObjectManager.addGameObject();
	player2->addComponent(renderable2 );
	player2->addComponent( animation2 );
	player2->rotate = glm::vec3( 90 , 0 , 0 );
	player2->active = false;

	
	MarkerPack::global.load( "assets/cardPack.aclf" );
	for ( unsigned int i = 0; i < MarkerPack::global.getGeometryListSize(); ++i )
	{
		GeometryInfo* geoLoad = MarkerPack::global.getGeometry( i );
		if ( !geoLoad ) continue;
		geoLoad->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
		geoLoad->addShaderStreamedParameter( 3 , PT_VEC2 , VertexInfo::STRIDE , VertexInfo::UV_OFFSET );
		geoLoad->addShaderStreamedParameter( 6 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGINDEX_OFFSET );
		geoLoad->addShaderStreamedParameter( 7 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGWEIGHT_OFFSET );
	}
	AudioController::globalAudioController.initialize();
	AudioController::globalAudioController.playSound( "assets/audio/music.mp3" , true );
	timer = new QTimer();
	connect( timer , SIGNAL( timeout() ) , this , SLOT( update() ) );
	timer->start( 0 );
}

void War::update()
{
	cameraSource->update();
	WindowInfo::width = width();
	WindowInfo::height = height();
	Clock::update();
	if ( cameraSource )
	{
		ARMarkerDetector::global.findCard( cameraSource->fetcher, &MarkerPack::global);
	}
	std::vector<FoundMarkerInfo>* list = 0;
	if ( ARMarkerDetector::global.getMarkerFound( &list ) )
	{
		if ( list && list->size() )
		{
			player1Fails = 0;
			//transform = list->at( 0 );
			//transform *= glm::inverse( transform );
			glm::vec3 characterPos( ( list->at( 0 ).center.x * plane->scale.x ) / 2 , ( list->at( 0 ).center.y * plane->scale.y ) / 2 , 0 );
			//diamond->translate = characterPos;
			player1->translate = characterPos;
			renderable1->geometryInfo = MarkerPack::global.getCardGeometry( list->at( 0 ).cardIndex );
			renderable1->swapTexture( MarkerPack::global.getCardTexture( list->at( 0 ).cardIndex ) , 0 );
			player1->active = true;

			if ( list->size() < 2 )
			{
				if ( player2->active && player2Fails < maxFails ) ++player2Fails;
			}
			else
			{
				player2Fails = 0;
				//transform = list->at( 0 );
				//transform *= glm::inverse( transform );
				glm::vec3 characterPos2( ( list->at( 1 ).center.x * plane->scale.x ) / 2 , ( list->at( 1 ).center.y * plane->scale.y ) / 2 , 0 );
				//diamond->translate = characterPos;
				player2->translate = characterPos;
				renderable2->geometryInfo = MarkerPack::global.getCardGeometry( list->at( 1 ).cardIndex );
				renderable2->swapTexture( MarkerPack::global.getCardTexture( list->at( 1 ).cardIndex ) , 0 );
				player2->active = true;
			}
		}
		else
		{
			if ( player1->active && player1Fails < maxFails ) ++player1Fails;
			if ( player2->active && player2Fails < maxFails ) ++player2Fails;
		}
		if ( player1->active && player1Fails >= maxFails )
		{
			player1->active = false;
		}
		if ( player2->active && player2Fails >= maxFails )
		{
			player2->active = false;
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