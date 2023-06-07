#ifndef IMAGEOPS_H
#define IMAGEOPS_H

#include <cstdint>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bmp.h"

#define CLAMP(z) ( (z > 255) ? 255 : ( (z < 0) ? 0 : z) )
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

#define BYTE_TO_INVERTED_BINARY(byte)  \
  ((byte) & 0x80 ? '0' : '1'), \
  ((byte) & 0x40 ? '0' : '1'), \
  ((byte) & 0x20 ? '0' : '1'), \
  ((byte) & 0x10 ? '0' : '1'), \
  ((byte) & 0x08 ? '0' : '1'), \
  ((byte) & 0x04 ? '0' : '1'), \
  ((byte) & 0x02 ? '0' : '1'), \
  ((byte) & 0x01 ? '0' : '1') 


enum DitherMode { FloydSteinberg, Sierra, Bayer2, Bayer3, Bayer4, Bayer8, Bayer16, None };

bool Mono(uint8_t* pDst, int nWidth, int nHeight);

uint8_t* ConvertToGrayscale(unsigned char* pDst, int nWidth, int nHeight);
uint8_t* ConvertTo1Bit(unsigned char* pSrc, int nWidth, int nHeight);
uint8_t* ConvertTo8Bit(unsigned char* pSrc, int nWidth, int nHeight);
uint8_t* FlipVertical(uint8_t* pSrc, int width, int height);

uint8_t* DitherFS(uint8_t* pSrc, int width, int height, int bitDepth);
void	DitherBayer16( uint8_t* pixels, int width, int height )	;
void	DitherBayer8 ( uint8_t* pixels, int width, int height )	;
void	DitherBayer4 ( uint8_t* pixels, int width, int height )	;
void	DitherBayer3 ( uint8_t* pixels, int width, int height )	;
void	DitherBayer2 ( uint8_t* pixels, int width, int height )	;
void	DitherSierra ( uint8_t* pixels, int width, int height )	;


int SaveBMP(uint8_t *pData, int width, int height, int bitDepth);
int SavePak(uint8_t* pData, int width, int height);
int SavePBM(uint8_t* pData, int width, int height, char* filename = "output.pbm");
int SavePPM(uint8_t* frame, int width, int height);
int SaveEVA(uint8_t* frame);
#endif