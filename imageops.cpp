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

/*******************************************
*	Dithering
********************************************/


uint8_t* DitherFS(uint8_t* pSrc, int width, int height, int bitDepth)
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



typedef	int	pixel;



/////////////////////////////////////////////////////////////////////////////
//	Ordered dither matrices
/////////////////////////////////////////////////////////////////////////////

const	int BAYER_PATTERN_2X2[2][2]		=	{	//	2x2 Bayer Dithering Matrix. Color levels: 5
												{	 51, 206	},
												{	153, 102	}
											};

const	int BAYER_PATTERN_3X3[3][3]		=	{	//	3x3 Bayer Dithering Matrix. Color levels: 10
												{	 181, 231, 131	},
												{	  50,  25, 100	},
												{	 156,  75, 206	}
											};

const	int BAYER_PATTERN_4X4[4][4]		=	{	//	4x4 Bayer Dithering Matrix. Color levels: 17
												{	 15, 195,  60, 240	},
												{	135,  75, 180, 120	},
												{	 45, 225,  30, 210	},
												{	165, 105, 150,  90	}

											};

const	int BAYER_PATTERN_8X8[8][8]		=	{	//	8x8 Bayer Dithering Matrix. Color levels: 65
												{	  0, 128,  32, 160,   8, 136,  40, 168	},
												{	192,  64, 224,  96, 200,  72, 232, 104	},
												{	 48, 176,  16, 144,  56, 184,  24, 152	},
												{	240, 112, 208,  80, 248, 120, 216,  88	},
												{	 12, 140,  44, 172,   4, 132,  36, 164	},
												{	204,  76, 236, 108, 196,  68, 228, 100	},
												{	 60, 188,  28, 156,  52, 180,  20, 148	},
												{	252, 124, 220,  92, 244, 116, 212,  84	}
											};

const	int	BAYER_PATTERN_16X16[16][16]	=	{	//	16x16 Bayer Dithering Matrix.  Color levels: 256
												{	  0, 191,  48, 239,  12, 203,  60, 251,   3, 194,  51, 242,  15, 206,  63, 254	}, 
												{	127,  64, 175, 112, 139,  76, 187, 124, 130,  67, 178, 115, 142,  79, 190, 127	},
												{	 32, 223,  16, 207,  44, 235,  28, 219,  35, 226,  19, 210,  47, 238,  31, 222	},
												{	159,  96, 143,  80, 171, 108, 155,  92, 162,  99, 146,  83, 174, 111, 158,  95	},
												{	  8, 199,  56, 247,   4, 195,  52, 243,  11, 202,  59, 250,   7, 198,  55, 246	},
												{	135,  72, 183, 120, 131,  68, 179, 116, 138,  75, 186, 123, 134,  71, 182, 119	},
												{	 40, 231,  24, 215,  36, 227,  20, 211,  43, 234,  27, 218,  39, 230,  23, 214	},
												{	167, 104, 151,  88, 163, 100, 147,  84, 170, 107, 154,  91, 166, 103, 150,  87	},
												{	  2, 193,  50, 241,  14, 205,  62, 253,   1, 192,  49, 240,  13, 204,  61, 252	},
												{	129,  66, 177, 114, 141,  78, 189, 126, 128,  65, 176, 113, 140,  77, 188, 125	},
												{	 34, 225,  18, 209,  46, 237,  30, 221,  33, 224,  17, 208,  45, 236,  29, 220	},
												{	161,  98, 145,  82, 173, 110, 157,  94, 160,  97, 144,  81, 172, 109, 156,  93	},
												{	 10, 201,  58, 249,   6, 197,  54, 245,   9, 200,  57, 248,   5, 196,  53, 244	},
												{	137,  74, 185, 122, 133,  70, 181, 118, 136,  73, 184, 121, 132,  69, 180, 117	},
												{	 42, 233,  26, 217,  38, 229,  22, 213,  41, 232,  25, 216,  37, 228,  21, 212	},
												{	169, 106, 153,  90, 165, 102, 149,  86, 168, 105, 152,  89, 164, 101, 148,  85	}
											};

/////////////////////////////////////////////////////////////////////////////


void	DitherBayer16( uint8_t* pixels, int width, int height )	
{
	int	col	= 0;
	int	row	= 0;

	for( int y = 0; y < height; y++ )
	{
		row	= y & 15;	//	y % 16
        
		for( int x = 0; x < width; x++ )
		{
			col	= x & 15;	//	x % 16

			const pixel	blue	= pixels[x * 3 + 0];
            const pixel	green	= pixels[x * 3 + 1];
            const pixel	red		= pixels[x * 3 + 2];

			pixel	color	= ((red + green + blue)/3 < BAYER_PATTERN_16X16[col][row] ? 0 : 255);
			
            pixels[x * 3 + 0]	= color;	//	blue
            pixels[x * 3 + 1]	= color;	//	green
            pixels[x * 3 + 2]	= color;	//	red
		}

		pixels	+= width * 3;
	}
}

void	DitherBayer8( uint8_t* pixels, int width, int height )	
{
	int	col	= 0;
	int	row	= 0;

	for( int y = 0; y < height; y++ )
	{
		row	= y & 7;		//	% 8;
        
		for( int x = 0; x < width; x++ )
		{
			col	= x & 7;	//	% 8;

			const pixel	blue	= pixels[x * 3 + 0];
            const pixel	green	= pixels[x * 3 + 1];
            const pixel	red		= pixels[x * 3 + 2];

			pixel	color	= ((red + green + blue)/3 < BAYER_PATTERN_8X8[col][row] ? 0 : 255);
			
            pixels[x * 3 + 0]	= color;	//	blue
            pixels[x * 3 + 1]	= color;	//	green
            pixels[x * 3 + 2]	= color;	//	red
		}

		pixels	+= width * 3;
	}
}

void	DitherBayer4( uint8_t* pixels, int width, int height )	
{
	int	col	= 0;
	int	row	= 0;

	for( int y = 0; y < height; y++ )
	{
		row	= y & 3;	//	% 4
        
		for( int x = 0; x < width; x++ )
		{
			col	= x & 3;	//	% 4

			const pixel	blue	= pixels[x * 3 + 0];
            const pixel	green	= pixels[x * 3 + 1];
            const pixel	red		= pixels[x * 3 + 2];

			pixel	color	= ((red + green + blue)/3 < BAYER_PATTERN_4X4[col][row] ? 0 : 255);
			
            pixels[x * 3 + 0]	= color;	//	blue
            pixels[x * 3 + 1]	= color;	//	green
            pixels[x * 3 + 2]	= color;	//	red
		}

		pixels	+= width * 3;
	}
}

void	DitherBayer3( uint8_t* pixels, int width, int height )	
{
	int	col	= 0;
	int	row	= 0;

	for( int y = 0; y < height; y++ )
	{
		row	= y % 3;
        
		for( int x = 0; x < width; x++ )
		{
			col	= x % 3;

			const pixel	blue	= pixels[x * 3 + 0];
            const pixel	green	= pixels[x * 3 + 1];
            const pixel	red		= pixels[x * 3 + 2];

			pixel	color	= ((red + green + blue)/3 < BAYER_PATTERN_3X3[col][row] ? 0 : 255);
			
            pixels[x * 3 + 0]	= color;	//	blue
            pixels[x * 3 + 1]	= color;	//	green
            pixels[x * 3 + 2]	= color;	//	red
		}

		pixels	+= width * 3;
	}
}

void	DitherBayer2( uint8_t* pixels, int width, int height )	
{
	int	col	= 0;
	int	row	= 0;

	for( int y = 0; y < height; y++ )
	{
		row	= y & 1;	//	y % 2
        
		for( int x = 0; x < width; x++ )
		{
			col	= x & 1;	//	x % 2

			const pixel	blue	= pixels[x * 3 + 0];
            const pixel	green	= pixels[x * 3 + 1];
            const pixel	red		= pixels[x * 3 + 2];

			pixel	color	= ((red + green + blue)/3 < BAYER_PATTERN_2X2[col][row] ? 0 : 255);
			
            pixels[x * 3 + 0]	= color;	//	blue
            pixels[x * 3 + 1]	= color;	//	green
            pixels[x * 3 + 2]	= color;	//	red
		}

		pixels	+= width * 3;
	}
}

//	Black-white Sierra Lite dithering (variation of Floyd-Steinberg with less computational cost)
#define	SIERRA_COEF( v, err )	((( (err) * ((v) << 8)) >> 5) >> 8)
void	DitherSierra( uint8_t* pixels, int width, int height )	
{
	//	To avoid real number calculations, I will raise the level of INT arythmetics by shifting with 8 bits to the left ( << 8 )
	//	Later, when it is necessary will return to te normal level by shifting back 8 bits to the right ( >> 8 )
	//	       X   5   3
    //	2   4  5   4   2
    //	    2  3   2
    //	    (1/32)

	//~~~~~~~~

	const int	size	= width * height;

	int*	error	= (int*)malloc( size * sizeof(int) );

	//	Clear the errors buffer.
	memset( error, 0, size * sizeof(int) );

	//~~~~~~~~

	int	i	= 0;

	for( int y = 0; y < height; y++ )
	{
		for( int x = 0; x < width; x++,i++ )
		{
			const pixel	blue	= pixels[x * 3 + 0];
            const pixel	green	= pixels[x * 3 + 1];
            const pixel	red		= pixels[x * 3 + 2];

			//	Get the pixel gray value.
			int	newVal	= (red + green + blue) / 3 + error[i];		//	PixelGray + error correction
			int	newc	= (newVal < 128 ? 0 : 255);

			pixels[x * 3 + 0]	= newc;
			pixels[x * 3 + 1]	= newc;
			pixels[x * 3 + 2]	= newc;

			//	Correction - the new error
			const int	cerror	= newVal - newc;

			int idx = i;
			if( x + 1 < width )
				error[idx+1] += SIERRA_COEF( 5, cerror );

			if( x + 2 < width )
				error[idx+2] += SIERRA_COEF( 3, cerror );

			if( y + 1 < height )
			{
				idx += width;
				if( x-2 >= 0 )
					error[idx-2] += SIERRA_COEF( 2, cerror );
				
				if( x-1 >= 0 )
					error[idx-1] += SIERRA_COEF( 4, cerror );

				error[idx] += SIERRA_COEF( 5, cerror );

				if( x+1 < width )
					error[idx+1] += SIERRA_COEF( 4, cerror );

				if( x+2 < width )
					error[idx+2] += SIERRA_COEF( 2, cerror );
			}

			if( y + 2 < height )
			{
				idx	+= width;
				if( x-1 >= 0 )
					error[idx-1] += SIERRA_COEF( 2, cerror );

				error[idx] += SIERRA_COEF( 3, cerror );

				if( x+1 < width )
					error[idx+1] += SIERRA_COEF( 2, cerror );
			}
		}
		
		pixels	+= width*3;
	}

	free( error );
}




/*******************************************
*	Image manipulation
********************************************/


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



/*******************************************
*	Write various formats to disk
********************************************/

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