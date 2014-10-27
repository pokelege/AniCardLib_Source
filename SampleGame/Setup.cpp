#include "Setup.h"
#include <WebCamSource.h>
#include <QtGui\QComboBox>
#include <QtGui\QPushButton>
#include <QtGui\QVBoxLayout>
#include "War.h"
Setup::Setup()
{
	source = new WebCamSource;
	IEnumMoniker* cameraList;
	source->getListOfCameras( &cameraList );
	IMoniker* selected = 0;
	cameraSelection = new QComboBox;
	resolutionSelection = new QComboBox;
	QPushButton* button = new QPushButton( "Start" );
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget( cameraSelection );
	layout->addWidget( resolutionSelection );
	layout->addWidget( button );
	setLayout( layout );
	while ( cameraList->Next( 1 , &selected , NULL ) == S_OK )
	{
		IPropertyBag* properties;
		if ( FAILED( selected->BindToStorage( 0 , 0 , IID_PPV_ARGS( &properties ) ) ) )
		{
			selected->Release();
			continue;
		}

		VARIANT var;
		VariantInit( &var );

		if ( SUCCEEDED( properties->Read( L"FriendlyName" , &var , 0 ) ) )
		{
			cameras.push_back( selected );
			cameraSelection->addItem( QString::fromUtf16((ushort*)var.bstrVal) );
		}
		else
		{
			selected->Release();
		}
	}

	connect( cameraSelection , SIGNAL( activated( int ) ) , this , SLOT( selectedCamera( int ) ) );
	connect( button , SIGNAL( clicked( bool ) ) , this , SLOT( run() ) );
}

void Setup::selectedCamera(int selection)
{
	resolutions.clear();
	source->selectCamera( *cameras[selection], resolutions );
	resolutionSelection->clear();
	for ( unsigned int i = 0; i < resolutions.size(); ++i )
	{
		resolutionSelection->addItem( QString::number( resolutions[i].caps.MaxOutputSize.cx ) + "x" + QString::number( resolutions[i].caps.MaxOutputSize.cy ) );
	}
}

void Setup::run()
{
	if ( resolutionSelection->currentIndex() < 0 ) return;
	source->selectResolution( resolutions[resolutionSelection->currentIndex()] );
	source->initialize();
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