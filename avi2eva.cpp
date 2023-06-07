/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
*/

#include "eva.h"
#include "imageops.h"
#include "video.h"


#define	VERSION	1



bool convert(char* infile, char* outfile, int fps, bool compress, DitherMode ditherMode)
{
	uint8_t*	pImageFrame;
	uint8_t*	pAudioFrame;
	bool	debugmode = false;
	int		noOfFrames;
	int 	width = XSIZE*2;
	int		height = YSIZE*2;
	double 	srcFPS;
	int		noOfMakes = 0;
	int		nPcmSize = 0;
	int		srcWidth, srcHeight;
	float	duration = 0.0;

	Video *video = new Video(infile);
    noOfFrames = video->getFrameCount();
	srcFPS = video->getFramerate();
    srcWidth = video->getWidth();
    srcHeight = video->getHeight();
	duration = video->getLength();

	if(fps == 0)  fps = srcFPS;
	if(fps != srcFPS)
	{
		noOfFrames = duration * fps;
	}

	video->prepareScaler(width, height); //(256 colors, 192x184)
	video->decodeAllFrames();
	
	int decodedFrames = video->getFrames().capacity();
	
	 	
	fprintf(stdout, "%s : %dx%d / %0.2f[Fps], frames : %i, length : %0.2f (sec)\n", infile, srcWidth, srcHeight, srcFPS, video->getFrameCount(), video->getLength() );
	fprintf(stdout, "%s : %dx%d / %d[Fps], frames : %i, length : %0.2f (sec)\n", outfile, width, height, fps, noOfFrames, (noOfFrames / (float)fps));



	Eva *eva = new Eva(fps);

	pImageFrame = (uint8_t*)malloc(width * height * 3);

	//No idea of audio buffer yet
	//lpAudioTmp = (void*)((unsigned char*)lpTmp + nDstWidth * nDstHeight * 3);

				
	
	float frameStep = (decodedFrames / duration) / fps;
	for(float i = 0.0; i < decodedFrames; i += frameStep )
	{					
		int frameIdx = floor(i);

		//Read AVI Video
		pImageFrame = video->getFrames()[frameIdx];
		if(!pImageFrame)
		{ 
			printf("Error decoding frame %i", frameIdx);
			return -1;
		}
		//pAudioFrame = v->getAudioFrames()[frameIdx];

					
		uint8_t* pConvertedFrame = (uint8_t*)malloc(FRAMESIZE);
		pConvertedFrame = eva->encodeFrame(pImageFrame, width, height, ditherMode);
		if(debugmode)
		{
			SavePPM(pImageFrame, width, height);
			SaveEVA(pConvertedFrame);
		}
		
		uint8_t *pDst =  (uint8_t*)malloc(FRAMESIZE);
		memcpy(pDst, pConvertedFrame, FRAMESIZE);
		eva->appendFrame(pDst);
		
		noOfMakes++;
		free(pConvertedFrame);
		//free(pDst);
					
	}
				
	if(noOfMakes == eva->getFrameCount())
		printf("Re-encoded %i frames\n", eva->getFrameCount());
	else
		printf("Error re-encodeding: %i frames encoded, should be %i\n", noOfMakes, eva->getFrameCount());

	//Write file to disk
	if(compress)
		eva->exportEva4(outfile);
	else			
		eva->exportEva3(outfile);

	
/*
	nPcmSize = v->getSampleRate() / 2 / fps;
	printf("pcm: %i, fps: %i\n", nPcmSize, fps);		    
*/
	free(pImageFrame);	
	//free(pAudioFrame);
	free(eva);
	
	
	return true;
}


void putUsage()
{
	fprintf(stdout,
			"\n"
			"usage : avi2eva -i [video file] -o [eva file] [options]\n"
			"\n"
			"option: -r (x)    output framerate\n"
			"        -n		   don't dither output\n"
			"        -c		   compress frames (EVA4)\n"
			"        -d	(x)	   dither mode\n"
			"        		   0=FloydSteinberg, 1=Sierra, 2=Bayer2, 3=Bayer3\n"
			"        		   4=Bayer4, 5=Bayer8, 6=Bayer16, 7=None\n"
	);
}


int main(int argc, char *argv[])
{
	char	*pstrInFile = NULL;
	char	*pstrOutFile = NULL;
	int		nFps = 0;	
	bool	compress = false;
	bool	outputframes = false;
	DitherMode	ditherMode = FloydSteinberg;


	fprintf(stdout, "Video to EVA converter Version %d.%d, created by Codeninja.\n", VERSION>>8,VERSION&255);
	char c;
    while ((c = getopt (argc, argv, "i:o:r:d:nc")) != -1)
            switch (c)
        {
            case 'i':
                pstrInFile = optarg;
                break;
            case 'o':
                pstrOutFile = optarg;
                break;
            case 'r':
                nFps = atoi(optarg);
                break;
            case 'n':
                ditherMode = None;
                break;
			case 'd':
				ditherMode = DitherMode( atoi(optarg) );
				break;
			case 'c':
				compress = true;
				break;
			case 'v':
				outputframes = true;
				break;
            case '?':
                if (optopt == 'i' || optopt == 'o' || optopt == 'r')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                return 1;
            default:
                abort();
        }


	if(pstrInFile == NULL || pstrOutFile == NULL){
		putUsage();
		return 1;
	}
	
	return convert(pstrInFile, pstrOutFile, nFps, compress, ditherMode) ? 0 : 1;
}
