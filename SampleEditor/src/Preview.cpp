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
void Preview::initializeGL()
{
	CommonGraphicsCommands::initializeGlobalGraphics();
	GameObjectManager::globalGameObjectManager.initialize();


	cardRenderer.initialize( 1 );
	ShaderInfo* cardShader;
	{
		std::string errors;
		std::string vert = FileReader( "assets/shaders/CardVertex.glsl" );
		std::string frag = FileReader( "assets/shaders/CardFragment.glsl" );
		cardShader = GraphicsShaderManager::globalShaderManager.createShaderInfo( vert.c_str() , frag.c_str() , &errors );
		std::cout << errors.c_str() << std::endl;
	}
	GeometryInfo* plane = GraphicsGeometryManager::globalGeometryManager.addPMDGeometry( "assets/models/plane.pmd" , GraphicsBufferManager::globalBufferManager );
	plane->addShaderStreamedParameter( 0 , PT_VEC3 , VertexInfo::STRIDE , VertexInfo::POSITION_OFFSET );
	plane->addShaderStreamedParameter( 3 , PT_VEC2 , VertexInfo::STRIDE , VertexInfo::UV_OFFSET );
	cardTexture = GraphicsTextureManager::globalTextureManager.addTexture( 0 , 0 , 0 , 0 );

	Renderable* renderable = cardRenderer.addRenderable();
	renderable->initialize( 10 , 2 );
	renderable->geometryInfo = plane;
	renderable->shaderInfo = cardShader;
	renderable->alphaBlendingEnabled = false;
	renderable->culling = CT_NONE;
	renderable->addTexture( cardTexture );


	cardPlane = GameObjectManager::globalGameObjectManager.addGameObject();
	cardPlane->addComponent( renderable );

	Camera* cardCamera = GraphicsCameraManager::globalCameraManager.addCamera();
	cardCamera->initializeRenderManagers();
	cardCamera->addRenderList( &cardRenderer );
	GameObject* cardViewer = GameObjectManager::globalGameObjectManager.addGameObject();
	cardViewer->addComponent( cardCamera );

	connect( &updateTimer , SIGNAL( timeout() ) , this , SLOT( update() ) );
	updateTimer.start( 0 );
	emit initialized();
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



	cardPlane->scale = glm::vec3( theWindScale , 1 );

	GameObjectManager::globalGameObjectManager.earlyUpdateParents();
	GameObjectManager::globalGameObjectManager.updateParents();
	GameObjectManager::globalGameObjectManager.lateUpdateParents();
	repaint();
}

void Preview::paintGL()
{
	drawing = true;
	while ( changingTexture );
	GraphicsCameraManager::globalCameraManager.drawAllCameras();
	drawing = false;
}

void Preview::setCard( unsigned char* cardImage , const unsigned int& width , const unsigned int& height )
{
	changingTexture = true;
	while ( drawing );
	GraphicsTextureManager::globalTextureManager.editTexture( cardTexture , (char*)cardImage , width , height , 0);
	unsigned int maxDim = std::max( width , height);
	if ( minDim == width )
	{
		cardScale.x = 1;
		cardScale.y = ( ( float ) maxDim ) / height;
	}
	else
	{
		cardScale.y = 1;
		cardScale.x = ( ( float ) maxDim ) / width;
	}
	changingTexture = false;
}