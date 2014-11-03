#include <Editor.h>
#include <QtGui\QVBoxLayout>
#include <QtGui\QHBoxLayout>
#include <QtGui\QLabel>
#include <QtGui\QPushButton>
#include <QtGui\QListView>
Editor::Editor()
{
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget( new QLabel( "AR Cards" ) );
	mainLayout->addWidget( arCardsList = new QListView );
	QPushButton* addCardButton;
	mainLayout->addWidget( addCardButton = new QPushButton( "Add Card" ));

	mainLayout->addWidget( new QLabel( "Models" ) );
	mainLayout->addWidget( modelsList = new QListView );

	QHBoxLayout* modelsButtons = new QHBoxLayout;
	QPushButton* addModelButton = new QPushButton( "Add Model" );
	modelsButtons->addWidget( addModelButton );
	QPushButton* linkModel = new QPushButton( "Link Model" );
	modelsButtons->addWidget( linkModel );
	mainLayout->addLayout( modelsButtons );

	mainLayout->addWidget( new QLabel( "Textures" ) );
	mainLayout->addWidget( texturesList = new QListView );

	QHBoxLayout* texturesButtons = new QHBoxLayout;
	QPushButton* addTextureButton = new QPushButton( "Add Texture" );
	QPushButton* linkTexture = new QPushButton( "Link Texture" );
	texturesButtons->addWidget( addTextureButton );
	texturesButtons->addWidget( linkTexture );
	mainLayout->addLayout( texturesButtons );

	QPushButton* saveButton = new QPushButton( "Save" );
	mainLayout->addWidget( saveButton );
	QPushButton* loadButton = new QPushButton( "Load" );
	mainLayout->addWidget( loadButton );
	setLayout( mainLayout );

	connect( addCardButton , SIGNAL( clicked() ) , this , SLOT( addCard() ) );
	connect( addModelButton , SIGNAL( clicked() ) , this , SLOT( addModel() ) );
	connect( linkModel , SIGNAL( clicked() ) , this , SLOT( linkModel() ) );
	connect( addTextureButton , SIGNAL( clicked() ) , this , SLOT( addTexture() ) );
	connect( linkTexture , SIGNAL( clicked() ) , this , SLOT( linkTexture() ) );
	connect( saveButton , SIGNAL( clicked() ) , this , SLOT( save() ) );
	connect( loadButton , SIGNAL( clicked() ) , this , SLOT( load() ) );
}

void Editor::addCard()
{

}
void Editor::addModel()
{

}
void Editor::linkModel()
{

}
void Editor::addTexture()
{

}
void Editor::linkTexture()
{

}
void Editor::save()
{

}
void Editor::load()
{

}