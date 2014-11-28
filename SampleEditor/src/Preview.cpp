#include <Preview.h>
#include <Graphics\CommonGraphicsCommands.h>
#include <Core\GameObjectManger.h>
#include <Graphics\GraphicsShaderManager.h>
#include <iostream>
#include <Misc\FileReader.h>
#include <Graphics\GraphicsGeometryManager.h>
#include <Graphics\GraphicsBufferManager.h>
#include <Graphics\GeometryInfo.h>
#include <Graphics\VertexInfo.h>
#include <Graphics\GraphicsTextureManager.h>
#include <Graphics\RenderableInfo.h>
#include <Core\GameObject.h>
#include <Graphics\GraphicsCameraManager.h>
#include <Graphics\Camera.h>
#include <Core\WindowInfo.h>
#include <Misc\Clock.h>
#include <Graphics\AnimationRenderingInfo.h>
#include <Graphics\GraphicsSharedUniformManager.h>
#include <Input\MouseInput.h>
#include <Input\FirstPersonCameraInput.h>
#include <DebugMemory.h>
void Preview::initializeGL()
{
	CommonGraphicsCommands::initializeGlobalGraphics();
	GameObjectManager::globalGameObjectManager.initialize();
	cameras.initialize( 2 );
	ShaderInfo* modelShader;
	{
		std::string errors;
		std::string vert = FileReader( "assets/shaders/ModelVertex.glsl" );
		std::string frag = FileReader( "assets/shaders/ModelFragment.glsl" );
		modelShader = GraphicsShaderManager::globalShaderManager.createShaderInfo( vert.c_str() , frag.c_str() , &errors );
		std::cout << errors.c_str() << std::endl;
	}

	ShaderInfo* cardShader;
	{
		std::string errors;
		std::string vert = FileReader( "assets/shaders/CardVertex.glsl" );
		std::string frag = FileReader( "assets/shaders/CardFragment.glsl" );
		cardShader = GraphicsShaderManager::globalShaderManager.createShaderInfo( vert.c_str() , frag.c_str() , &errors );
		std::cout << errors.c_str() << std::endl;
	}

	modelRenderable = GraphicsRenderingManager::globalRenderingManager.addRenderable();
	modelRenderable->initialize( 10 , 1 );
	modelRenderable->sharedUniforms = &GraphicsSharedUniformManager::globalSharedUniformManager;
	modelRenderable->shaderInfo = modelShader;
	modelRenderable->alphaBlendingEnabled = false;
	modelRenderable->culling = CT_NONE;
	
	GameObject* modelObject = GameObjectManager::globalGameObjectManager.addGameObject();
	modelObject->addComponent( modelRenderable );
	modelObject->addComponent( animations = new AnimationRenderingInfo );
	modelObject->scale = glm::vec3( 5 , 5 , 5 );
	Camera* modelCamera = cameras.addCamera();
	modelCamera->initializeRenderManagers();
	modelCamera->addRenderList( &GraphicsRenderingManager::globalRenderingManager );
	GameObject* modelViewer = GameObjectManager::globalGameObjectManager.addGameObject();
	modelViewer->addComponent( modelCamera );
	modelViewer->translate = glm::vec3( 0 , 0 , 25 );
	fpsInput = new FirstPersonCameraInput;
	fpsInput->moveSensitivity = 1;
	fpsInput->rotationSensitivity = 0.1f;
	modelViewer->addComponent( fpsInput );

	cardRenderer.initialize( 1 );
	GeometryInfo* plane = GraphicsGeometryManager::globalGeometryManager.addPMDGeometry( "assets/models/plane.pmd" , GraphicsBufferManager::globalBufferManager );
	plane->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
	plane->addShaderStreamedParameter( 3 , PT_VEC2 , VertexInfo::STRIDE , VertexInfo::UV_OFFSET );
	cardTexture = GraphicsTextureManager::globalTextureManager.addTexture( 0 , 0 , 0 , 0 );

	Renderable* renderable = cardRenderer.addRenderable();
	renderable->initialize( 10 , 1 );
	renderable->geometryInfo = plane;
	renderable->shaderInfo = cardShader;
	renderable->alphaBlendingEnabled = false;
	renderable->culling = CT_NONE;
	renderable->addTexture( cardTexture );


	cardPlane = GameObjectManager::globalGameObjectManager.addGameObject();
	cardPlane->addComponent( renderable );

	Camera* cardCamera = cameras.addCamera();
	cardCamera->initializeRenderManagers();
	cardCamera->addRenderList( &cardRenderer );
	cardCamera->x = 0.75f;
	cardCamera->y = 0.75f;
	cardCamera->width = 0.25f;
	cardCamera->height = 0.25f;
	GameObject* cardViewer = GameObjectManager::globalGameObjectManager.addGameObject();
	cardViewer->addComponent( cardCamera );


	MouseInput::globalMouseInput.updateOldMousePosition = false;

	setMouseTracking( true );
	QCursor c = cursor();
	c.setPos( mapToGlobal( QPoint( width() / 2 , height() / 2 ) ) );
	c.setShape( Qt::BlankCursor );
	setCursor( c );
	MouseInput::globalMouseInput.updateMousePosition( glm::vec2( width() / 2 , height() / 2 ) );

	connect( &updateTimer , SIGNAL( timeout() ) , this , SLOT( update() ) );
	updateTimer.start( 0 );
	emit initialized();
}

void Preview::mouseMoveEvent( QMouseEvent* e )
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

void Preview::update()
{
	WindowInfo::width = width();
	WindowInfo::height = height();
	Clock::update();

	unsigned int minWindow = std::min( WindowInfo::width , WindowInfo::height );
	glm::vec2 theWindScale;
	if ( minWindow == WindowInfo::width )
	{
		theWindScale.x = 1;
		theWindScale.y = ((float)minWindow) / WindowInfo::height;
	}
	else
	{
		theWindScale.y = 1;
		theWindScale.x = ((float)minWindow) / WindowInfo::width;
	}
	theWindScale *= cardScale;



	cardPlane->scale = glm::vec3(theWindScale , 1 );

	GameObjectManager::globalGameObjectManager.earlyUpdateParents();
	GameObjectManager::globalGameObjectManager.updateParents();
	GameObjectManager::globalGameObjectManager.lateUpdateParents();
	MouseInput::globalMouseInput.mouseDelta = glm::vec2();
	repaint();
}

void Preview::paintGL()
{
	drawing = true;
	if ( changingCard )
	{
		drawing = false;
		return;
	}
	cameras.drawAllCameras();
	drawing = false;
}

void Preview::setCard( const unsigned char* cardImage , const unsigned int& width , const unsigned int& height , GeometryInfo* cardGeo , TextureInfo* cardModelTexture )
{
	changingCard = true;
	while ( drawing );
	GraphicsTextureManager::globalTextureManager.editTexture( cardTexture , (char*)cardImage , width , height , 0);
	unsigned int maxDim = std::max( width , height);
	if ( maxDim == width )
	{
		cardScale.x = 1;
		cardScale.y = (float)height / maxDim;
	}
	else
	{
		cardScale.y = 1;
		cardScale.x = (float)width/maxDim ;
	}
	if ( cardGeo )
	{
		cardGeo->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
		cardGeo->addShaderStreamedParameter( 3 , PT_VEC2 , VertexInfo::STRIDE , VertexInfo::UV_OFFSET );
		cardGeo->addShaderStreamedParameter( 6 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGINDEX_OFFSET );
		cardGeo->addShaderStreamedParameter( 7 , PT_VEC4 , VertexInfo::STRIDE , VertexInfo::BLENDINGWEIGHT_OFFSET );
	}

	modelRenderable->geometryInfo = cardGeo;
	modelRenderable->swapTexture( cardModelTexture , 0 );
	changingCard = false;
}

void Preview::lockUpdate()
{
	changingCard = true;
	while ( drawing );
}
void Preview::unlockUpdate()
{
	changingCard = false;
}

Preview::~Preview()
{
	GameObjectManager::globalGameObjectManager.destroy();
	cameras.destroy();
	cardRenderer.destroy();
	CommonGraphicsCommands::destroyGlobalGraphics();
	delete animations;
	delete fpsInput;
}