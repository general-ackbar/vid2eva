#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <alloc.h>
//#include "avi.h"
#include "eva.h"
#include "mono.h"
#include "video.h"


#define	VERSION	8


bool conv(char* lpszInFile, char* lpszOutFile, int nDstWidth, int nDstHeight, int nFps, int nStartFrame, int nOutFrames, bool bDither, bool bAdjust, bool bMono)
{
	bool	bResult = false;
	void*	lpTmp;
	void*	lpAudioTmp;
	int		nFrame;
	int		i;
	int		nFrameCnt = 0;
	int		nRate, nScale;
	int		nMakes = 0;
	int		nPcmCnt = 0;
	int		nPcmSize;
	int		nSrcWidth, nSrcHeight;

	video *v = new video(lpszInFile);
	//if(OpenAVI(lpszInFile)){
    if(true){
		if(CreateEVA(lpszOutFile, nFps != 10)){
            nFrame = v->getFrameCount(); // 10; // AVIGetVideoFrame();
            nRate = 5; //AVIGetVideoRate();
            nScale = 1; // AVIGetVideoScale();
            nSrcWidth = v->getWidth(); //AVIGetVideoWidth();
            nSrcHeight = v->getHeight(); //AVIGetVideoHeight();

			if(nDstHeight == 0 && nDstWidth == 0){
				nDstWidth = nSrcWidth;
				nDstHeight = nSrcHeight;
			} else if(nDstHeight == 0){
				if(bAdjust){
					nDstHeight = nDstWidth * nSrcHeight * 125 / (nSrcWidth * 100);
				} else {
					nDstHeight = nDstWidth * nSrcHeight / nSrcWidth;
				}
			} else if(nDstWidth == 0){
				if(bAdjust){
					nDstWidth = nDstHeight * nSrcWidth * 8 / (nSrcHeight * 10);
				} else {
					nDstWidth = nDstHeight * nSrcWidth / nSrcHeight;
				}
			} else {
				if(bAdjust){
					nDstHeight = nDstWidth * 125 / 100;
				}
			}
			v->prepareScaler(nDstWidth, nDstHeight);
			//AVISetVideoSize(nDstWidth, nDstHeight);

			i = nRate * 100 / nScale;
			fprintf(stdout, "%s : %dx%d / %d.%02d[Fps]\n", lpszInFile, nSrcWidth, nSrcHeight, i / 100, i % 100);
			fprintf(stdout, "%s : %dx%d / %d[Fps]\n", lpszOutFile, nDstWidth, nDstHeight, nFps);

			lpTmp = malloc(nDstWidth * nDstHeight * 3 + (15750 / nFps) + 1);
			if(lpTmp){
				lpAudioTmp = (void*)((unsigned char*)lpTmp + nDstWidth * nDstHeight * 3);
				for(i = nStartFrame; i < nFrame;){
					fprintf(stdout, "\x0d%d/%d(%d)",  i, nFrame-1, nMakes+1);

					/* PCM Size */
					nPcmCnt += 15750;
					nPcmSize = nPcmCnt / nFps;
					nPcmCnt %= nFps;

					/* Read AVI Video */
					int timestamp = i *  AV_TIME_BASE * (1/nFps);
					lpTmp = v->getFrameAt(timestamp);
                    /*
					if(!AVIReadVideo(lpTmp, i)){
						fprintf(stdout, "\n");
						fprintf(stderr, "AVI video read error\n");
						break;
					}
                    */
                    
					/* Read AVI Audio */
                    /*
					if(!AVIReadAudio(lpAudioTmp, nPcmSize)){
						fprintf(stdout, "\n");
						fprintf(stderr, "AVI audio read error\n");
						break;
					}
                    */
					/* Mono */
					if(bMono){
						if(!Mono((unsigned char*)lpTmp, nDstWidth, nDstHeight)){
							fprintf(stdout, "\n");
							fprintf(stderr, "mono error\n");
							break;
						}
					}

					/* Convert EVA */
					if(!ConvEva(lpTmp, nDstWidth, nDstHeight, lpAudioTmp, nPcmSize, bDither, bMono)){
						fprintf(stdout, "\n");
						fprintf(stderr, "EVA convert error\n");
						break;
					}
					if(!AppendEVA()){
						fprintf(stdout, "\n");
						fprintf(stderr, "EVA write error\n");
						break;
					}

					nMakes++;
					if(nOutFrames && nMakes >= nOutFrames) break;

					/* next Frame */
					nFrameCnt += nRate;
					if(nFrameCnt > nFps * nScale){
						i += nFrameCnt / (nFps * nScale);
						nFrameCnt %= nFps * nScale;
					}
				}

				if(i >= nFrame || nMakes >= nOutFrames){
					fprintf(stdout, "\n");
					bResult = true;
				}

				free(lpTmp);
			} else fprintf(stderr, "memory allocate error\n");
			CloseEVA();
		} else fprintf(stderr, "can not open '%s'\n", lpszOutFile);
		//CloseAVI();
	} else fprintf(stderr, "can not open '%s'\n", lpszInFile);
	return bResult;
}


void putUsage()
{
	fprintf(stdout,
			"\n"
			"usage : avi2eva [avi file] [eva file] [options]\n"
			"\n"
			"option: -FPSn     output frame rate\n"
			"        -VSIZEn   bitmap height\n"
			"        -HSIZEn   bitmap width\n"
			"        -FRAMEn   output frames\n"
			"        -STARTn   start AVI frame number\n"
			"        -DITHER   use dither pattern\n"
			"        -ADJUST   adjust aspect ratio\n"
			"        -MONO     output monochrome image\n"
	);
}


int main(int argc, char *argv[])
{
	int		i;
	char	*pstrInFile = NULL;
	char	*pstrOutFile = NULL;
	int		nFps = 1;
	int		nWidth = 0, nHeight = 0;
	int		nStart = 0, nOuts = 0;
	bool	bDither = false;
	bool	bAdjust = false;
	bool	bMono = false;

	fprintf(stdout, "AVI to EVA converter Version %d.%d, created by BUPPU.\n", VERSION>>8,VERSION&255);

	for(i = 1; i < argc; i++){
		if(*argv[i] == '-'){
			if(strcmp(argv[i]+1, "h") == 0){
				putUsage();
				return 0;
			} else if(strcmp(argv[i]+1,"help") == 0){
				putUsage();
				return 0;
			} else if(strcmp(argv[i]+1,"?") == 0){
				putUsage();
				return 0;
			} else if(strncmp(argv[i]+1,"dither", 6) == 0){
				bDither = true;
				continue;
			} else if(strncmp(argv[i]+1,"fps", 3) == 0){
				nFps = atoi(argv[i]+4);
				continue;
			} else if(strncmp(argv[i]+1, "vsize", 5) == 0){
				nHeight = atoi(argv[i]+6);
				continue;
			} else if(strncmp(argv[i]+1, "hsize", 5) == 0){
				nWidth = atoi(argv[i]+6);
				continue;
			} else if(strncmp(argv[i]+1, "start", 5) == 0){
				nStart = atoi(argv[i]+6);
				continue;
			} else if(strncmp(argv[i]+1, "frame", 5) == 0){
				nOuts = atoi(argv[i]+6);
				continue;
			} else if(strncmp(argv[i]+1, "adjust", 6) == 0){
				bAdjust = true;
				continue;
			} else if(strncmp(argv[i]+1, "mono", 6) == 0){
				bMono = true;
				continue;
			}
		} else {
			if(!pstrInFile){				
				pstrInFile = argv[i];
				continue;
			} else if(!pstrOutFile){
				pstrOutFile = argv[i];
				continue;
			}
		}

		fprintf(stderr, "unknown option '%s'\n", argv[i]);
		return 1;
	}

	if(pstrInFile == NULL || pstrOutFile == NULL){
		putUsage();
		return 1;
	}

	return conv(pstrInFile, pstrOutFile, nWidth, nHeight, nFps, nStart, nOuts, bDither, bAdjust, bMono) ? 0 : 1;
}
