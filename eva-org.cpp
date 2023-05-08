#include "eva-org.h"

unsigned char* lpBuff;
int    hFileEva;

bool convPCM(unsigned char* lpDst, void* lpSound, int nSize)
{
	memcpy(lpDst, lpSound, nSize);
	return true;
}

bool CloseEVA()
{
    if(hFileEva != INVALID_HANDLE) close(hFileEva);
	hFileEva = INVALID_HANDLE;
	return true;
}

bool CreateEVA(const char * lpszFilename, int frequency, int nFPS, int nFrames)
{

	uint16_t WFREQ = frequency;
	uint16_t pcmlen = WFREQ / 2 / nFPS;

	CloseEVA();

	hFileEva = open(lpszFilename, O_CREAT|O_RDWR, 0666);
	
	EvaHeader eva;
	eva.Identifier = 0x45 | (0x56 << 8) | (0x41 << 16) | (0x35 << 24);
	eva.comment = 0x1A;
	eva.NoOfFrames = nFrames; // (0x01 << 8) | 0xD3;
	eva.WidthInDots = 96; //(0x00 << 8) | 0x60; //96;
	eva.HeightInDots = 92; //(0x00 << 8) | 0x5C; //92;
	eva.BitsPerPixel = 0x02;
	eva.AspectRatio = 0x18;
	eva.PCMFormat = 0x01;
	eva.SamplingFrequency = WFREQ; 
	eva.PCMDataLength = pcmlen; // l_2 << 8 | l_1;
	eva.Dummy = 0x00;

	write(hFileEva, &eva.Identifier, sizeof(eva.Identifier));
	write(hFileEva, &eva.comment, sizeof(eva.comment));
	write(hFileEva, &eva.NoOfFrames, sizeof(eva.NoOfFrames));
	write(hFileEva, &eva.WidthInDots, sizeof(eva.WidthInDots));
	write(hFileEva, &eva.HeightInDots, sizeof(eva.HeightInDots));
	write(hFileEva, &eva.BitsPerPixel, sizeof(eva.BitsPerPixel));
	write(hFileEva, &eva.AspectRatio, sizeof(eva.AspectRatio));
	write(hFileEva, &eva.PCMFormat, sizeof(eva.PCMFormat));
	write(hFileEva, &eva.SamplingFrequency, sizeof(eva.SamplingFrequency));
	write(hFileEva, &eva.PCMDataLength, sizeof(eva.PCMDataLength));
	write(hFileEva, &eva.Dummy, sizeof(eva.Dummy));

	

	return hFileEva != INVALID_HANDLE;
}

bool CreateEVA3(const char * lpszFilename, int nFPS, int nFrames)
{
	CloseEVA();
	hFileEva = open(lpszFilename, O_CREAT|O_RDWR, 0666);
	
	char header[9];
	strcpy((char*)header,"EVA3");
    header[4]= 0x1a;
    header[5]= nFrames & 0xff;
    header[6]= (nFrames & 0xff00) >> 8;
    header[7]= nFPS & 0xff;
    header[8]= (nFPS & 0xff00) >> 8;
    write(hFileEva, header, 9);
                        
	return hFileEva != INVALID_HANDLE;
}

bool CreateEVA4(const char * lpszFilename, int nFPS, int nFrames)
{
	CloseEVA();
	hFileEva = open(lpszFilename, O_CREAT|O_RDWR, 0666);
	
	char header[9];
	strcpy((char*)header,"EVA4");
    header[4]= 0x1a;
    header[5]= nFrames & 0xff;
    header[6]= (nFrames & 0xff00) >> 8;
    header[7]= nFPS & 0xff;
    header[8]= (nFPS & 0xff00) >> 8;
    write(hFileEva, header, 9);
                        
	return hFileEva != INVALID_HANDLE;
}

bool AppendFrame(uint8_t* pFrame, int frame_length)
{
	unsigned long	dwWritten;
	if(hFileEva == INVALID_HANDLE) return false;
	dwWritten = write(hFileEva, pFrame, frame_length);    
	return dwWritten == frame_length;
}

bool AppendEVA(int frame_length)
{
	unsigned long	dwWritten;
	if(hFileEva == INVALID_HANDLE) return false;
	dwWritten = write(hFileEva, lpBuff, frame_length);
    
	return dwWritten == frame_length;
}


int ConvertToEva5(void* pSurface, int width, int height, void* lpSound, int nPcmSize)
{
//	unsigned char*	lpBuff;
	int frameSize = 2;
	frameSize += nPcmSize;

	int fsize = 24*92;


	lpBuff = (uint8_t*) malloc(frameSize + fsize); //Max frame size

	/* Zero */
	memset(lpBuff, 0, frameSize + fsize);


	/*Audio first*/
	if(lpSound){
		if(VERBOSE)printf("Sound added (size: %i)\n", nPcmSize);
		if(!convPCM(lpBuff, lpSound, nPcmSize)) return false;
	} else {
		if(VERBOSE)printf("No sound added (size: %i)\n", nPcmSize);		
		memset(lpBuff, 0x00, nPcmSize);
	}

	//Figure out frame size	- most likely 24*92 uncompressed
	//fsize = arc4random_uniform(fsize);

	uint8_t partA = (fsize) & 255;
	uint8_t partB  = ((fsize) >> 8) & 255;	
	memcpy(lpBuff+nPcmSize, &partA, 1);
	memcpy(lpBuff+nPcmSize+1, &partB, 1);


	/* picture */
	if(pSurface){
		fsize = ConvertToEva3(lpBuff + nPcmSize +2, pSurface, width, height);
		if(VERBOSE)printf("Picture added (%i bytes)\n", fsize);
		frameSize += fsize;

		//uint8_t* eva3 = (uint8_t*)convertToEva3(lpSurface);
		//memcpy(lpBuff + nPcmSize + 2, lp, FSIZE);
		//All wrong. None of these method convert to the desired format
		//uint8_t* eva3 = ConvertToEva3(lpSurface);
		//frameSize += ConvertToEva4(eva3);
	} else {
		if(VERBOSE)printf("No picture added (noise %i bytes)\n", fsize);
		memset(lpBuff + nPcmSize + 2, 0xFF, fsize);
		frameSize += fsize;
	}

	
	return frameSize;
}


int ConvertToEva3(uint8_t* pDst, void* pSrc, int width, int height) 
{
	uint8_t *pBmp = (uint8_t*)pSrc;
	pBmp = ConvertToGrayscale(pBmp, width, height);	
	pBmp = Dither(pBmp, width, height, 24);	
	if(!OUTPUT_PAK && !OUTPUT_PPM && !OUTPUT_PBM) pBmp = FlipVertical(pBmp, width, height);	
	if(!OUTPUT_PPM) pBmp = ConvertTo1Bit(pBmp, width, height);	



	if(OUTPUT_PPM)SavePPM(pBmp, width,height);
	if(OUTPUT_PBM) SavePBM(pBmp, width,height);
	if(OUTPUT_PAK)SavePak(pBmp, width, height);
	if(OUTPUT_BMP)SaveBMP(pBmp, width, height, 24);
	
	unsigned short off=0;
	unsigned char ytable[5]={0,0,1,2,3};
	
	int c = 0;
	for(int j = 0; j < YSIZE; j++ )
	{
		unsigned short i, x, y, ptr;
		unsigned char indat, outdat, w1;
		unsigned char image[XSIZE*2][2];
		unsigned char  wbuff[XSIZE/4];

		for (y=0;y<2;y++)
		{
			for (x=0;x<XSIZE*2;x+=8)
			{
				indat = pBmp[c];
				c++;
				for (i=0;i<8;i++)
				{
					image[7-i+x][y]=((indat & (1<<i)) != 0) ? 1:0;
				}
			}
		}
		ptr=0;
		for (x=0;x<XSIZE;x+=4)
		{
			outdat=0;
			for (i=0;i<4;i++)
			{
				w1 = image[x*2+i*2][0]
					+image[x*2+1+i*2][0]
					+image[x*2+i*2][1]
					+image[x*2+1+i*2][1];
				outdat |= ytable[w1] << ((3-i)*2);
			}
			wbuff[ptr++]=outdat;
		}
		
		memcpy(&pDst[DSIZE-XSIZE/4-off],wbuff,XSIZE/4);
		off+=XSIZE/4;
	}

	//if(OUTPUT_EVA) SaveFrame(pDst);
	//pDst = obuff;
	return DSIZE;
}

int ConvertToEva4(uint8_t* pSrc, int width, int height, int mode, int level)
{

	uint8_t kbuf[FSIZE];
	uint8_t fbuf[8][FSIZE];
	uint8_t obuf[FSIZE * 2];
	

	int	i;
	int	t, tmax;
	int	pw;
	int	rc;
	uint8_t	*p0, *p, *p2;

//Reads 8 uncompressed frames into a buffer
	/*
	for(i = 0; i < 7; i++)
	{
		if(i < tmax)
			fread(fbuf[i], 1, FSIZE, fp1);
		else
			memcpy(fbuf[i], fbuf[i-1], FSIZE);
	}
	
	rc = 7;
	*/
	p0 = fbuf[0];
	
	for(t = 0; t < tmax; t++)
	{	uint8_t	*q;
		int	l;

		printf("%d\r", t + 1);
		
		p = fbuf[t & 7];
		p2 = fbuf[(t + 1) & 7];

		
		if(mode == 1)
		{
			for(i = 0; i < FSIZE; i++)
			{
				if(p0[i] == p2[i] && p0[i] != p[i])
				{
					int	c, c0, hc, k;
					
					c = p[i];
					c0 = p0[i];
					
					hc = 0;
					for(k = 0; k < 4; k ++)
					{	int	d;
						
						d = (c & 3) - (c0 & 3);
						if(d != 0) hc++;
						c >>= 2;
						c0 >>= 2;
					}
					
					if(hc <= level) p[i] = p0[i];
				}
			}
		}
		
		if(mode == 2)
		{
			for(i = 0; i < FSIZE; i++)
			{
				if(p[i] != p2[i])
				{	int	c, j, j1;
					static int b[4], d[4];
					
					c = p[i];
					b[0] = ((c     ) & 3);
					b[1] = ((c >> 2) & 3);
					b[2] = ((c >> 4) & 3);
					b[3] = ((c >> 6) & 3);
#ifdef DEBUG
	printf("\n\n");
	printf("%2d %2d %2d %2d\r",b[3],b[2],b[1],b[0]);
#endif
					for(j = 1; j < 7; j++)
					{	int	k, hc;
						
						c = fbuf[(t+j)&7][i];
						hc = 0;
						
						for(k = 0; k < 4; k++)
						{
							d[k] = (c & 3);
							if(d[k] != ((b[k]+(j/2))/j))
								hc++;
							c >>= 2;
						}

#ifdef DEBUG
	printf("\n%2d %2d %2d %2d\r",d[3],d[2],d[1],d[0]);
#endif

						if(hc <= level)
						{	b[0] += d[0];
							b[1] += d[1];
							b[2] += d[2];
							b[3] += d[3];
						} else
						{
							break;
						}
					}
					j1 = j;

#ifdef DEBUG
	printf("%2d %2d %2d %2d %2d\n",b[3],b[2],b[1],b[0],j);
#endif

					if(j > 1)
					{	c = (((b[0]+(j/2)) / j)     ) +
						    (((b[1]+(j/2)) / j) << 2) +
						    (((b[2]+(j/2)) / j) << 4) +
						    (((b[3]+(j/2)) / j) << 6);

#ifdef DEBUG
	printf("%2X\n",c);
#endif

						for(j = 0; j < j1; j++)
							fbuf[(t+j)&7][i] = c;
					}
				}
			}
		}
		
		for(i = 0; i < FSIZE; i++)		/* �ω��_���o */
		{	int	c;
			
			kbuf[i] = 0;
			c = p[i];
			if(t > 0 && c == p0[i])
				kbuf[i] = 1;
			else if(i > 0 && c == p[i-1])
				kbuf[i] = 2;
		}

		for(i = 0; i < FSIZE - 2; i++)		/* ���ʂȔ�ω��_������ */
		{	if(kbuf[i] == 0 && kbuf[i+2] == 0)
			 	kbuf[i+1] = 0;
		}

		q = obuf + 2;

		i = 0;
		while(i < FSIZE)
		{	int	s1 = 0, s2 = 0, s3 = 0;
			
			*q = 0;
			if(kbuf[i] == 1 && i < FSIZE)	/* �ω��Ȃ� */
			{	i++;
				s1 = 1;
				while(s1 < 63 && i < FSIZE && kbuf[i] == 1)
				{	i++;
					s1++;
				}
				if(s1 < 8)
					*q = (s1 << 3);
				else
				{	*q++ = 0x40 + s1;
					*q = 0;
				}
			}

			if(kbuf[i] == 2 && i < FSIZE)	/* �A���f�[�^ */
			{
				s3 = 0;
				while(s3 < 7 && i < FSIZE && kbuf[i] == 2)
				{	i++;
					s3++;
				}

				*q++ += 0xC0 + s3;
				*q = 0;
			}

			if(kbuf[i] == 0 && i < FSIZE)	/* �ω����� */
			{	int	i0, j;

				i0 = i;
				i++;
				s2 = 1;
				while(s2 < 63 && i < FSIZE && kbuf[i] == 0)
				{	i++;
					s2++;
				}
				if(s2 < 8)
					*q++ += s2;
				else
				{	if(*q != 0) q++;
					*q++ = 0x80 + s2;
				}
				i = i0;
				for(j = 0; j < s2; j++)
					*q++ = p[i++];
				*q = 0;
			}
			if(*q != 0) q++;
		}
		
		l = q - &obuf[2];
		obuf[0] = (l & 255);
		obuf[1] = (l >> 8);
		
		l += 2;
		
		//obuff = frame data 
		//l = frame length / data to write

		/*
		if(fwrite(obuf, 1, l, fp2) != l)
			printf("file write error");
		
		p0 = p;
		
		if(rc < tmax)
			fread(fbuf[rc & 7], 1, FSIZE, fp1);
		else
			memcpy(fbuf[rc &  7], fbuf[(rc-1) & 7], FSIZE);
		rc++;
		*/
	}
}

int SaveFrame(uint8_t* frame)
{
	FILE* fileHandle;	
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
                    
    fwrite(frame, 1, DSIZE, fileHandle);
    fclose(fileHandle);
    frameNumber++;

    return 1;
}
