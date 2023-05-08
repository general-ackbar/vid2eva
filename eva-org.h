
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdint>
#include <stdlib.h>


#include "imageops.h"


#define XSIZE 96
#define YSIZE 92
#define DSIZE 24*YSIZE
#define FSIZE XSIZE*YSIZE
#define INVALID_HANDLE -1
#define VERBOSE 0
#define OUTPUT_EVA 0
#define OUTPUT_BMP 0
#define OUTPUT_PAK 0
#define OUTPUT_PPM 0
#define OUTPUT_PBM 0


typedef struct {
    uint32_t Identifier;
    uint8_t comment;
    uint16_t NoOfFrames;
    uint16_t WidthInDots;
    uint16_t HeightInDots;
    uint8_t BitsPerPixel;
    uint8_t AspectRatio;
    uint8_t PCMFormat;
    uint16_t SamplingFrequency;
    uint16_t PCMDataLength;
    uint8_t Dummy;
} EvaHeader;

bool CloseEVA();
bool CreateEVA(const char * lpszFilename, int frequency, int nFPS, int nFrames);
bool CreateEVA3(const char * lpszFilename, int nFPS, int nFrames);
bool AppendEVA(int frame_length);
bool AppendFrame(uint8_t* pFrame, int frame_length);
int ConvertToEva5(void* pSurface, int width, int height, void* lpSound, int nPcmSize);
int ConvertToEva3(unsigned char* pDst, void* pSrc, int width, int height);
int ConvertToEva4(uint8_t* pSrc, int width, int height, int mode, int level);

int SaveFrame(uint8_t* frame);
