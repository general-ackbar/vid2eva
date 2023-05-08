#include "imageops.h"


bool Mono(uint8_t* pDst, int nWidth, int nHeight)
{
	int		u,v,y;

	for(v = 0; v < nHeight; v++){
		for(u = 0; u < nWidth; u++){
			y = ((int)*(pDst+0) * 153 + (int)*(pDst+1) * 27 + (int)*(pDst+2) * 76) / 256;
			*pDst++ = (uint8_t)(y & 255);
			*pDst++ = (uint8_t)(y & 255);
			*pDst++ = (uint8_t)(y & 255);
		}
	}
	return true;
}



uint8_t* ConvertToGrayscale(unsigned char* pSrc, int nWidth, int nHeight)
{
	int		u,v,y;
	uint8_t* pDst = NULL;
	pDst = (uint8_t*)malloc(nWidth*nHeight*3);
	//uint8_t* pDst (uint8_t*)malloc(nWidth*nHeight*3);
	//uint8_t pDst[nWidth*nHeight*3];
	int c = 0;
	for(v = 0; v < nHeight; v++){
		for(u = 0; u < nWidth; u++){
			y = ((int)*(pSrc+0) * 153 + (int)*(pSrc+1) * 27 + (int)*(pSrc+2) * 76) / 256;
			pSrc +=3;
			pDst[c++] = (unsigned char)(y & 255);
			pDst[c++] = (unsigned char)(y & 255);
			pDst[c++] = (unsigned char)(y & 255);

			//*pDst++ = (unsigned char)(y & 255);
			//*pDst++ = (unsigned char)(y & 255);
			//*pDst++ = (unsigned char)(y & 255);
		}
	}	
	//SavePPM(pDst, 192, 184);	
	return pDst;
}

uint8_t* ConvertTo1Bit(unsigned char* pSrc, int nWidth, int nHeight)
{
	int c = 0, x = 0;
	uint8_t* pDst = (uint8_t*)malloc(nWidth*nHeight / 8);
	//uint8_t pDst[nWidth*nHeight / 8];
	int bitCounter = 0;	
	uint8_t currentByte = 0x00;
	

	for(int pos = 0; pos < nWidth*nHeight*3; pos+=3)
	{
		if(pSrc[pos] > 127 )
		{	
			currentByte |= (1 << (7 -bitCounter));
		}

		bitCounter++;
		if(bitCounter > 7)
		{						
			pDst[c] = (currentByte);			
			currentByte = 0x00;
			bitCounter = 0;
			c++;
		}
	}
	return pDst;
}

uint8_t* ConvertTo8Bit(unsigned char* pSrc, int nWidth, int nHeight)
{
	int dstCounter = 0;
	uint8_t* pDst = (uint8_t*)malloc(nWidth*nHeight);
	
	for(int pos = 0; pos < nWidth*nHeight*3; pos+=3)
	{
		pDst[dstCounter] = pSrc[pos];
		dstCounter++;
	}
	return pDst;
}

uint8_t* Dither(uint8_t* pSrc, int width, int height, int bitDepth)
{        
	int rowLength = 0;
	int currentPos = 0;
	int widthInBytes = width*(bitDepth / 8);
	int nextByteOffset = bitDepth / 8;
    
	// use Error-Diffusion
    for( int row=0; row < height; row++ ){
        rowLength = row * widthInBytes;
        
        
        for( int col = 0; col < widthInBytes; col+=3 ){
            currentPos = rowLength + col;
			
            
            uint8_t origValue = pSrc[currentPos];
            uint8_t newValue = (origValue > 127 ) ? 255 : 0;
            int error = -(newValue - origValue);
            
            pSrc[currentPos] = newValue;
			pSrc[currentPos+1] = newValue;	
			pSrc[currentPos+2] = newValue;
			
            pSrc[currentPos+nextByteOffset] = CLAMP(pSrc[currentPos+nextByteOffset] + (7*error/16));
            
            int nextRowPos = rowLength + col + 3*width;
            pSrc[nextRowPos -nextByteOffset] = CLAMP( pSrc[nextRowPos -nextByteOffset] + (3*error/16) );
            pSrc[nextRowPos] = CLAMP( pSrc[nextRowPos] + (5*error/16) );
            pSrc[nextRowPos +nextByteOffset] = CLAMP( pSrc[nextRowPos +nextByteOffset] + (error/16) );
			
        }
    }

	return pSrc;
}

uint8_t* FlipVertical(uint8_t* pSrc, int width, int height)
{
	uint8_t* pDst = NULL;

	int widthStep = (((width * 24) + 31) & (~31)) / 8; // line width of buffer % 4 == 0
	int dstSize = widthStep * height;
	pDst = (uint8_t*)malloc(dstSize);
	if (!pDst){
		printf("Error init bmp buffer");
		return NULL;
	}
	memset(pDst, 0, dstSize);

	// seems ppm data is bottom to top
	uint8_t *pp = pSrc;
	uint8_t *pb = pDst + dstSize;
	int hh = height;
	while (hh > 0){
		pb -= widthStep;
		memcpy(pb, pp, width * 3);
		pp += width * 3;

		hh--;
	}	
	return pDst;
}


int SavePBM(uint8_t* pData, int width, int height, char* filename)
{
        FILE* fileHandle;
        
    
        fileHandle = fopen(filename, "wb");

        if (fileHandle == NULL)
        {
            printf("Unable to open %s...\n", filename);
            return -1;
        }
        fprintf(fileHandle, "P1\n%d %d\n", width, height);
		for(int i = 0; i < (width*height/8); i++)
		{
			fprintf(fileHandle, BYTE_TO_BINARY_PATTERN, BYTE_TO_INVERTED_BINARY(pData[i]));

		}
        
        fclose(fileHandle);
        
    return 1;
}

int SavePak(uint8_t* pData, int width, int height)
{
	FILE* fileHandle;	
    char filename[32];
    static long frameNumber = 0;

    sprintf(filename, "frame_pak_%04ld.pak", frameNumber);
	fileHandle = fopen(filename, "wb");
    if (fileHandle == NULL)
    {
        printf("Unable to open %s...\n", filename);
        return -1;
    }

	uint8_t header[10] = {'p','a','k', 0x0, 0x0A, (width>>8) & 0xFF, width & 0xFF, (height >> 8) & 0xFF, height & 0xFF, 1 & 0xFF};
	
    fwrite(header,1,10,fileHandle);
                    
    fwrite(pData, 1, width*height/8, fileHandle);
    fclose(fileHandle);
    frameNumber++;

    return 1;
}

int SavePPM(uint8_t* frame, int width, int height)
{
        FILE* fileHandle;
        char filename[32];
        static long frameNumber = 0;
        sprintf(filename, "frame_ppm_%05ld.ppm", frameNumber);
    
        fileHandle = fopen(filename, "wb");

        if (fileHandle == NULL)
        {
            printf("Unable to open %s...\n", filename);
            return -1;
        }
        fprintf(fileHandle, "P6\n%d %d\n%d\n", width, height, 255);
        fwrite(frame, 1, width*height*3, fileHandle);
        fclose(fileHandle);
        frameNumber++;

    return 1;
}

int SaveBMP(uint8_t *pData, int width, int height, int bitDepth)
{
	int bufBmpSize = width*height*(bitDepth / 8);

	BITMAPINFO bitmapinfo;
	memset(&bitmapinfo, 0, sizeof(BITMAPINFO));
	bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfo.bmiHeader.biWidth = width;
	bitmapinfo.bmiHeader.biHeight = height;
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = bitDepth;
	bitmapinfo.bmiHeader.biXPelsPerMeter = 0;
	bitmapinfo.bmiHeader.biYPelsPerMeter = 0;
	bitmapinfo.bmiHeader.biSizeImage = bufBmpSize;
	bitmapinfo.bmiHeader.biClrUsed = 0;
	bitmapinfo.bmiHeader.biClrImportant = 0;

	// bmp file header
	BITMAPFILEHEADER bmpHeader;
	memset(&bmpHeader, 0, sizeof(BITMAPFILEHEADER));
	bmpHeader.bfType = 0x4D42;
	bmpHeader.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
	bmpHeader.bfSize = bmpHeader.bfOffBits + bufBmpSize;

	// open bmp file
	FILE *fpBmp = NULL;
   	char bmpFilename[32];
    static long frameNumber = 0;

    sprintf(bmpFilename, "frame_bmp_%04ld.bmp", frameNumber);
	fpBmp = fopen(bmpFilename, "wb");
	if (!fpBmp){
		printf("bmp file %s open failed.\n", bmpFilename);		
	}

	// write bmp file
	fwrite(&bmpHeader, 1, sizeof(BITMAPFILEHEADER), fpBmp);
	fwrite(&(bitmapinfo.bmiHeader), 1, sizeof(BITMAPINFOHEADER), fpBmp);
	fwrite(pData, 1, bufBmpSize, fpBmp);

	

	frameNumber++;

	return 0;
}

int SaveEVA(uint8_t* frame)
{
	FILE* fileHandle;	
	const int FRAMESIZE = (96/4)*92;
    char filename[32];
    static long frameNumber = 0;

    sprintf(filename, "frame_eva_%04ld.eva", frameNumber);
	fileHandle = fopen(filename, "wb");
    if (fileHandle == NULL)
    {
        printf("Unable to open %s...\n", filename);
        return -1;
    }

	char header[9];
	strcpy((char*)header,"EVA3");
    header[4]= 0x1a;
    header[5]= 1 & 0xff;
    header[6]= (1 & 0xff00) >> 8;
    header[7]= 10 & 0xff;
    header[8]= (10 & 0xff00) >> 8;
    fwrite(header,1,9,fileHandle);
                    
    fwrite(frame, 1, FRAMESIZE, fileHandle);
    fclose(fileHandle);
    frameNumber++;

    return 1;
}