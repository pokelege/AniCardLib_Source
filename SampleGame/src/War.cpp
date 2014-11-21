#define GLM_FORCE_RADIANS
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
#include <DebugMemory.h>
#include <Input\FirstPersonCameraInput.h>
#include <gtc\matrix_transform.hpp>
#include <gtx\quaternion.hpp>
#include <Misc\ExtraFunctions.h>
#include <Graphics\Light.h>
#include <Graphics\GraphicsLightManager.h>
#include <thread>
War::War() : animating(false) , lerp(0) , speed(1)
{
	texture = 1;
	maxFails = 100;
	landMax = 1;
	land = 0;
	aniCardLib = new AniCardLibCommonGame;
}
War::~War()
{
	CommonGraphicsCommands::destroyGlobalGraphics();
	GameObjectManager::globalGameObjectManager.destroy();
	delete animation;
	delete animation2;
	delete timer;
	delete aniCardLib;
	delete fpsInput;
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

	GeometryInfo* geometry = GraphicsGeometryManager::globalGeometryManager.addPMDGeometry( "assets/models/TableTop.pmd" , GraphicsBufferManager::globalBufferManager );
	geometry->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
	geometry->addShaderStreamedParameter( 2 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::NORMAL_OFFSET );
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
	renderable->culling = CT_BACK;
	renderable->addTexture( planeTexture );
	renderable->addTexture( planeDebugTexture );
	renderable->setRenderableUniform( "debug" , PT_INT , &texture );
	plane = GameObjectManager::globalGameObjectManager.addGameObject();
	plane->addComponent( renderable );
	GeometryInfo* tableGeo = GraphicsGeometryManager::globalGeometryManager.addPMDGeometry( "assets/models/TableBody.pmd",GraphicsBufferManager::globalBufferManager );
	tableGeo->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
	tableGeo->addShaderStreamedParameter( 2 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::NORMAL_OFFSET );
	tableGeo->addShaderStreamedParameter( 3 , PT_VEC2 , VertexInfo::STRIDE , VertexInfo::UV_OFFSET );
	tableGeo->addShaderStreamedParameter( 6 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGINDEX_OFFSET );
	tableGeo->addShaderStreamedParameter( 7 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGWEIGHT_OFFSET );
	Renderable* tableRenderable = GraphicsRenderingManager::globalRenderingManager.addRenderable();
	tableRenderable->initialize( 10 , 1 );
	tableRenderable->sharedUniforms = &GraphicsSharedUniformManager::globalSharedUniformManager;
	tableRenderable->geometryInfo = tableGeo;
	tableRenderable->shaderInfo = shader2;
	tableRenderable->alphaBlendingEnabled = false;
	tableRenderable->culling = CT_BACK;
	tableRenderable->addTexture( GraphicsTextureManager::globalTextureManager.addTexture( "assets/textures/table.tex" ) );
	GameObject* table = GameObjectManager::globalGameObjectManager.addGameObject();
	table->addComponent( tableRenderable );
	plane->addChild( table );


	GeometryInfo* sceneGeo = GraphicsGeometryManager::globalGeometryManager.addPMDGeometry( "assets/models/Scene.pmd" , GraphicsBufferManager::globalBufferManager );
	sceneGeo->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
	sceneGeo->addShaderStreamedParameter( 2 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::NORMAL_OFFSET );
	sceneGeo->addShaderStreamedParameter( 3 , PT_VEC2 , VertexInfo::STRIDE , VertexInfo::UV_OFFSET );
	sceneGeo->addShaderStreamedParameter( 6 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGINDEX_OFFSET );
	sceneGeo->addShaderStreamedParameter( 7 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGWEIGHT_OFFSET );
	Renderable* sceneRenderable = GraphicsRenderingManager::globalRenderingManager.addRenderable();
	sceneRenderable->initialize( 10 , 1 );
	sceneRenderable->sharedUniforms = &GraphicsSharedUniformManager::globalSharedUniformManager;
	sceneRenderable->geometryInfo = sceneGeo;
	sceneRenderable->shaderInfo = shader2;
	sceneRenderable->alphaBlendingEnabled = false;
	sceneRenderable->culling = CT_BACK;
	sceneRenderable->addTexture( GraphicsTextureManager::globalTextureManager.addTexture( "assets/textures/scene.tex" ) );
	GameObject* scene = GameObjectManager::globalGameObjectManager.addGameObject();
	scene->addComponent( sceneRenderable );


	camera = GraphicsCameraManager::globalCameraManager.addCamera();
	camera->initializeRenderManagers();
	camera->addRenderList( &GraphicsRenderingManager::globalRenderingManager );
	camera->FOV = 60.0f;
	camera->nearestObject = 0.01f;
	player = GameObjectManager::globalGameObjectManager.addGameObject();
	player->translate = plane->translate + glm::vec3(0,2,2);
	camera->direction = glm::normalize( plane->translate - player->translate );
	player->addComponent( camera );
	fpsInput = new FirstPersonCameraInput;
	fpsInput->moveSensitivity = 0.1f;
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
	player1->active = false;
	player1->scale = glm::vec3( 0.1f , 0.1f , 0.1f );

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
	player2->active = false;
	player2->scale = glm::vec3( 0.1f , 0.1f , 0.1f );
	
	aniCardLib->setMarkerPack( "assets/cardPack.aclf" );
	for ( unsigned int i = 0; i < aniCardLib->getGeometryListSize(); ++i )
	{
		GeometryInfo* geoLoad = aniCardLib->getGeometry( i );
		if ( !geoLoad ) continue;
		geoLoad->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
		geoLoad->addShaderStreamedParameter( 2 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::NORMAL_OFFSET );
		geoLoad->addShaderStreamedParameter( 3 , PT_VEC2 , VertexInfo::STRIDE , VertexInfo::UV_OFFSET );
		geoLoad->addShaderStreamedParameter( 6 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGINDEX_OFFSET );
		geoLoad->addShaderStreamedParameter( 7 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGWEIGHT_OFFSET );
	}

	GameObject* lightBulb = GameObjectManager::globalGameObjectManager.addGameObject();
	Light* light = GraphicsLightManager::global.addLight();
	light->setColor( glm::vec4( 1 , 1 , 1 , 1 ));
	lightBulb->addComponent( light );
	lightBulb->translate = glm::vec3( 0 , 5 , 5 );
	MouseInput::globalMouseInput.updateOldMousePosition = false;

	setMouseTracking( true );
	QCursor c = cursor();
	c.setPos( mapToGlobal( QPoint( width() / 2 , height() / 2 ) ) );
	c.setShape( Qt::BlankCursor );
	setCursor( c );
	MouseInput::globalMouseInput.updateMousePosition( glm::vec2( width() / 2 , height() / 2 ) );
	marker1.used = true;
	marker1.cardIndex = UINT_MAX;
	marker2.used = true;
	marker2.cardIndex = UINT_MAX;
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
	WindowInfo::width = width();
	WindowInfo::height = height();
	Clock::update();
	aniCardLib->update();
	if ( !animating )
	{
		player->translate = plane->translate + glm::vec3( 0 , 2 , 2 );
		camera->direction = glm::normalize( plane->translate - player->translate );
	}
	bool animate = findMarkers();
	if ( !animating && animate )
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
	
	glm::vec3 worldCenterPos( ( center.x * plane->scale.x ) / 2 , 1 , -(( center.y * plane->scale.y ) / 2) );
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
					animation->play( 3 );
					animation2->play( 2 );
				}
				else if ( marker1.cardIndex % 13 < marker2.cardIndex % 13 )
				{
					animation->play( 2 );
					animation2->play( 3 );
				}
				else
				{
					animation->play( 2 );
					animation2->play( 2 );
				}
			}
			player1->translate = glm::mix( player1OldPos , worldCenterPos , lerp );
			player2->translate = glm::mix( player2OldPos , worldCenterPos , lerp );
			player->translate = glm::mix( plane->translate + glm::vec3( 0 , 2 , 2 ), worldCenterPos + glm::vec3( 0 , 0 , 0.5f ),lerp );
			camera->direction = glm::mix( glm::normalize( plane->translate - player->translate ),
										  glm::normalize( glm::vec3( 0 , 0 , -0.5f )),
										  lerp);
			break;
		case EndFight:
			lerp -= speed * Clock::dt;
			if ( lerp <= 0 )
			{
				lerp = 0;
				aniState = Land;
				if ( marker1.cardIndex % 13 > marker2.cardIndex % 13 )
				{
					animation->play( 4 );
				}
				else if ( marker1.cardIndex % 13 < marker2.cardIndex % 13 )
				{
					animation2->play( 4 );
				}
				land = landMax;
			}
			player1->translate = glm::mix( player1OldPos , worldCenterPos , lerp );
			player2->translate = glm::mix( player2OldPos , worldCenterPos , lerp );
			player->translate = glm::mix( plane->translate + glm::vec3( 0 , 2 , 2 ) , worldCenterPos + glm::vec3( 0 , 0 , 0.5f ) , lerp );
			camera->direction = glm::mix( glm::normalize( plane->translate - player->translate ) ,
										  glm::normalize( glm::vec3( 0 , 0 , -0.5f ) ) ,
										  lerp );
			break;
		case Land:
			land -= Clock::dt;
			if ( land <= 0 )
			{
				aniState = None;
				animating = false;
				animation->play( 0 );
				animation2->play( 0 );
				player1->active = false;
				player2->active = false;
				marker1.used = true;
				marker2.used = true;
			}
			break;
		default:
			aniState = ToFight;
			animation->play( 1 );
			animation2->play( 1 );
			player1->rotate = rotationBetweenVectors( glm::vec3( 0 , 0 , 1 ) , ( worldCenterPos - glm::vec3( 0 , 1 , 0 ) ) - player1->translate , 4 , glm::vec3( 0 , 0 , 1 ) , glm::vec3( 0 , 0 , -1 ) , glm::vec3( 1 , 0 , 0 ) , glm::vec3( -1 , 0 , 0 ) );
			player2->rotate = rotationBetweenVectors( glm::vec3( 0 , 0 , 1 ) , ( worldCenterPos - glm::vec3( 0 , 1 , 0 ) ) - player2->translate , 4 , glm::vec3( 0 , 0 , 1 ) , glm::vec3( 0 , 0 , -1 ) , glm::vec3( 1 , 0 , 0 ) , glm::vec3( -1 , 0 , 0 ) );
			//player1->rotate = glm::eulerAngles( glm::quat_cast( glm::lookAt(player1OldPos , worldCenterPos - glm::vec3( 0 , 1 , 0 ) , glm::vec3( 0 , 1 , 0 ) ) ) );
			//player2->rotate = glm::eulerAngles( glm::quat_cast( glm::lookAt(player2OldPos , worldCenterPos - glm::vec3(0,1,0)  , glm::vec3( 0 , 1 , 0 ) ) ) );
			break;
	}
}

bool War::findMarkers()
{
	std::vector<FoundMarkerInfo> list = aniCardLib->queryResultList();
	bool player1OK = false, player2OK = false;
	for ( unsigned int i = 0; i < list.size(); ++i )
	{
		if ( !animating )
		{
			if ( marker1.used && ( marker1.cardIndex != list.at( i ).cardIndex && marker2.cardIndex != list.at( i ).cardIndex ) )
			{
				player1OK = true;
				player1->active = true;
				marker1 = list.at( i );
				glm::vec3 characterPos( ( list.at( i ).center.x * plane->scale.x ) , 0 , -( list.at( i ).center.y * plane->scale.z ) );
				player1->translate = characterPos;
				renderable1->geometryInfo = aniCardLib->getCardGeometry( list.at( i ).cardIndex );
				renderable1->swapTexture( aniCardLib->getCardTexture( list.at( i ).cardIndex ) , 0 );
				player1OldPos = characterPos;
			}
			else if ( marker2.used && ( marker1.cardIndex != list.at( i ).cardIndex && marker2.cardIndex != list.at( i ).cardIndex ) )
			{
				player2OK = true;
				player2->active = true;
				marker2 = list.at( i );
				glm::vec3 characterPos2( ( list.at( i ).center.x * plane->scale.x ) , 0 , -( list.at( i ).center.y * plane->scale.z ) );
				player2->translate = characterPos2;
				renderable2->geometryInfo = aniCardLib->getCardGeometry( list.at( i ).cardIndex );
				renderable2->swapTexture( aniCardLib->getCardTexture( list.at( i ).cardIndex ) , 0 );
				player2OldPos = characterPos2;
			}
		}

		if ( !marker1.used && marker1.cardIndex == list.at( i ).cardIndex )
		{
			player1OK = true;
			player1->active = true;
			glm::vec3 characterPos( ( list.at( i ).center.x * plane->scale.x ) , 0 , -( list.at( i ).center.y * plane->scale.z ) );
			player1->translate = characterPos;
			player1OldPos = characterPos;
		}
		else if ( marker1.used && marker1.cardIndex == list.at( i ).cardIndex ) player1OK = true;
		else if ( !marker2.used && marker2.cardIndex == list.at( i ).cardIndex )
		{
			player2OK = true;
			player2->active = true;
			glm::vec3 characterPos2( ( list.at( i ).center.x * plane->scale.x ) , 0 , -( list.at( i ).center.y * plane->scale.z ) );
			player2->translate = characterPos2;
			player2OldPos = characterPos2;
		}
		else if ( marker2.used && marker2.cardIndex == list.at( i ).cardIndex ) player2OK = true;
		if ( player1OK && player2OK ) break;
	}

	if ( player1OK )
	{
		player1Fails = 0;
		
	}
	else ++player1Fails;
	if ( player2OK )
	{
		player2Fails = 0;
		
	}
	else ++player2Fails;
	if ( !animating )
	{
		if ( player1Fails >= maxFails )
		{
			player1->active = false;
			marker1.used = true;
			marker1.cardIndex = UINT_MAX;
		}
		if ( player2Fails >= maxFails )
		{
			player2->active = false;
			marker2.used = true;
			marker2.cardIndex = UINT_MAX;
		}
	}
		//if ( list.size() )
		//{

		//	marker1 = list.at( 0 );
		//	player1Fails = 0;
		//	/*glm::vec3 characterPos( ( list.at( 0 ).center.x * plane->scale.x ) , 0 , -( list.at( 0 ).center.y * plane->scale.z ) );
		//	player1->translate = characterPos;
		//	renderable1->geometryInfo = aniCardLib->getCardGeometry( list.at( 0 ).cardIndex );
		//	renderable1->swapTexture( aniCardLib->getCardTexture( list.at( 0 ).cardIndex ) , 0 );*/
		//	
		//	player1->active = true;
		//	if ( list.size() < 2 )
		//	{
		//		if ( player2->active && player2Fails < maxFails ) ++player2Fails;
		//	}
		//	else
		//	{
		//		for ( unsigned int i = 1; i < list.size(); ++i )
		//		{
		//			glm::vec3 characterPos2( ( list.at( i ).center.x * plane->scale.x )  , 0 , -( list.at( i ).center.y * plane->scale.y ) );
		//			if ( glm::length( characterPos - characterPos2 ) > 0.5f )
		//			{
		//				player2Fails = 0;
		//				player2->translate = characterPos2;
		//				renderable2->geometryInfo = aniCardLib->getCardGeometry( list.at( i ).cardIndex );
		//				renderable2->swapTexture( aniCardLib->getCardTexture( list.at( i ).cardIndex ) , 0 );
		//				player2OldPos = characterPos2;
		//				marker2 = list.at( i );
		//				player2->active = true;
		//				break;
		//			}
		//		}
		//	}
		//}
		//else
		//{
		//	if ( player1->active && player1Fails < maxFails ) ++player1Fails;
		//	if ( player2->active && player2Fails < maxFails ) ++player2Fails;
	return (player1OK && player1->active) && (player2OK && player2->active);
}


void War::paintGL()
{
	if ( aniCardLib )
	{
		long width;
		long height;
		if ( aniCardLib->copyPicture(planeTexture, &width, &height) )
		{
			float toSize = std::min( (float)width , (float)height );
			toSize = 1.0f / toSize;
			//std::cout << toSize * width << std::endl;
			plane->scale = glm::vec3( toSize * width , 1.0f , toSize * height );
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
	GraphicsSharedUniformManager::globalSharedUniformManager.updateLights();
	GraphicsCameraManager::globalCameraManager.drawAllCameras();
}
void War::setCameraSource( CameraItem& camera , CameraMode& mode )
{
	aniCardLib->setCamera( camera , mode );
}