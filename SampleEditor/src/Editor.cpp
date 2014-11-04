#include <Editor.h>
#include <QtGui\QVBoxLayout>
#include <QtGui\QHBoxLayout>
#include <QtGui\QLabel>
#include <QtGui\QPushButton>
#include <QtGui\QListWidget>
#include <QtGui\QFileDialog>
#include <QtCore\QCoreApplication>
#include <Marker.h>
Editor::Editor()
{
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget( new QLabel( "AR Cards" ) );
	mainLayout->addWidget( arCardsList = new QListWidget );
	QPushButton* addCardButton;
	mainLayout->addWidget( addCardButton = new QPushButton( "Add Card" ));

	mainLayout->addWidget( new QLabel( "Models" ) );
	mainLayout->addWidget( modelsList = new QListWidget );

	QHBoxLayout* modelsButtons = new QHBoxLayout;
	QPushButton* addModelButton = new QPushButton( "Add Model" );
	modelsButtons->addWidget( addModelButton );
	QPushButton* linkModel = new QPushButton( "Link Model" );
	modelsButtons->addWidget( linkModel );
	mainLayout->addLayout( modelsButtons );

	mainLayout->addWidget( new QLabel( "Textures" ) );
	mainLayout->addWidget( texturesList = new QListWidget );

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

	QHBoxLayout* masterLayout = new QHBoxLayout;
	masterLayout->addWidget( &preview );
	masterLayout->addLayout( mainLayout );

	setLayout( masterLayout );

	file = 0;

	connect( addCardButton , SIGNAL( clicked() ) , this , SLOT( addCard() ) );
	connect( addModelButton , SIGNAL( clicked() ) , this , SLOT( addModel() ) );
	connect( linkModel , SIGNAL( clicked() ) , this , SLOT( linkModel() ) );
	connect( addTextureButton , SIGNAL( clicked() ) , this , SLOT( addTexture() ) );
	connect( linkTexture , SIGNAL( clicked() ) , this , SLOT( linkTexture() ) );
	connect( saveButton , SIGNAL( clicked() ) , this , SLOT( save() ) );
	connect( loadButton , SIGNAL( clicked() ) , this , SLOT( load() ) );
	connect( &preview , SIGNAL( initialized() ) , this , SLOT( initialize() ) );
	connect( arCardsList , SIGNAL( currentRowChanged( int ) ) , this , SLOT(selectCard( int )) );
	preview.setMinimumWidth( 1280 );
	preview.setMinimumHeight( 720 );
}
void Editor::initialize()
{
	file = new AniCardLibFileInfo();
}

void Editor::selectCard( int selected )
{
	if ( selected < 0 ) return;
	Marker* marker = file->getMarker( selected );
	if ( marker )
	{
		preview.setCard( file->getPicturePointer( selected ) , marker->width , marker->height);
	}
}

void Editor::addCard()
{
	QFileDialog dialogbox;
	QFileInfo fileName(dialogbox.getOpenFileName( NULL , "Add Card" ));
	int cardIndex = file->addMarker( fileName.absoluteFilePath().toUtf8()) ;
	if ( cardIndex >= 0 )
	{
		arCardsList->addItem( fileName.baseName());
	}
}
void Editor::addModel()
{
	QFileDialog dialogbox;
	QFileInfo fileName( dialogbox.getOpenFileName( NULL , "Add Model", QCoreApplication::applicationDirPath(), "*.pmd" ) );
	int modelIndex = file->addModel( fileName.absoluteFilePath().toUtf8() );
	if ( modelIndex >= 0 )
	{
		modelsList->addItem( fileName.baseName() );
	}
}
void Editor::linkModel()
{
	if ( modelsList->currentRow() < 0 ) return;
	Marker* marker = file->getMarker( arCardsList->currentRow() );
	if ( marker)
	{
		marker->linkedModel = modelsList->currentRow();
	}
}
void Editor::addTexture()
{
	QFileDialog dialogbox;
	QFileInfo fileName( dialogbox.getOpenFileName( NULL , "Add Texture" , QCoreApplication::applicationDirPath() , "*.png" ) );
	int textureIndex = file->addTexture( fileName.absoluteFilePath().toUtf8() );
	if ( textureIndex >= 0 )
	{
		texturesList->addItem( fileName.baseName() );
	}
}
void Editor::linkTexture()
{
	if ( texturesList->currentRow() < 0 ) return;
	Marker* marker = file->getMarker( arCardsList->currentRow() );
	if ( marker )
	{
		marker->linkedTexture = texturesList->currentRow();
	}
}
void Editor::save()
{
	QFileDialog dialogbox;
	file->save( dialogbox.getSaveFileName( NULL , "Save File" , QCoreApplication::applicationDirPath() , "*.aclf" ).toUtf8() );
}
void Editor::load()
{
	QFileDialog dialogbox;
	delete file;
	file = new AniCardLibFileInfo(dialogbox.getOpenFileName( NULL , "Load File" , QCoreApplication::applicationDirPath() , "*.aclf" ).toUtf8());
	arCardsList->clear();
	for ( unsigned int i = 0; i < file->getMarkerListSize(); ++i )
	{
		arCardsList->addItem( QString::number( i ) );
	}
	modelsList->clear();
	for ( unsigned int i = 0; i < file->getGeometryListSize(); ++i )
	{
		modelsList->addItem( QString::number( i ) );
	}

	texturesList->clear();
	for ( unsigned int i = 0; i < file->getTextureListSize(); ++i )
	{
		texturesList->addItem( QString::number( i ) );
	}
	
}