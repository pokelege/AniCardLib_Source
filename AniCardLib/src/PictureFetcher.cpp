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

PictureFetcher::PictureFetcher(AM_MEDIA_TYPE mediaType) : picture(0)
{
	this->mediaType = mediaType;
	VIDEOINFOHEADER *head = ( VIDEOINFOHEADER* )mediaType.pbFormat;
	width = abs( head->rcSource.left - head->rcSource.right );
	height = abs( head->rcSource.top - head->rcSource.bottom );
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
	byte* pBuffer;
	pSample->GetPointer(&pBuffer);
	
	long BufferLen = pSample->GetSize();
	if ( picture ) delete[] picture;
	picture = new byte[BufferLen];
	bufferLength = BufferLen;
	memcpy( picture , pBuffer , BufferLen );
	return S_OK;
}
STDMETHODIMP PictureFetcher::BufferCB( double SampleTime , BYTE *pBuffer , long BufferLen )
{
	if ( picture ) delete[] picture;
	picture = new byte[BufferLen];
	bufferLength = BufferLen;
	memcpy( picture , pBuffer , BufferLen );
	return S_OK;
}