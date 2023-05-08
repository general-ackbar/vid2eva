#include <windows.h>
#include <mmsystem.h>
#include "stretch.h"
#include <vfw.h>
#include <alloc.h>
#include <stdio.h>

BOOL				bAVIInit;
PAVIFILE			pAviFile;
PAVISTREAM			pAviStreamVideo;
PAVISTREAM			pAviStreamAudio;
BITMAPINFOHEADER	biBitmap;
LONG				lStreamPosVideo;
LONG				lStreamPosAudio;
PGETFRAME			pGetFrame;
AVIFILEINFO			AviFileInfo;
LPWAVEFORMATEX		lpwfxWave;
HACMSTREAM			hasFormat;
HACMSTREAM			hasRate;
DWORD				dwWaveDstSize;
int					nWaveDstOfs;
LPVOID				lpWaveDst;
int					nDstWidth, nDstHeight;


BOOL setDummyAudio()
{
	dwWaveDstSize = 16384;
	lpWaveDst = malloc(dwWaveDstSize);
	if(lpWaveDst == NULL){
		fprintf(stderr, "can not allocate memory\n");
		return FALSE;
	}
	memset(lpWaveDst, 0x80, dwWaveDstSize);
	return TRUE;
}


BOOL readconvAudio()
{
	LPVOID				lpTmp, lpTmp2;
	long				leng, sample;
	ACMSTREAMHEADER		ash;
	HRESULT				hr;
	DWORD				dwSize;

	/* とりあえず開放 */
	if(lpWaveDst){
		free(lpWaveDst);
		lpWaveDst = NULL;
		dwWaveDstSize = 0;
	}

	/* ダミーを返す */
	if(pAviStreamAudio == NULL){
		return setDummyAudio();
	} else if(lStreamPosAudio >= AVIStreamEnd(pAviStreamAudio)){
		return setDummyAudio();
	}

	/* サイズ得る */
	hr = AVIStreamRead(pAviStreamAudio, lStreamPosAudio, AVISTREAMREAD_CONVENIENT, NULL, 0, &leng, &sample);
	if(hr){
		lStreamPosAudio = AVIStreamEnd(pAviStreamAudio);
		return setDummyAudio();
	}

	/* バッファ確保 */
	lpTmp = malloc(leng);
	if(lpTmp == NULL){
		fprintf(stderr, "can not allocate memory\n");
		return FALSE;
	}

	/* ストリーム読み込み */
	hr = AVIStreamRead(pAviStreamAudio, lStreamPosAudio, sample, lpTmp, leng, NULL, NULL);
	if(hr){
		free(lpTmp);
		fprintf(stderr, "can not read audio stream\n");
		return FALSE;
	}

	if(hasFormat){
		/* 変換後のサイズを得る */
		hr = acmStreamSize(hasFormat, leng, &dwSize, ACM_STREAMSIZEF_SOURCE);
		if(hr){
			free(lpTmp);
			fprintf(stderr, "can not get convert stream size\n");
			return FALSE;
		}
		/* 変換後の格納バッファを確保 */
		lpTmp2 = malloc(dwSize);
		if(lpTmp2==NULL){
			free(lpTmp);
			fprintf(stderr, "can not allocate memory\n");
			return FALSE;
		}
		/* バッファの準備 */
		ZeroMemory(&ash, sizeof(ash));
		ash.cbStruct		= sizeof(ash);
		ash.fdwStatus		= 0;
		ash.dwUser			= 0;
		ash.pbSrc			= (LPBYTE)lpTmp;
		ash.cbSrcLength		= leng;
		ash.cbSrcLengthUsed	= 0;
		ash.dwSrcUser		= 0;
		ash.pbDst			= (LPBYTE)lpTmp2;
		ash.cbDstLength		= dwSize;
		ash.cbDstLengthUsed	= 0;
		ash.dwDstUser		= 0;
		hr = acmStreamPrepareHeader(hasFormat, &ash, 0);
		if(hr){
			free(lpTmp2);
			free(lpTmp);
			fprintf(stderr, "prepare header error\n");
			return FALSE;
		}
		/* 変換 */
		hr = acmStreamConvert(hasFormat, &ash, 0);
		if(hr){
			acmStreamUnprepareHeader(hasFormat, &ash, 0);
			free(lpTmp2);
			free(lpTmp);
			fprintf(stderr, "convert error\n");
			return FALSE;
		}
		/* バッファの後始末 */
		acmStreamUnprepareHeader(hasFormat, &ash, 0);
		/* 入れ替え */
		free(lpTmp);
		lpTmp = lpTmp2;
		leng = ash.cbDstLengthUsed;
	}

	/* 変換後のサイズを得る */
	hr = acmStreamSize(hasRate, leng, &dwSize, ACM_STREAMSIZEF_SOURCE);
	if(hr){
		free(lpTmp);
		fprintf(stderr, "can not get convert stream size\n");
		return FALSE;
	}

	/* バッファ確保 */
	lpWaveDst = malloc(dwSize);
	if(lpWaveDst==NULL){
		free(lpTmp);
		fprintf(stderr, "can not allocate memory\n");
		return FALSE;
	}

	/* バッファ準備 */
	ZeroMemory(&ash, sizeof(ash));
	ash.cbStruct		= sizeof(ash);
	ash.fdwStatus		= 0;
	ash.dwUser			= 0;
	ash.pbSrc			= (LPBYTE)lpTmp;
	ash.cbSrcLength		= leng;
	ash.cbSrcLengthUsed	= 0;
	ash.dwSrcUser		= 0;
	ash.pbDst			= (LPBYTE)lpWaveDst;
	ash.cbDstLength		= dwSize;
	ash.cbDstLengthUsed	= 0;
	ash.dwDstUser		= 0;
	hr = acmStreamPrepareHeader(hasRate, &ash, 0);
	if(hr){
		free(lpWaveDst);
		lpWaveDst = NULL;
		free(lpTmp);
		fprintf(stderr, "prepare header error\n");
		return FALSE;
	}

	/* 変換 */
	hr = acmStreamConvert(hasRate, &ash, 0);
	if(hr){
		acmStreamUnprepareHeader(hasRate, &ash, 0);
		free(lpWaveDst);
		lpWaveDst = NULL;
		free(lpTmp);
		fprintf(stderr, "convert error\n");
		return FALSE;
	}

	/* 後始末 */
	acmStreamUnprepareHeader(hasRate, &ash, 0);

	free(lpTmp);

	dwWaveDstSize = ash.cbDstLengthUsed;
	lStreamPosAudio += sample;
	return TRUE;
}




BOOL CloseAVI()
{
	if(lpWaveDst){
		free(lpWaveDst);
		lpWaveDst = NULL;
	}
	if(hasRate){
		acmStreamClose(hasRate, 0);
		hasRate = NULL;
	}
	if(hasFormat){
		acmStreamClose(hasFormat, 0);
		hasFormat = NULL;
	}
	if(lpwfxWave){
		free(lpwfxWave);
		lpwfxWave = NULL;
	}
	if(pAviStreamAudio){
		AVIStreamRelease(pAviStreamAudio);
		pAviStreamAudio = NULL;
	}
	if(pGetFrame){
		AVIStreamGetFrameClose(pGetFrame);
		pGetFrame = NULL;
	}
	if(pAviStreamVideo){
		AVIStreamRelease(pAviStreamVideo);
		pAviStreamVideo = NULL;
	}
	if(pAviFile){
		AVIFileRelease(pAviFile);
		pAviFile = NULL;
	}
	if(bAVIInit){
		AVIFileExit();
		bAVIInit = FALSE;
	}
	return TRUE;
}


BOOL OpenAVI(LPSTR lpszFile)
{
	HRESULT			hr; 
	LONG			lStreamSize; 
	WAVEFORMATEX	wfxDst;
	WAVEFORMATEX	wfxDst2;

	CloseAVI();

	/* AVI File を開く */
	AVIFileInit();
	bAVIInit = TRUE;
	hr = AVIFileOpen(&pAviFile, lpszFile, 0, 0L);
	if(hr != 0){
fprintf(stderr, "can not open %s\n", lpszFile);
		CloseAVI();
		return FALSE;
	}

	/* AVI file お情報 */
	if(AVIFileInfo(pAviFile, &AviFileInfo, sizeof(AviFileInfo))){
fprintf(stderr, "can not get AVI info\n");
		CloseAVI();
		return FALSE;
	}

	/* VIDEO ストリームハンドルを得る */
	hr = AVIFileGetStream(pAviFile, &pAviStreamVideo, streamtypeVIDEO, 0);
	if(hr != 0){
fprintf(stderr, "can not get video stream\n");
		CloseAVI();
		return FALSE;
	}

	/* bitmap format を得る */
	lStreamSize = sizeof(biBitmap);
	hr = AVIStreamReadFormat(pAviStreamVideo, 0, &biBitmap, &lStreamSize);
	if(hr != 0){
fprintf(stderr, "can not get video format\n");
		CloseAVI();
		return FALSE;
	}

	/* Decode 済みのビットマップを得る準備 */
	biBitmap.biSize = sizeof(biBitmap);
	//biBitmap.biWidth = 128;
	//biBitmap.biHeight = 106;
	biBitmap.biPlanes = 1;
	biBitmap.biBitCount = 24;
	biBitmap.biCompression = BI_RGB;
	biBitmap.biSizeImage = 0;
	//biBitmap.biXPelsPerMeter = 0;
	//biBitmap.biYPelsPerMeter = 0;
	biBitmap.biClrUsed = 0;
	biBitmap.biClrImportant = 0;
	pGetFrame = AVIStreamGetFrameOpen(pAviStreamVideo, &biBitmap);
	if(pGetFrame == NULL){
fprintf(stderr, "can not open get frame\n");
		CloseAVI();
		return FALSE;
	}

	/* AUDIO ストリームハンドルを得る */
	hr = AVIFileGetStream(pAviFile, &pAviStreamAudio, streamtypeAUDIO, 0);
	if(hr == 0){

		/* Wave format のサイズを得る */
		hr = AVIStreamFormatSize(pAviStreamAudio, 0, &lStreamSize);
		if(hr != 0){
fprintf(stderr, "can not get audio format size\n");
			CloseAVI();
			return FALSE;
		}

		/* Wave format を確保する */
		lpwfxWave = (LPWAVEFORMATEX)malloc(lStreamSize > sizeof(WAVEFORMATEX) ? lStreamSize : sizeof(WAVEFORMATEX));
		if(lpwfxWave == NULL){
fprintf(stderr, "can not allocate WAVEFORMAT\n");
			CloseAVI();
			return FALSE;
		}
		ZeroMemory(lpwfxWave, lStreamSize > sizeof(WAVEFORMATEX) ? lStreamSize : sizeof(WAVEFORMATEX));

		/* WAVE format を得る */
		hr = AVIStreamReadFormat(pAviStreamAudio, AVIStreamStart(pAviStreamAudio), lpwfxWave, &lStreamSize);
		if(hr != 0){
fprintf(stderr, "can not get audio format\n");
			CloseAVI();
			return FALSE;
		}

#ifdef DEBUG
fprintf(stdout, "FormatTag = %d\n", lpwfxWave->wFormatTag);
fprintf(stdout, "Channels = %d\n", lpwfxWave->nChannels);
fprintf(stdout, "SamplesPerSec = %d\n", lpwfxWave->nSamplesPerSec);
fprintf(stdout, "AvgBytesPerSec = %d\n", lpwfxWave->nAvgBytesPerSec);
fprintf(stdout, "BlockAlign = %d\n", lpwfxWave->nBlockAlign);
fprintf(stdout, "BitsPerSample = %d\n", lpwfxWave->wBitsPerSample);
fprintf(stdout, "cbSize = %d\n", lpwfxWave->cbSize);
#endif

		wfxDst.wFormatTag = WAVE_FORMAT_PCM;
		wfxDst.nChannels = 1;
		wfxDst.wBitsPerSample = 16;
		wfxDst.nSamplesPerSec = lpwfxWave->nSamplesPerSec;
		wfxDst.nAvgBytesPerSec = wfxDst.nSamplesPerSec * wfxDst.nChannels * (wfxDst.wBitsPerSample / 8);
		wfxDst.nBlockAlign = wfxDst.nChannels * (wfxDst.wBitsPerSample / 8);
		wfxDst.cbSize = 0;
#ifdef DEBUG
fprintf(stdout, "\nFormatTag = %d\n", wfxDst.wFormatTag);
fprintf(stdout, "Channels = %d\n", wfxDst.nChannels);
fprintf(stdout, "SamplesPerSec = %d\n", wfxDst.nSamplesPerSec);
fprintf(stdout, "AvgBytesPerSec = %d\n", wfxDst.nAvgBytesPerSec);
fprintf(stdout, "BlockAlign = %d\n", wfxDst.nBlockAlign);
fprintf(stdout, "BitsPerSample = %d\n", wfxDst.wBitsPerSample);
fprintf(stdout, "cbSize = %d\n", wfxDst.cbSize);
#endif

		wfxDst2.wFormatTag = WAVE_FORMAT_PCM;
		wfxDst2.nChannels = 1;
		wfxDst2.nSamplesPerSec = 15750;
		wfxDst2.nAvgBytesPerSec = wfxDst2.nSamplesPerSec;
		wfxDst2.nBlockAlign = 1;
		wfxDst2.wBitsPerSample = 8;
		wfxDst2.cbSize = 0;

		/* 直接変換できる？ */
		hr = acmStreamOpen(&hasRate, NULL, lpwfxWave, &wfxDst2, NULL, 0, 0, ACM_STREAMOPENF_QUERY);
		if(!hr){
			/* 直接変換する */
			hr = acmStreamOpen(&hasRate, NULL, lpwfxWave, &wfxDst2, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME);
			if(hr){
				fprintf(stderr, "can not open acm stream\n");
				CloseAVI();
				return FALSE;
			}
		} else {
			/* まず liner PCM へ変換 */
			hr = acmStreamOpen(&hasFormat, NULL, lpwfxWave, &wfxDst, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME);
			if(hr){
				fprintf(stderr, "can not open acm stream1 %d %d\n", hr,ACMERR_NOTPOSSIBLE	);
				CloseAVI();
				return FALSE;
			}
			/* 再生レートと量子幅を変換 */
			hr = acmStreamOpen(&hasRate, NULL, &wfxDst, &wfxDst2, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME);
			if(hr){
				fprintf(stderr, "can not open acm stream2\n");
				CloseAVI();
				return FALSE;
			}
		}
	} else {
		pAviStreamAudio = NULL;
fprintf(stdout, "can not get audio stream\n");
	}

	/* 始めのストリーム位置 */
	lStreamPosVideo = 0;
	if(pAviStreamAudio != NULL) lStreamPosAudio = AVIStreamStart(pAviStreamAudio);

	dwWaveDstSize = nWaveDstOfs = 0;
fprintf(stderr,"ok\n");
	return TRUE;
}


void AVISetVideoSize(int nWidth, int nHeight)
{
	nDstWidth = nWidth;
	nDstHeight = nHeight;
	SetStretchPer(nDstWidth, nDstHeight, biBitmap.biWidth, biBitmap.biHeight);
}


int AVIGetVideoFrame()
{
	return AVIStreamEnd(pAviStreamVideo);
}


int AVIGetVideoRate()
{
	return AviFileInfo.dwRate;
}


int AVIGetVideoScale()
{
	return AviFileInfo.dwScale;
}


int AVIGetVideoWidth()
{
	return biBitmap.biWidth;
}


int AVIGetVideoHeight()
{
	return biBitmap.biHeight;
}


BOOL AVIIsEndAudio()
{
	if(pAviStreamAudio == NULL) return TRUE;
	return lStreamPosAudio == AVIStreamEnd(pAviStreamAudio);
}


BOOL AVIReadVideo(LPVOID lpDst, int nFrame)
{
	LPBITMAPINFOHEADER		pTmp;

	pTmp = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pGetFrame, nFrame);
	return Stretch((LPBYTE)lpDst, (LPBYTE)(pTmp + 1));
}


BOOL AVIReadAudio(LPVOID lpDst, int nSize)
{
	int		nRead;

	while(nSize>0){
		nRead = nSize;
		if(nRead > dwWaveDstSize - nWaveDstOfs) nRead = dwWaveDstSize - nWaveDstOfs;
		if(nRead > 0){
			CopyMemory(lpDst, (LPBYTE)lpWaveDst + nWaveDstOfs, nRead);
			nWaveDstOfs += nRead;
			nSize -= nRead;
			(LPBYTE)lpDst += nRead;
		}
		if(nWaveDstOfs >= dwWaveDstSize){
			if(!readconvAudio()) return FALSE;
			nWaveDstOfs = 0;
		}
	}
	return TRUE;
}


