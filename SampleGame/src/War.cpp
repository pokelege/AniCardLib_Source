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
#include <Marker.h>
#include <Input\FirstPersonCameraInput.h>
#include <gtc\matrix_transform.hpp>
#include <gtx\quaternion.hpp>
War::War() :cameraSource(0) , animating(false) , lerp(0) , speed(1)
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
	player->addComponent( camera );
	FirstPersonCameraInput* fpsInput = new FirstPersonCameraInput;
	fpsInput->moveSensitivity = 1;
	fpsInput->rotationSensitivity = 0.1f;
	player->addComponent( fpsInput );


	renderable1 = GraphicsRenderingManager::globalRenderingManager.addRenderable();
	renderable1->initialize( 10 , 1 );
	renderable1->sharedUniforms = &GraphicsSharedUniformManager::globalSharedUniformManager;
	renderable1->shaderInfo = shader2;
	renderable1->alphaBlendingEnabled = false;
	renderable1->culling = CT_NONE;
	//model1Renderable->setRenderableUniform( "extraModelToWorld" , PT_MAT4 , &transform , 1 );
	animation = new AnimationRenderingInfo;
	animation->animationFrameRate = 60;
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
	animation2->animationFrameRate = 60;
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

	MouseInput::globalMouseInput.updateOldMousePosition = false;

	setMouseTracking( true );
	QCursor c = cursor();
	c.setPos( mapToGlobal( QPoint( width() / 2 , height() / 2 ) ) );
	c.setShape( Qt::BlankCursor );
	setCursor( c );
	MouseInput::globalMouseInput.updateMousePosition( glm::vec2( width() / 2 , height() / 2 ) );

	AudioController::globalAudioController.initialize();
	AudioController::globalAudioController.playSound( "assets/audio/music.mp3" , true );
	timer = new QTimer();
	connect( timer , SIGNAL( timeout() ) , this , SLOT( update() ) );
	timer->start( 0 );
}

void War::mouseMoveEvent( QMouseEvent* e )
{
	e;
	QCursor c = cursor();
	QPoint mapFromGlobal = this->mapFromGlobal( c.pos() );
	if ( GetAsyncKeyState( VK_LBUTTON ) < 0 && mapFromGlobal.x() >= 0 && mapFromGlobal.x() <= width() &&
		 mapFromGlobal.y() >= 0 && mapFromGlobal.y() <= height() && MouseInput::globalMouseInput.oldMousePosition != glm::vec2( c.pos().x() , c.pos().y() ) )
	{
		if ( !MouseInput::globalMouseInput.getDeltaTracking() )MouseInput::globalMouseInput.setDeltaTracking( true );
		glm::vec2 oldPos = MouseInput::globalMouseInput.oldMousePosition;
		MouseInput::globalMouseInput.updateMousePosition( glm::vec2( c.pos().x() , c.pos().y() ) );

		c.setPos( QPoint( oldPos.x , oldPos.y ) );
		c.setShape( Qt::BlankCursor );
	}
	else if ( MouseInput::globalMouseInput.oldMousePosition != glm::vec2( c.pos().x() , c.pos().y() ) )
	{
		c.setShape( Qt::ArrowCursor );
		if ( MouseInput::globalMouseInput.getDeltaTracking() )MouseInput::globalMouseInput.setDeltaTracking( false );
		MouseInput::globalMouseInput.oldMousePosition = glm::vec2( c.pos().x() , c.pos().y() );
		MouseInput::globalMouseInput.updateMousePosition( glm::vec2( c.pos().x() , c.pos().y() ) );
	}
	clearFocus();
	setCursor( c );
	setFocus();
}

void War::update()
{
	//std::cout << player1->active << std::endl;
	//std::cout << MarkerPack::global.getMarker( 0 )->width << std::endl;
	cameraSource->update();
	WindowInfo::width = width();
	WindowInfo::height = height();
	Clock::update();

	if ( !animating && findMarkers() )
	{
		animating = true;
	}
	if ( animating )
	{
		animationUpdate();
	}
	GameObjectManager::globalGameObjectManager.earlyUpdateParents();
	GameObjectManager::globalGameObjectManager.updateParents();
	GameObjectManager::globalGameObjectManager.lateUpdateParents();
	MouseInput::globalMouseInput.mouseDelta = glm::vec2();
	repaint();
}

void War::animationUpdate()
{
	glm::vec2 center = 0.5f * (marker1.center + marker2.center);
	
	glm::vec3 worldCenterPos( (center.x * plane->scale.x ) / 2 , ( center.y * plane->scale.y ) / 2 , 10 );
	switch ( aniState )
	{
		case ToFight:
			lerp += speed * Clock::dt;
			if ( lerp >= 1 )
			{
				lerp = 1;
				aniState = EndFight;
				if ( marker1.cardIndex % 13 > marker2.cardIndex % 13 )
				{
					animation->play( 0 );
					animation2->play( 2 );
				}
				else if ( marker1.cardIndex % 13 < marker2.cardIndex % 13 )
				{
					animation->play( 2 );
					animation2->play( 0 );
				}
				else
				{
					animation->play( 2 );
					animation2->play( 2 );
				}
			}
			player1->translate = glm::mix( player1OldPos , worldCenterPos , lerp );
			player2->translate = glm::mix( player2OldPos , worldCenterPos , lerp );
			break;
		case EndFight:
			lerp -= speed * Clock::dt;
			if ( lerp <= 0 )
			{
				lerp = 0;
				aniState = None;
				animation->play( 0 );
				animation2->play( 0 );
				animating = false;
				player1->active = false;
				player2->active = false;
			}
			player1->translate = glm::mix( player1OldPos , worldCenterPos , lerp );
			player2->translate = glm::mix( player2OldPos , worldCenterPos , lerp );
			break;
		default:
			aniState = ToFight;
			animation->play( 1 );
			animation2->play( 1 );
			player1->rotate = glm::eulerAngles(glm::quat_cast(glm::lookAt( player1->translate , worldCenterPos - glm::vec3(0,0,10) , glm::vec3( 0 , 0 , -1 ))));
			player2->rotate = glm::eulerAngles( glm::quat_cast( glm::lookAt( player2->translate , worldCenterPos - glm::vec3( 0 , 0 , 10 ) , glm::vec3( 0 , 0 , -1 ) ) ) );
			break;
	}
}

bool War::findMarkers()
{
	if ( cameraSource )
	{
		ARMarkerDetector::global.findCard( cameraSource->fetcher , &MarkerPack::global );
	}
	std::vector<FoundMarkerInfo>* list = 0;
	if ( ARMarkerDetector::global.getMarkerFound( &list ) )
	{
		if ( list && list->size() )
		{
			//std::cout << list->size() << std::endl;
			player1Fails = 0;
			//transform = list->at( 0 );
			//transform *= glm::inverse( transform );
			glm::vec3 characterPos( ( list->at( 0 ).center.x * plane->scale.x ) / 2 , ( list->at( 0 ).center.y * plane->scale.y ) / 2 , 0 );
			//diamond->translate = characterPos;
			player1->translate = characterPos;
			std::cout <<list->at( 0 ).cardIndex << std::endl;
			std::cout << list->at( 0 ).dissimilarity << std::endl;
			std::cout << list->size() << std::endl;
			renderable1->geometryInfo = MarkerPack::global.getCardGeometry( list->at( 0 ).cardIndex );
			renderable1->swapTexture( MarkerPack::global.getCardTexture( list->at( 0 ).cardIndex ) , 0 );
			player1OldPos = characterPos;
			marker1 = list->at( 0 );
			player1->active = true;
			//std::cout << list->at( 0 ).cardIndex << std::endl;
			if ( list->size() < 2 )
			{
				if ( player2->active && player2Fails < maxFails ) ++player2Fails;
			}
			else
			{
				for ( unsigned int i = 1; i < list->size(); ++i )
				{
					glm::vec3 characterPos2( ( list->at( i ).center.x * plane->scale.x ) / 2 , ( list->at( i ).center.y * plane->scale.y ) / 2 , 0 );
					if ( glm::length( characterPos - characterPos2 ) > 5 )
					{
						player2Fails = 0;
						//transform = list->at( 0 );
						//transform *= glm::inverse( transform );
						glm::vec3 characterPos2( ( list->at( i ).center.x * plane->scale.x ) / 2 , ( list->at( i ).center.y * plane->scale.y ) / 2 , 0 );
						//diamond->translate = characterPos;
						player2->translate = characterPos2;
						renderable2->geometryInfo = MarkerPack::global.getCardGeometry( list->at( i ).cardIndex );
						renderable2->swapTexture( MarkerPack::global.getCardTexture( list->at( i ).cardIndex ) , 0 );
						player2OldPos = characterPos2;
						marker2 = list->at( i );
						player2->active = true;
						break;
					}
				}
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
	return player1->active && player2->active;
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

	unsigned char* pictureData = 0;
	long width;
	long height;
	if ( ARMarkerDetector::global.getPicture( &pictureData , &width , &height ) )
	{
		GraphicsTextureManager::globalTextureManager.editTexture( planeDebugTexture , ( char* ) pictureData , width , height , 1, GL_RGB );
		ARMarkerDetector::global.finishedUsing();
	}

	GraphicsCameraManager::globalCameraManager.drawAllCameras();
}
void War::setCameraSource(WebCamSource* webcam)
{
	cameraSource = webcam;
}