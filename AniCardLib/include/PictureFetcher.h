#include <qedit.h>
#include <AniCardLibExportHeader.h>
class ANICARDLIB_SHARED PictureFetcher : public ISampleGrabberCB
{
	AM_MEDIA_TYPE mediaType;
	unsigned char* picture;
	
	long width;
	long height;
	double SampleTime;
	unsigned int numUsing;
	bool canGrab;
public:
	long bufferLength;
	PictureFetcher( AM_MEDIA_TYPE mediaType );
	~PictureFetcher();
	bool getPicture( unsigned char** bytes , long* width = 0 , long* height = 0 );
	bool finishedUsing();
	STDMETHODIMP_( ULONG ) AddRef();
	STDMETHODIMP_( ULONG ) Release();

	STDMETHODIMP QueryInterface( REFIID riid , void **ppvObject );

	STDMETHODIMP SampleCB( double SampleTime , IMediaSample *pSample ) ;
	STDMETHODIMP BufferCB( double SampleTime , BYTE *pBuffer , long BufferLen );
};