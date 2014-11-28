#include <Editor.h>
#include <QtGui\QVBoxLayout>
#include <QtGui\QHBoxLayout>
#include <QtGui\QLabel>
#include <QtGui\QPushButton>
#include <QtGui\QListWidget>
#include <QtGui\QFileDialog>
#include <QtCore\QCoreApplication>
#include <Marker.h>
#include <SOIL.h>
#include <fstream>
#include <QtGui\QComboBox>
#include <QtGui\QSpinBox>
#include <Graphics\AnimationFrameRangeInfo.h>
#include <Graphics\RenderableInfo.h>
#include <Graphics\GeometryInfo.h>
#include <Graphics\AnimationRenderingInfo.h>
#include <Core\GameObject.h>
#include <QtGui\QGroupBox>
#include <QtGui\QRadioButton>
#include <DebugMemory.h>
Editor::Editor()
{
	QVBoxLayout* mainLayout = new QVBoxLayout;

	QGroupBox* modeSelection = new QGroupBox( "Mode" );
	QHBoxLayout* modeSelectionLayout = new QHBoxLayout;
	QRadioButton* cardMode = new QRadioButton( "Card" );
	modeSelectionLayout->addWidget( cardMode );
	QRadioButton* modelMode = new QRadioButton( "Model" );
	modeSelectionLayout->addWidget( modelMode );
	modeSelection->setLayout( modeSelectionLayout );
	mainLayout->addWidget( modeSelection );
	arCardsWidget = new QWidget;
	QVBoxLayout* arCardsWidgetLayout = new QVBoxLayout( );
	arCardsWidget->setLayout( arCardsWidgetLayout );

	arCardsWidgetLayout->addWidget(new QLabel("Cards"));
	arCardsWidgetLayout->addWidget( arCardsList = new QListWidget );
	arCardsList->setSelectionMode( QAbstractItemView::SelectionMode::ExtendedSelection );
	QHBoxLayout* arCardsButtons = new QHBoxLayout;
	QPushButton* addCardButton;
	arCardsButtons->addWidget( addCardButton = new QPushButton( "Add Card" ) );
	QPushButton* swapCardsButton;
	arCardsButtons->addWidget( swapCardsButton = new QPushButton("Swap Cards") );
	arCardsWidgetLayout->addLayout( arCardsButtons );
	arCardsWidget->setLayout( arCardsWidgetLayout );
	mainLayout->addWidget( arCardsWidget );
	arCardsWidget->hide();

	pmdEditorWidget = new QWidget;
	QVBoxLayout* pmdEditorWidgetLayout = new QVBoxLayout;
	QHBoxLayout* comboBoxLayout = new QHBoxLayout;
	frameList = new QComboBox;
	comboBoxLayout->addWidget( frameList );
	QPushButton* addFrames = new QPushButton( "Add" );
	comboBoxLayout->addWidget( addFrames );

	pmdEditorWidgetLayout->addLayout( comboBoxLayout );
	nextFrame = new QSpinBox;
	nextFrame->setRange( 0 , INT_MAX );
	nextFrame->setValue( 0 );
	pmdEditorWidgetLayout->addWidget( nextFrame );
	startFrame = new QSpinBox;
	startFrame->setRange( 0 , INT_MAX );
	startFrame->setValue( 0 );
	pmdEditorWidgetLayout->addWidget( startFrame );
	endFrame = new QSpinBox;
	endFrame->setRange( 0 , INT_MAX );
	endFrame->setValue( INT_MAX );
	pmdEditorWidgetLayout->addWidget( endFrame );
	pmdEditorWidget->setLayout( pmdEditorWidgetLayout );
	mainLayout->addWidget( pmdEditorWidget );
	pmdEditorWidget->hide();
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

	connect( modelsList , SIGNAL( currentRowChanged( int ) ) , this , SLOT( selectModel( int ) ) );
	connect( texturesList , SIGNAL( currentRowChanged( int ) ) , this , SLOT( selectModel( int ) ) );

	connect( frameList , SIGNAL( currentIndexChanged( int ) ) , this , SLOT( frameIndexChanged( int ) ) );
	connect( addFrames , SIGNAL( pressed() ) , this , SLOT( addFrameRange() ) );

	connect( nextFrame , SIGNAL( valueChanged( int ) ) , this , SLOT( changeParamsNext() ) );
	connect( startFrame , SIGNAL( valueChanged( int ) ) , this , SLOT( changeParamsStart() ) );
	connect( endFrame , SIGNAL( valueChanged( int ) ) , this , SLOT( changeParamsEnd() ) );

	connect( cardMode , SIGNAL( toggled( bool ) ) , this , SLOT( changeCardMode( bool ) ) );
	connect( modelMode , SIGNAL( toggled( bool ) ) , this , SLOT( changeModelMode( bool ) ) );
	cardMode->setChecked( true );
	preview.setMinimumWidth( 1280 );
	preview.setMinimumHeight( 720 );
	changing = false;
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
	QStringList fileNames( dialogbox.getOpenFileNames( NULL , "Add Card" , QString() , "*.png" ) );

	for ( int i = 0; i < fileNames.size(); ++i )
	{
		QFileInfo fileName( fileNames[i] );
		if ( !fileName.isFile() ) return;
		int cardIndex = editor->addMarker( fileName.absoluteFilePath().toUtf8() );
		if ( cardIndex >= 0 )
		{
			arCardsList->addItem( fileName.baseName() );
		}
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
	QStringList fileNames( dialogbox.getOpenFileNames( NULL , "Add Model" , QString() , "*.pmd" ) );
	for ( int i = 0; i < fileNames.size(); ++i )
	{
		QFileInfo fileName( fileNames[i] );
		if ( !fileName.isFile() ) return;
		int modelIndex = editor->addModel( fileName.absoluteFilePath().toUtf8() );
		if ( modelIndex >= 0 )
		{
			modelsList->addItem( fileName.baseName() );
		}
	}
}
void Editor::linkModel()
{
	if ( modelsList->currentRow() < 0 ) return;
	QList<QListWidgetItem*> items = arCardsList->selectedItems();
	if ( !items.size() ) return;
	for ( int i = 0; i < items.size(); ++i )
	{
		Marker* marker = editor->getMarker( arCardsList->row( items[i] ) );
		if ( marker )
		{
			marker->linkedModel = modelsList->currentRow();
		}
	}

	selectCard( arCardsList->row( items[0] ) );
}
void Editor::addTexture()
{
	QFileDialog dialogbox;

	QStringList fileNames( dialogbox.getOpenFileNames( NULL , "Add Texture" , QString() , "*.tex;*.png" ) );

	for ( int i = 0; i < fileNames.size(); ++i )
	{
		QFileInfo fileName( fileNames[i] );
		if ( !fileName.isFile() ) return;
		int textureIndex = -1;
		if ( QString::compare( fileName.suffix() , "tex" , Qt::CaseInsensitive ) == 0 )
		{
			textureIndex = editor->addTexture( fileName.absoluteFilePath().toUtf8() );
		}
		else
		{
			int width , height , channels;
			unsigned char* theTex = SOIL_load_image( fileName.absoluteFilePath().toUtf8() , &width , &height , &channels , SOIL_LOAD_RGBA );
			std::string outputStream;
			for ( int y = height - 1; y >= 0; --y )
			{
				for ( int x = 0; x < width; ++x )
				{
					outputStream += std::string( ( char* ) &theTex[( y * 4 * width ) + ( x * 4 )] , sizeof( unsigned char ) );
					outputStream += std::string( ( char* ) &theTex[( ( y * 4 * width ) + ( x * 4 ) ) + 1] , sizeof( unsigned char ) );
					outputStream += std::string( ( char* ) &theTex[( ( y * 4 * width ) + ( x * 4 ) ) + 2] , sizeof( unsigned char ) );
					outputStream += std::string( ( char* ) &theTex[( ( y * 4 * width ) + ( x * 4 ) ) + 3] , sizeof( unsigned char ) );
				}
			}

			textureIndex = editor->addTexture( outputStream.c_str() , width , height );
		}

		if ( textureIndex >= 0 )
		{
			texturesList->addItem( fileName.baseName() );
		}
	}
}
void Editor::linkTexture()
{
	if ( texturesList->currentRow() < 0 ) return;
	QList<QListWidgetItem*> items = arCardsList->selectedItems();
	if ( !items.size() ) return;
	for ( int i = 0; i < items.size(); ++i )
	{
		Marker* marker = editor->getMarker( arCardsList->row( items[i] ) );
		if ( marker )
		{
			marker->linkedTexture = texturesList->currentRow();
		}
	}

	selectCard( arCardsList->row( items[0] ) );
}
void Editor::save()
{
	QFileDialog dialogbox;
	QString fileName = dialogbox.getSaveFileName( NULL , "Save File" , QString() , "*.aclf" ).toUtf8();
	if ( fileName.isEmpty() ) return;
	std::vector<std::string> cardNames;
	std::vector<std::string> modelNames;
	std::vector<std::string> textureNames;

	for ( int i = 0; i < arCardsList->count(); ++i )
	{
		cardNames.push_back( std::string(arCardsList->item( i )->text().toUtf8()) );
	}

	for ( int i = 0; i < modelsList->count(); ++i )
	{
		modelNames.push_back( std::string( modelsList->item( i )->text().toUtf8() ) );
	}

	for ( int i = 0; i < texturesList->count(); ++i )
	{
		textureNames.push_back( std::string( texturesList->item( i )->text().toUtf8() ) );
	}

	editor->save( fileName.toUtf8(), true, cardNames, modelNames, textureNames );
}
void Editor::load()
{
	QFileDialog dialogbox;
	QString fileName = dialogbox.getOpenFileName( NULL , "Load File" , QString() , "*.aclf" ).toUtf8();
	if ( fileName.isEmpty() ) return;
	delete editor;
	editor = new AniCardLibCommonEditor;

	editor->load( fileName.toUtf8() , QFileInfo( fileName + ".aclfn" ).exists() );
	arCardsList->clear();
	for ( unsigned int i = 0; i < editor->getMarkerListSize(); ++i )
	{
		QString toPut( editor->getCardName( i ).c_str() );
		if ( toPut.compare( "" ) )
		{
			arCardsList->addItem( toPut );
		}
		else
		{
			arCardsList->addItem( QString::number( i ) );
		}
	}
	modelsList->clear();
	for ( unsigned int i = 0; i < editor->getGeometryListSize(); ++i )
	{
		QString toPut( editor->getModelName( i ).c_str() );
		if ( toPut.compare( "" ) )
		{
			modelsList->addItem( toPut );
		}
		else
		{
			modelsList->addItem( QString::number( i ) );
		}
	}

	texturesList->clear();
	for ( unsigned int i = 0; i < editor->getTextureListSize(); ++i )
	{
		QString toPut( editor->getTextureName( i ).c_str() );
		if ( toPut.compare( "" ) )
		{
			texturesList->addItem( toPut );
		}
		else
		{
			texturesList->addItem( QString::number( i ) );
		}
	}
	
}

void Editor::frameIndexChanged( int index )
{
	if ( !( preview.modelRenderable && preview.modelRenderable->geometryInfo && preview.modelRenderable->geometryInfo->modelData ) ) return;
	unsigned int numFrames;
	AnimationFrameRangeInfo* frameRange = preview.modelRenderable->geometryInfo->modelData->getAnimationFrameRange( &numFrames );

	if ( frameRange )
	{
		blockSignals( true );
		changing = true;
		startFrame->setMaximum( INT_MAX );
		endFrame->setMinimum( 0 );
		nextFrame->setValue( frameRange[index].nextAnimationFrameInfo );
		startFrame->setValue( frameRange[index].firstFrame );
		endFrame->setValue( frameRange[index].lastFrame );
		changing = false;
		blockSignals( false );
	}
}

void Editor::addFrameRange()
{
	if ( !( preview.modelRenderable && preview.modelRenderable->geometryInfo && preview.modelRenderable->geometryInfo->modelData ) ) return;
	unsigned int numFrames;
	AnimationFrameRangeInfo* frameRange = preview.modelRenderable->geometryInfo->modelData->getAnimationFrameRange( &numFrames );

	char* newFrameRange = new char[sizeof( AnimationFrameRangeInfo ) * ( numFrames + 1 )];
	ZeroMemory( newFrameRange , sizeof( AnimationFrameRangeInfo ) * ( numFrames + 1 ) );
	memcpy( newFrameRange , reinterpret_cast< char* >( frameRange ) , sizeof( AnimationFrameRangeInfo ) * ( numFrames ) );
	preview.modelRenderable->geometryInfo->modelData->setNewAnimationFrameRangeInfo( reinterpret_cast<AnimationFrameRangeInfo* >( newFrameRange ) , numFrames + 1 );
	delete[] newFrameRange;
	frameRange = preview.modelRenderable->geometryInfo->modelData->getAnimationFrameRange( &numFrames );

	frameList->addItem( QString::number( numFrames - 1 ) );

	frameList->setCurrentIndex( numFrames - 1 );
	nextFrame->setMaximum( numFrames - 1 );
}

void Editor::changeParamsNext()
{
	if ( changing ) return;
	int index = frameList->currentIndex();
	if ( index < 0 ) return;
	unsigned int numFrames;
	AnimationFrameRangeInfo* frameRange = preview.modelRenderable->geometryInfo->modelData->getAnimationFrameRange( &numFrames );

	frameRange[index].nextAnimationFrameInfo = nextFrame->value();
	preview.modelRenderable->parent->getComponent<Animator>()->play( frameList->currentIndex() );
}
void Editor::changeParamsStart()
{
	if ( changing ) return;
	int index = frameList->currentIndex();
	if ( index < 0 ) return;
	unsigned int numFrames;
	AnimationFrameRangeInfo* frameRange = preview.modelRenderable->geometryInfo->modelData->getAnimationFrameRange( &numFrames );

	frameRange[index].firstFrame = startFrame->value();
	endFrame->setMinimum( frameRange[index].firstFrame );
	preview.modelRenderable->parent->getComponent<Animator>()->play( frameList->currentIndex() );
}
void Editor::changeParamsEnd()
{
	if ( changing ) return;

	int index = frameList->currentIndex();
	if ( index < 0 ) return;
	unsigned int numFrames;
	AnimationFrameRangeInfo* frameRange = preview.modelRenderable->geometryInfo->modelData->getAnimationFrameRange( &numFrames );

	frameRange[index].lastFrame = endFrame->value();
	startFrame->setMaximum( frameRange[index].lastFrame );
	preview.modelRenderable->parent->getComponent<Animator>()->play( frameList->currentIndex() );
}

void Editor::selectModel( int selected )
{
	selected;
	if ( !editor || pmdEditorWidget->isHidden() ) return;
	if ( modelsList->currentRow() >= 0 ) preview.modelRenderable->geometryInfo = editor->getGeometry( ( uint ) modelsList->currentRow() );
	if ( texturesList->currentRow() >= 0 ) preview.modelRenderable->swapTexture(editor->getTexture( ( uint ) modelsList->currentRow() ), 0, 0);
}

void Editor::changeCardMode( bool isTrue )
{
	if ( isTrue )
	{
		arCardsWidget->show();
		selectCard( arCardsList->currentRow() );
	}
	else arCardsWidget->hide();
}
void Editor::changeModelMode( bool isTrue )
{
	if ( isTrue )
	{
		pmdEditorWidget->show();
		selectModel( 0 );
	}
	else pmdEditorWidget->hide();
}

Editor::~Editor()
{
	delete editor;
}