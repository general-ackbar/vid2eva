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

uint8_t * Eva::encodeFrame(uint8_t *pSrc, int width, int height)
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

void Eva::exportEva4(const char *file)
{
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
