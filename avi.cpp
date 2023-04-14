//#include <windows.h>
//#include <mmsystem.h>
#include "stretch.h"
//#include <vfw.h>
//#include <alloc.h>
#include <stdio.h>

bool				bAVIInit;
PAVIFILE			pAviFile;
PAVISTREAM			pAviStreamVideo;
PAVISTREAM			pAviStreamAudio;
BITMAPINFOHEADER	biBitmap;
long				lStreamPosVideo;
long				lStreamPosAudio;
PGETFRAME			pGetFrame;
AVIFILEINFO			AviFileInfo;
LPWAVEFORMATEX		lpwfxWave;
HACMSTREAM			hasFormat;
HACMSTREAM			hasRate;
unsigned long				dwWaveDstSize;
int					nWaveDstOfs;
void*				lpWaveDst;
int					nDstWidth, nDstHeight;


bool setDummyAudio()
{
	dwWaveDstSize = 16384;
	lpWaveDst = malloc(dwWaveDstSize);
	if(lpWaveDst == NULL){
		fprintf(stderr, "can not allocate memory\n");
		return false;
	}
	memset(lpWaveDst, 0x80, dwWaveDstSize);
	return true;
}


bool readconvAudio()
{
	void*				lpTmp, lpTmp2;
	long				leng, sample;
	ACMSTREAMHEADER		ash;
	HRESULT				hr;
	unsigned long				dwSize;

	/* �Ƃ肠�����J�� */
	if(lpWaveDst){
		free(lpWaveDst);
		lpWaveDst = NULL;
		dwWaveDstSize = 0;
	}

	/* �_�~�[��Ԃ� */
	if(pAviStreamAudio == NULL){
		return setDummyAudio();
	} else if(lStreamPosAudio >= AVIStreamEnd(pAviStreamAudio)){
		return setDummyAudio();
	}

	/* �T�C�Y���� */
	hr = AVIStreamRead(pAviStreamAudio, lStreamPosAudio, AVISTREAMREAD_CONVENIENT, NULL, 0, &leng, &sample);
	if(hr){
		lStreamPosAudio = AVIStreamEnd(pAviStreamAudio);
		return setDummyAudio();
	}

	/* �o�b�t�@�m�� */
	lpTmp = malloc(leng);
	if(lpTmp == NULL){
		fprintf(stderr, "can not allocate memory\n");
		return false;
	}

	/* �X�g���[���ǂݍ��� */
	hr = AVIStreamRead(pAviStreamAudio, lStreamPosAudio, sample, lpTmp, leng, NULL, NULL);
	if(hr){
		free(lpTmp);
		fprintf(stderr, "can not read audio stream\n");
		return false;
	}

	if(hasFormat){
		/* �ϊ���̃T�C�Y�𓾂� */
		hr = acmStreamSize(hasFormat, leng, &dwSize, ACM_STREAMSIZEF_SOURCE);
		if(hr){
			free(lpTmp);
			fprintf(stderr, "can not get convert stream size\n");
			return false;
		}
		/* �ϊ���̊i�[�o�b�t�@���m�� */
		lpTmp2 = malloc(dwSize);
		if(lpTmp2==NULL){
			free(lpTmp);
			fprintf(stderr, "can not allocate memory\n");
			return false;
		}
		/* �o�b�t�@�̏��� */
		ZeroMemory(&ash, sizeof(ash));
		ash.cbStruct		= sizeof(ash);
		ash.fdwStatus		= 0;
		ash.dwUser			= 0;
		ash.pbSrc			= (unsigned char*)lpTmp;
		ash.cbSrcLength		= leng;
		ash.cbSrcLengthUsed	= 0;
		ash.dwSrcUser		= 0;
		ash.pbDst			= (unsigned char*)lpTmp2;
		ash.cbDstLength		= dwSize;
		ash.cbDstLengthUsed	= 0;
		ash.dwDstUser		= 0;
		hr = acmStreamPrepareHeader(hasFormat, &ash, 0);
		if(hr){
			free(lpTmp2);
			free(lpTmp);
			fprintf(stderr, "prepare header error\n");
			return false;
		}
		/* �ϊ� */
		hr = acmStreamConvert(hasFormat, &ash, 0);
		if(hr){
			acmStreamUnprepareHeader(hasFormat, &ash, 0);
			free(lpTmp2);
			free(lpTmp);
			fprintf(stderr, "convert error\n");
			return false;
		}
		/* �o�b�t�@�̌�n�� */
		acmStreamUnprepareHeader(hasFormat, &ash, 0);
		/* ����ւ� */
		free(lpTmp);
		lpTmp = lpTmp2;
		leng = ash.cbDstLengthUsed;
	}

	/* �ϊ���̃T�C�Y�𓾂� */
	hr = acmStreamSize(hasRate, leng, &dwSize, ACM_STREAMSIZEF_SOURCE);
	if(hr){
		free(lpTmp);
		fprintf(stderr, "can not get convert stream size\n");
		return false;
	}

	/* �o�b�t�@�m�� */
	lpWaveDst = malloc(dwSize);
	if(lpWaveDst==NULL){
		free(lpTmp);
		fprintf(stderr, "can not allocate memory\n");
		return false;
	}

	/* �o�b�t�@���� */
	ZeroMemory(&ash, sizeof(ash));
	ash.cbStruct		= sizeof(ash);
	ash.fdwStatus		= 0;
	ash.dwUser			= 0;
	ash.pbSrc			= (unsigned char*)lpTmp;
	ash.cbSrcLength		= leng;
	ash.cbSrcLengthUsed	= 0;
	ash.dwSrcUser		= 0;
	ash.pbDst			= (unsigned char*)lpWaveDst;
	ash.cbDstLength		= dwSize;
	ash.cbDstLengthUsed	= 0;
	ash.dwDstUser		= 0;
	hr = acmStreamPrepareHeader(hasRate, &ash, 0);
	if(hr){
		free(lpWaveDst);
		lpWaveDst = NULL;
		free(lpTmp);
		fprintf(stderr, "prepare header error\n");
		return false;
	}

	/* �ϊ� */
	hr = acmStreamConvert(hasRate, &ash, 0);
	if(hr){
		acmStreamUnprepareHeader(hasRate, &ash, 0);
		free(lpWaveDst);
		lpWaveDst = NULL;
		free(lpTmp);
		fprintf(stderr, "convert error\n");
		return false;
	}

	/* ��n�� */
	acmStreamUnprepareHeader(hasRate, &ash, 0);

	free(lpTmp);

	dwWaveDstSize = ash.cbDstLengthUsed;
	lStreamPosAudio += sample;
	return true;
}




bool CloseAVI()
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
		bAVIInit = false;
	}
	return true;
}


bool OpenAVI(char* lpszFile)
{
	HRESULT			hr; 
	long			lStreamSize; 
	WAVEFORMATEX	wfxDst;
	WAVEFORMATEX	wfxDst2;

	CloseAVI();

	/* AVI File ���J�� */
	AVIFileInit();
	bAVIInit = true;
	hr = AVIFileOpen(&pAviFile, lpszFile, 0, 0L);
	if(hr != 0){
fprintf(stderr, "can not open %s\n", lpszFile);
		CloseAVI();
		return false;
	}

	/* AVI file ����� */
	if(AVIFileInfo(pAviFile, &AviFileInfo, sizeof(AviFileInfo))){
fprintf(stderr, "can not get AVI info\n");
		CloseAVI();
		return false;
	}

	/* VIDEO �X�g���[���n���h���𓾂� */
	hr = AVIFileGetStream(pAviFile, &pAviStreamVideo, streamtypeVIDEO, 0);
	if(hr != 0){
fprintf(stderr, "can not get video stream\n");
		CloseAVI();
		return false;
	}

	/* bitmap format �𓾂� */
	lStreamSize = sizeof(biBitmap);
	hr = AVIStreamReadFormat(pAviStreamVideo, 0, &biBitmap, &lStreamSize);
	if(hr != 0){
fprintf(stderr, "can not get video format\n");
		CloseAVI();
		return false;
	}

	/* Decode �ς݂̃r�b�g�}�b�v�𓾂鏀�� */
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
		return false;
	}

	/* AUDIO �X�g���[���n���h���𓾂� */
	hr = AVIFileGetStream(pAviFile, &pAviStreamAudio, streamtypeAUDIO, 0);
	if(hr == 0){

		/* Wave format �̃T�C�Y�𓾂� */
		hr = AVIStreamFormatSize(pAviStreamAudio, 0, &lStreamSize);
		if(hr != 0){
fprintf(stderr, "can not get audio format size\n");
			CloseAVI();
			return false;
		}

		/* Wave format ���m�ۂ��� */
		lpwfxWave = (LPWAVEFORMATEX)malloc(lStreamSize > sizeof(WAVEFORMATEX) ? lStreamSize : sizeof(WAVEFORMATEX));
		if(lpwfxWave == NULL){
fprintf(stderr, "can not allocate WAVEFORMAT\n");
			CloseAVI();
			return false;
		}
		ZeroMemory(lpwfxWave, lStreamSize > sizeof(WAVEFORMATEX) ? lStreamSize : sizeof(WAVEFORMATEX));

		/* WAVE format �𓾂� */
		hr = AVIStreamReadFormat(pAviStreamAudio, AVIStreamStart(pAviStreamAudio), lpwfxWave, &lStreamSize);
		if(hr != 0){
fprintf(stderr, "can not get audio format\n");
			CloseAVI();
			return false;
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

		/* ���ڕϊ��ł���H */
		hr = acmStreamOpen(&hasRate, NULL, lpwfxWave, &wfxDst2, NULL, 0, 0, ACM_STREAMOPENF_QUERY);
		if(!hr){
			/* ���ڕϊ����� */
			hr = acmStreamOpen(&hasRate, NULL, lpwfxWave, &wfxDst2, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME);
			if(hr){
				fprintf(stderr, "can not open acm stream\n");
				CloseAVI();
				return false;
			}
		} else {
			/* �܂� liner PCM �֕ϊ� */
			hr = acmStreamOpen(&hasFormat, NULL, lpwfxWave, &wfxDst, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME);
			if(hr){
				fprintf(stderr, "can not open acm stream1 %d %d\n", hr,ACMERR_NOTPOSSIBLE	);
				CloseAVI();
				return false;
			}
			/* �Đ����[�g�Ɨʎq����ϊ� */
			hr = acmStreamOpen(&hasRate, NULL, &wfxDst, &wfxDst2, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME);
			if(hr){
				fprintf(stderr, "can not open acm stream2\n");
				CloseAVI();
				return false;
			}
		}
	} else {
		pAviStreamAudio = NULL;
fprintf(stdout, "can not get audio stream\n");
	}

	/* �n�߂̃X�g���[���ʒu */
	lStreamPosVideo = 0;
	if(pAviStreamAudio != NULL) lStreamPosAudio = AVIStreamStart(pAviStreamAudio);

	dwWaveDstSize = nWaveDstOfs = 0;
fprintf(stderr,"ok\n");
	return true;
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


bool AVIIsEndAudio()
{
	if(pAviStreamAudio == NULL) return true;
	return lStreamPosAudio == AVIStreamEnd(pAviStreamAudio);
}


bool AVIReadVideo(void* lpDst, int nFrame)
{
	LPBITMAPINFOHEADER		pTmp;

	pTmp = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pGetFrame, nFrame);
	return Stretch((unsigned char*)lpDst, (unsigned char*)(pTmp + 1));
}


bool AVIReadAudio(void* lpDst, int nSize)
{
	int		nRead;

	while(nSize>0){
		nRead = nSize;
		if(nRead > dwWaveDstSize - nWaveDstOfs) nRead = dwWaveDstSize - nWaveDstOfs;
		if(nRead > 0){
			CopyMemory(lpDst, (unsigned char*)lpWaveDst + nWaveDstOfs, nRead);
			nWaveDstOfs += nRead;
			nSize -= nRead;
			(unsigned char*)lpDst += nRead;
		}
		if(nWaveDstOfs >= dwWaveDstSize){
			if(!readconvAudio()) return false;
			nWaveDstOfs = 0;
		}
	}
	return true;
}


