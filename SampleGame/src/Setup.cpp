#include "Setup.h"
#include <WebCamSource.h>
#include <QtGui\QComboBox>
#include <QtGui\QPushButton>
#include <QtGui\QVBoxLayout>
#include "War.h"
Setup::Setup()
{
	source = new WebCamSource;

	WebCamHelpers helper;
	cameras = helper.getListOfCameras();
	cameraSelection = new QComboBox;
	resolutionSelection = new QComboBox;
	QPushButton* button = new QPushButton( "Start" );
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget( cameraSelection );
	layout->addWidget( resolutionSelection );
	layout->addWidget( button );
	setLayout( layout );

	for ( unsigned int i = 0; i < cameras.size(); ++i )
	{
		cameraSelection->addItem( cameras.at(i).name.c_str() );
	}
	cameraSelection->setCurrentIndex( -1 );
	connect( cameraSelection , SIGNAL( activated( int ) ) , this , SLOT( selectedCamera( int ) ) );
	connect( button , SIGNAL( clicked( bool ) ) , this , SLOT( run() ) );
}

void Setup::selectedCamera(int selection)
{
	resolutionSelection->clear();
	for ( unsigned int i = 0; i < cameras.at(selection).modes.size(); ++i )
	{
		resolutionSelection->addItem( cameras.at(selection ).modes.at(i).name.c_str());
	}
}

void Setup::run()
{
	if ( resolutionSelection->currentIndex() < 0 ) return;
	source->initialize( cameras.at( cameraSelection->currentIndex() ) , cameras.at( cameraSelection->currentIndex() ).modes.at(resolutionSelection->currentIndex()) );
	War* war = new War;
	war->setAttribute( Qt::WA_DeleteOnClose );
	war->setCameraSource( source );
	war->show();
	source = 0;
	close();
}

Setup::~Setup()
{
	if ( source ) delete source;
}