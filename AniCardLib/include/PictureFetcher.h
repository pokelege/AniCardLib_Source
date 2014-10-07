#include <qedit.h>
class PictureFetcher : public ISampleGrabberCB
{
	AM_MEDIA_TYPE mediaType;
public:
	byte* picture;
	long bufferLength;
	long width;
	long height;
	bool canGrab;
	PictureFetcher( AM_MEDIA_TYPE mediaType );
	STDMETHODIMP_( ULONG ) AddRef();
	STDMETHODIMP_( ULONG ) Release();

	STDMETHODIMP QueryInterface( REFIID riid , void **ppvObject );

	STDMETHODIMP SampleCB( double SampleTime , IMediaSample *pSample ) ;
	STDMETHODIMP BufferCB( double SampleTime , BYTE *pBuffer , long BufferLen );
};