#include <Editor.h>
#include <QtGui\QVBoxLayout>
#include <QtGui\QHBoxLayout>
#include <QtGui\QLabel>
#include <QtGui\QPushButton>
#include <QtGui\QListWidget>
#include <QtGui\QFileDialog>
#include <QtCore\QCoreApplication>
#include <Marker.h>
#include <DebugMemory.h>
Editor::Editor()
{
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget( new QLabel( "AR Cards" ) );
	mainLayout->addWidget( arCardsList = new QListWidget );
	arCardsList->setSelectionMode( QAbstractItemView::SelectionMode::ExtendedSelection );
	QHBoxLayout* arCardsButtons = new QHBoxLayout;
	QPushButton* addCardButton;
	arCardsButtons->addWidget( addCardButton = new QPushButton( "Add Card" ) );
	QPushButton* swapCardsButton;
	arCardsButtons->addWidget( swapCardsButton = new QPushButton("Swap Cards") );
	mainLayout->addLayout( arCardsButtons );

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

	editor = 0;

	connect( addCardButton , SIGNAL( clicked() ) , this , SLOT( addCard() ) );
	connect( swapCardsButton , SIGNAL( clicked() ) , this , SLOT( swapCards() ) );
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
	editor = new AniCardLibCommonEditor();
}

void Editor::selectCard( int selected )
{
	if ( selected < 0 ) return;
	Marker* marker = editor->getMarker( selected );
	if ( marker )
	{
		preview.setCard( editor->getPicturePointer( selected ) , marker->width , marker->height, editor->getCardGeometry(selected), editor->getCardTexture(selected));
	}
}

void Editor::addCard()
{
	QFileDialog dialogbox;
	QFileInfo fileName( dialogbox.getOpenFileName( NULL , "Add Card" , QCoreApplication::applicationDirPath() , "*.png" ) );
	if ( !fileName.isFile() ) return;
	int cardIndex = editor->addMarker( fileName.absoluteFilePath().toUtf8()) ;
	if ( cardIndex >= 0 )
	{
		arCardsList->addItem( fileName.baseName());
	}
}

void Editor::swapCards()
{
	QList<QListWidgetItem*> items = arCardsList->selectedItems();
	if ( items.size() != 2 ) return;
	int row1 , row2;
	Marker* marker = editor->getMarker( row1 = arCardsList->row( items[0] ) );
	Marker* marker2 = editor->getMarker( row2 = arCardsList->row( items[1] ) );

	if ( !marker || !marker2 ) return;
	Marker temp = *marker2;
	*marker2 = *marker;
	*marker = temp;

	int maxs = std::max( row1 , row2 );
	if ( maxs == row1 )
	{
		QListWidgetItem* item1 = arCardsList->takeItem( row1 );
		QListWidgetItem* item2 = arCardsList->takeItem( row2 );
		arCardsList->insertItem( row2 , item1 );
		arCardsList->insertItem( row1 , item2 );
	}
	else
	{
		QListWidgetItem* item1 = arCardsList->takeItem( row2 );
		QListWidgetItem* item2 = arCardsList->takeItem( row1 );
		arCardsList->insertItem( row1 , item1 );
		arCardsList->insertItem( row2 , item2 );
	}
}

void Editor::addModel()
{
	QFileDialog dialogbox;
	QFileInfo fileName( dialogbox.getOpenFileName( NULL , "Add Model", QCoreApplication::applicationDirPath(), "*.pmd" ) );
	if ( !fileName.isFile() ) return;
	int modelIndex = editor->addModel( fileName.absoluteFilePath().toUtf8() );
	if ( modelIndex >= 0 )
	{
		modelsList->addItem( fileName.baseName() );
	}
}
void Editor::linkModel()
{
	if ( modelsList->currentRow() < 0 ) return;
	QList<QListWidgetItem*> items = arCardsList->selectedItems();
	for ( int i = 0; i < items.size(); ++i )
	{
		Marker* marker = editor->getMarker( arCardsList->row( items[i] ) );
		if ( marker )
		{
			marker->linkedModel = modelsList->currentRow();
		}
	}
}
void Editor::addTexture()
{
	QFileDialog dialogbox;
	QFileInfo fileName( dialogbox.getOpenFileName( NULL , "Add Texture" , QCoreApplication::applicationDirPath() , "*.tex" ) );
	if ( !fileName.isFile() ) return;
	int textureIndex = editor->addTexture( fileName.absoluteFilePath().toUtf8() );
	if ( textureIndex >= 0 )
	{
		texturesList->addItem( fileName.baseName() );
	}
}
void Editor::linkTexture()
{
	if ( texturesList->currentRow() < 0 ) return;
	QList<QListWidgetItem*> items = arCardsList->selectedItems();
	for ( int i = 0; i < items.size(); ++i )
	{
		Marker* marker = editor->getMarker( arCardsList->row( items[i] ) );
		if ( marker )
		{
			marker->linkedTexture = texturesList->currentRow();
		}
	}
}
void Editor::save()
{
	QFileDialog dialogbox;
	QString fileName = dialogbox.getSaveFileName( NULL , "Save File" , QCoreApplication::applicationDirPath() , "*.aclf" ).toUtf8();
	if ( fileName.isEmpty() ) return;
	editor->save( fileName.toUtf8() );
}
void Editor::load()
{
	QFileDialog dialogbox;
	QString fileName = dialogbox.getOpenFileName( NULL , "Load File" , QCoreApplication::applicationDirPath() , "*.aclf" ).toUtf8();
	if ( fileName.isEmpty() ) return;
	delete editor;
	editor = new AniCardLibCommonEditor;
	editor->load( fileName.toUtf8() );
	arCardsList->clear();
	for ( unsigned int i = 0; i < editor->getMarkerListSize(); ++i )
	{
		arCardsList->addItem( QString::number( i ) );
	}
	modelsList->clear();
	for ( unsigned int i = 0; i < editor->getGeometryListSize(); ++i )
	{
		modelsList->addItem( QString::number( i ) );
	}

	texturesList->clear();
	for ( unsigned int i = 0; i < editor->getTextureListSize(); ++i )
	{
		texturesList->addItem( QString::number( i ) );
	}
	
}

Editor::~Editor()
{
	delete editor;
}