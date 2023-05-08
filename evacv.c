/*   EVA3�i�����k�`���j �� EVA4�i���k�`���j�R���o�[�^	*/
/*     �i�t���k�^��t���k�Ή��j			*/


#define	VER	"2.01"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	BYTE	unsigned char
#define	WORD	unsigned short

/* constants */
#define	XSIZE	24
#define	YSIZE	92
#define	FSIZE	XSIZE * YSIZE
#define	XDOT	96
#define	YDOT	92

/* public declaration */

int	level = 1;
int	mode = 0;
static char filename[2][256];
/* sub functions */

void	err_exit(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}


void	usage()
{
	err_exit("EVACV -- EVA3 to EVA4 format converter version " VER
		 " by T.Yamamoto\n\n"
		 " Usage: EVACV [switch] <input name> <output name>\n\n"
		 "   -Mn   mode  (0~2 , default=0)\n"
		 "   -Ln   level (1~4 , default=1)\n"
		 "\n");
	exit(0);
}


void	fnmake(char *fn)
{
	char	*p, *p0;
	
	p = fn;
	while(*p++ != '\0')
		;
	p0 = --p;

	while(*p != '.' && *p != '\\' && *p != '/' && *p != ':'
	      && p >= fn)
		--p;
	
	if(*p != '.') strcpy(p0, ".EVA");
}


void	getcomlin(int argc, char *argv[])	/* analyze command line */
{
	int	i;
	int	fc = 0;

	if(argc < 2) usage();

	for(i = 1; i < argc; i++)
	{	char	*p;
		p = argv[i];
		if(*p == '-')
		{	switch(*++p)
			{	case 'l':
				case 'L': level = atoi(p+1);
					  if(level < 1 || level > 4)
						usage();
					  break;
				case 'm':
				case 'M': mode = atoi(p+1);
					  if(mode < 0 || mode > 2)
					  	usage();
					  break;
				default : usage();
			}
		} else
		{
			if(fc > 1) usage();
			strcpy(filename[fc], p);
			fnmake(filename[fc]);
			fc++;
		}
	}
	if(fc != 2) usage();
}


/* main function */

int main(int argc, char *argv[])
{
	static BYTE kbuf[FSIZE];
	static BYTE fbuf[8][FSIZE];
	static BYTE obuf[FSIZE * 2];
	static char header[FSIZE];

	FILE	*fp1, *fp2;
	int	i;
	int	t, tmax;
	int	pw;
	int	rc;
	BYTE	*p0, *p, *p2;

	getcomlin(argc, argv);

	fp1=fopen(filename[0], "rb");
	if(fp1 == NULL)
		err_exit("Input file not found.");

	fp2 = fopen(filename[1], "wb");
	if(fp2 == NULL)
		err_exit("Output file open error.");

	//EVA3 header = EVA3 0x1A frames (2 byte) fps (1 byte)

	fread(header, 1, 4, fp1);
	if(strncmp(header, "EVA3", 4) != 0)
		err_exit("���̃t�@�C���� EVA3 �`���ł͂���܂���B");
	while(fgetc(fp1)!= '\x1A' && feof(fp1) == 0)
		;
	tmax = fgetc(fp1);
	tmax = tmax + (fgetc(fp1) << 8);
	printf("frames = %d\n", tmax);

	pw = fgetc(fp1);
	fgetc(fp1);


	//EVA4 header = EVA4 0x1A frames (2 byte) fps (1 byte) + 0x00
	//EVA3 image/frame size = FSIZE = 24*92
	/* 'E','V','A','4',1A, �t���[����.����, �t���[����.���, �b�ԃt���[����, �\�� */
	fwrite("EVA4\x1A", 1, 5, fp2);
	fputc(tmax & 255, fp2);
	fputc(tmax >> 8, fp2);
	fputc(pw, fp2);
	fputc(0, fp2);

	for(i = 0; i < 7; i++)
	{
		if(i < tmax)
			fread(fbuf[i], 1, FSIZE, fp1);
		else
			memcpy(fbuf[i], fbuf[i-1], FSIZE);
	}
	
	rc = 7;
	p0 = fbuf[0];
	
	for(t = 0; t < tmax; t++)
	{	BYTE	*q;
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
		if(fwrite(obuf, 1, l, fp2) != l)
			err_exit("file write error");
		
		p0 = p;
		
		if(rc < tmax)
			fread(fbuf[rc & 7], 1, FSIZE, fp1);
		else
			memcpy(fbuf[rc &  7], fbuf[(rc-1) & 7], FSIZE);
		rc++;
	}

	fclose(fp1);
	fclose(fp2);
	

	printf("\n");
}

