#include <PictureFetcher.h>
#include <dshow.h>
STDMETHODIMP_( ULONG ) PictureFetcher::AddRef()
{
	return 1;
}
STDMETHODIMP_( ULONG ) PictureFetcher::Release()
{
	return 2;
}

PictureFetcher::PictureFetcher(AM_MEDIA_TYPE mediaType) : picture(0), numUsing(0) , canGrab(false)
{
	this->mediaType = mediaType;
}

bool PictureFetcher::getPicture( unsigned char** bytes , long* width , long* height )
{
	if ( canGrab && bytes )
	{
		++numUsing;
		*bytes = picture;
		if ( width ) *width = this->width;
		if ( height ) *height = this->height;
		return true;
	}
	return false;
}
bool PictureFetcher::finishedUsing()
{
	if ( numUsing > 0 ) { --numUsing; return true; }
	else return false;
}

STDMETHODIMP PictureFetcher::QueryInterface( REFIID riid , void **ppvObject )
{
	if ( !ppvObject ) return E_POINTER;
	if ( riid == __uuidof( IUnknown ) )
	{
		*ppvObject = static_cast< IUnknown* >( this );
		return S_OK;
	}
	if ( riid == IID_ISampleGrabberCB )
	{
		*ppvObject = static_cast< ISampleGrabberCB* >( this );
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP PictureFetcher::SampleCB( double SampleTime , IMediaSample *pSample )
{
	if ( numUsing ) return S_OK;
	canGrab = false;
	this->SampleTime = SampleTime;
	VIDEOINFOHEADER *head = ( VIDEOINFOHEADER* ) mediaType.pbFormat;
	byte* pBuffer;
	pSample->GetPointer(&pBuffer);
	
	long BufferLen = pSample->GetSize();
	if ( picture )
	{
		if ( width != head->bmiHeader.biWidth || height != head->bmiHeader.biHeight )
		{
			delete[] picture;
			picture = new byte[BufferLen];
		}
	}
	else picture = new byte[BufferLen];

	bufferLength = BufferLen;
	memcpy( picture , pBuffer , BufferLen );
	
	width = head->bmiHeader.biWidth;
	height = head->bmiHeader.biHeight;

	canGrab = true;
	return S_OK;
}
STDMETHODIMP PictureFetcher::BufferCB( double SampleTime , BYTE *pBuffer , long BufferLen )
{
	//if ( picture ) delete[] picture;
	//canGrab = false;
	//this->SampleTime = SampleTime;
	//picture = new byte[BufferLen];
	//bufferLength = BufferLen;
	//memcpy( picture , pBuffer , BufferLen );
	SampleTime;
	pBuffer;
	BufferLen;
	return E_NOTIMPL;
}

PictureFetcher::~PictureFetcher()
{
	numUsing = 0;
	while ( !canGrab )
	{
		numUsing = 0;
	}
	if ( picture )
	{
		delete[] picture;
	}
}