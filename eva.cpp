#include "eva.h"

Eva::Eva(const char *infile)
{
}

Eva::Eva(int fps)
{
    framerate = fps;
}

Eva::Eva()
{
}

Eva::~Eva()
{
}

uint8_t *Eva::getFrameAt(int index)
{
    return frames[index];
}

uint8_t * Eva::encodeFrame(uint8_t *pSrc, int width, int height, bool dither)
{
	if(dither)
		return encodeFrameDithered(pSrc, width, height);
	else
		return encodeFrameUndithered(pSrc, width, height);
}

uint8_t * Eva::encodeFrameDithered(uint8_t *pSrc, int width, int height)
{

    uint8_t *pBmp = pSrc;
    uint8_t *pDst = NULL;
	pDst = (uint8_t*)malloc(FRAMESIZE);
	pBmp = ConvertToGrayscale(pBmp, width, height);	
	pBmp = Dither(pBmp, width, height, 24);   
	pBmp = FlipVertical(pBmp, width, height);	
	pBmp = ConvertTo1Bit(pBmp, width, height);
	
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
		
		memcpy(&pDst[FRAMESIZE-XSIZE/4-off],wbuff,XSIZE/4);
		off+=XSIZE/4;
	}

	frames.push_back(pDst);
    frames.shrink_to_fit();
    return pDst;
}
uint8_t * Eva::encodeFrameUndithered(uint8_t *pSrc, int width, int height)
{

    uint8_t *pBmp = pSrc;
    uint8_t *pDst = NULL;
	pDst = (uint8_t*)malloc(FRAMESIZE);
	pBmp = ConvertToGrayscale(pBmp, width, height);		
	pBmp = FlipVertical(pBmp, width, height);	
	
	
	unsigned short off=0;
	unsigned char ytable[5]={0,0,1,2,3};
	

	int c = 0;
	for(int j = 0; j < YSIZE; j++ )
	{
		unsigned short i, x, y, ptr;
		unsigned char indat, outdat;
		int w1;
		int image[XSIZE*2][2];
		unsigned char  wbuff[XSIZE/4];

		for (y=0;y<2;y++)
		{
			for (x=0;x<XSIZE*2;x++)
			{
				indat = pBmp[c];
				c+=3;				
				image[x][y] = (int)indat;				
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

				w1 = (int)round(w1 / 255);


				outdat |=  w1 << ((3-i)*2);
			}
			wbuff[ptr++]=outdat;	
		}
		
		memcpy(&pDst[FRAMESIZE-XSIZE/4-off],wbuff,XSIZE/4);
		off+=XSIZE/4;
	}

	frames.push_back(pDst);
    frames.shrink_to_fit();
    return pDst;
}
void Eva::appendFrame(uint8_t *pFrame)
{
    frames.push_back(pFrame);
    frames.shrink_to_fit();
}

void Eva::dumpInfo()
{
}

void Eva::exportEva3(const char *filename)
{
    int hFileEva = open(filename, O_CREAT|O_RDWR, 0666);
	
	char header[9];
	strcpy((char*)header,"EVA3");
    header[4]= 0x1a;
    header[5]= frames.capacity() & 0xff;
    header[6]= (frames.capacity() & 0xff00) >> 8;
    header[7]= framerate & 0xff;
    header[8]= (framerate & 0xff00) >> 8;
    write(hFileEva, header, 9);
                        
    
	for(int i = 0; i < frames.capacity(); i++)
    {
        write(hFileEva, frames[i], FRAMESIZE);
    }

    close(hFileEva);
}

void Eva::exportEva4(const char *filename)
{

	int hFileEva = open(filename, O_CREAT|O_RDWR, 0666);
	uint8_t kbuf[FRAMESIZE];
	uint8_t* fbuf[8];
	uint8_t obuf[FRAMESIZE * 2];
	
	
	int	i;
	int	frameIdx, totalFrames;
	int	pw;
	int	rc;
	uint8_t	*p0, *p, *p2;
	int	level = 1; //1-4
	int	mode = 0; //0-2
	int currentPtr = 0;

	totalFrames = getFrameCount();

	char header[9];
	strcpy((char*)header,"EVA4");
    header[4]= 0x1a;
    header[5]= frames.capacity() & 0xff;
    header[6]= (frames.capacity() & 0xff00) >> 8;
    header[7]= framerate & 0xff;
    header[8]= (framerate & 0xff00) >> 8;
    
	write(hFileEva, header, 9);

	

	for(i = 0; i < 7; i++)
	{
		currentPtr = i;
		if(i < frames.capacity())
			fbuf[i] = frames[currentPtr];
		else
			fbuf[i] = fbuf[currentPtr-1];
	}
	
	rc = 7;
	p0 = fbuf[0];	
	for(frameIdx = 0; frameIdx < totalFrames; frameIdx++)
	{	uint8_t	*q;
		int	l;
		printf("%d\r", frameIdx + 1);
		
		p = fbuf[frameIdx & 7];
		p2 = fbuf[(frameIdx + 1) & 7];

		
		if(mode == 1)
		{
			for(i = 0; i < FRAMESIZE; i++)
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
					
					if(hc <= FRAMESIZE) p[i] = p0[i];
				}
			}
		}
		
		if(mode == 2)
		{
			for(i = 0; i < FRAMESIZE; i++)
			{
				if(p[i] != p2[i])
				{	int	c, j, j1;
					static int b[4], d[4];
					
					c = p[i];
					b[0] = ((c     ) & 3);
					b[1] = ((c >> 2) & 3);
					b[2] = ((c >> 4) & 3);
					b[3] = ((c >> 6) & 3);

					for(j = 1; j < 7; j++)
					{	int	k, hc;
						
						c = fbuf[(frameIdx+j)&7][i];
						hc = 0;
						
						for(k = 0; k < 4; k++)
						{
							d[k] = (c & 3);
							if(d[k] != ((b[k]+(j/2))/j))
								hc++;
							c >>= 2;
						}

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


					if(j > 1)
					{	c = (((b[0]+(j/2)) / j)     ) +
						    (((b[1]+(j/2)) / j) << 2) +
						    (((b[2]+(j/2)) / j) << 4) +
						    (((b[3]+(j/2)) / j) << 6);

						for(j = 0; j < j1; j++)
							fbuf[(frameIdx+j)&7][i] = c;
					}
				}
			}
		}
		
		for(i = 0; i < FRAMESIZE; i++)		/* �ω��_���o */
		{	int	c;
			
			kbuf[i] = 0;
			c = p[i];
			if(frameIdx > 0 && c == p0[i])
				kbuf[i] = 1;
			else if(i > 0 && c == p[i-1])
				kbuf[i] = 2;
		}

		for(i = 0; i < FRAMESIZE - 2; i++)		/* ���ʂȔ�ω��_������ */
		{	if(kbuf[i] == 0 && kbuf[i+2] == 0)
			 	kbuf[i+1] = 0;
		}

		q = obuf + 2;

		i = 0;
		while(i < FRAMESIZE)
		{	int	s1 = 0, s2 = 0, s3 = 0;
			
			*q = 0;
			if(kbuf[i] == 1 && i < FRAMESIZE)	/* �ω��Ȃ� */
			{	i++;
				s1 = 1;
				while(s1 < 63 && i < FRAMESIZE && kbuf[i] == 1)
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

			if(kbuf[i] == 2 && i < FRAMESIZE)	/* �A���f�[�^ */
			{
				s3 = 0;
				while(s3 < 7 && i < FRAMESIZE && kbuf[i] == 2)
				{	i++;
					s3++;
				}

				*q++ += 0xC0 + s3;
				*q = 0;
			}

			if(kbuf[i] == 0 && i < FRAMESIZE)	/* �ω����� */
			{	int	i0, j;

				i0 = i;
				i++;
				s2 = 1;
				while(s2 < 63 && i < FRAMESIZE && kbuf[i] == 0)
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

		//printf("Length: %i\n", l);		
		//uint8_t* ptr = (uint8_t*)malloc(l);
		//memcpy(ptr, obuf, l);
		//encodedFrames.push_back(ptr);
		
		write(hFileEva, &obuf, l);

		
		p0 = p;
		
		if(rc < totalFrames)
			fbuf[rc & 7] = frames[currentPtr++];
		else
			fbuf[rc &  7] = fbuf[(rc-1) & 7];
		rc++;
	}

/*
	encodedFrames.shrink_to_fit();
	for(int i = 0; i < encodedFrames.capacity(); i++)
    {
		
		int length = encodedFrames[i][1] << 8 | encodedFrames[i][0] ;
		printf("Length: %i\n", length);
        write(hFileEva, encodedFrames[i], length);
    }
*/
    close(hFileEva);

}

double Eva::getLength()
{
    return frames.capacity() / framerate;
}

void Eva::setFrameRate(int fps)
{
    framerate = fps;
}

int Eva::getFramerate()
{
    return framerate;
}

long Eva::getFrameCount()
{
    return frames.capacity();
}

int Eva::getWidth()
{
    return XSIZE;
}

int Eva::getHeight()
{
    return YSIZE;
}

std::vector<uint8_t *> Eva::getFrames()
{
    return std::vector<uint8_t *>();
}
