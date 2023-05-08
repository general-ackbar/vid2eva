/********************************************************/
/*							*/
/* EVA4to5 : EVA5 �f�[�^�쐬�v���O����			*/
/*							*/
/*    eva4to5  file1  file2  file3			*/
/*	input						*/
/*		file1 : EVA4 data			*/
/*		file2 : PCM data (4bit PCM, 15KHz)	*/
/*	output						*/
/*		file3 : EVA5 data			*/
/*							*/
/********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	BYTE	unsigned char
#define	WORD	unsigned short

/* constants */
#define	XSIZE	24
#define	YSIZE	92
#define	FSIZE	XSIZE * YSIZE
#define	PSIZE	15600
#define	XDOT	96
#define	YDOT	92
#define	WFREQ	15600

/* public declaration */

static char filename[3][256];

/* sub functions */

void	err_exit(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}


void	usage()
{
	err_exit("EVA4to5 -- EVA4 to EVA5 Converter   "
		 " by T.Yamamoto\n\n"
		 " Usage: EVACV <EVA4 file> <PCM file> <output file>\n"
		 "\n");
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
		if(*p == '-' )
		{
			usage();
		} else
		{
			if(fc > 2) usage();
			strcpy(filename[fc], p);
			if(fc == 2) fnmake(filename[fc]);
			fc++;
		}
	}
	if(fc != 3) usage();
}


/* main function */

 int main(int argc, char *argv[])
{
	static char header[256];
	static BYTE fbuf[FSIZE * 2];
	static BYTE pbuf[PSIZE];
	FILE	*fp_video, *fp_audio, *fp_output;
	unsigned int	t, total_frames;
	double	pw;

	getcomlin(argc, argv);

	fp_video=fopen(filename[0], "rb");
	if(fp_video == NULL)
		err_exit("Input file not found.");

	fp_audio=fopen(filename[1], "rb");
	if(fp_audio == NULL)
		err_exit("PCM file not found.");

	fp_output = fopen(filename[2], "wb");
	if(fp_output == NULL)
		err_exit("Output file open error.");


	
	fread(header, 1, 4, fp_video);
	if(strncmp(header, "EVA4", 4) != 0)
		err_exit("���̃t�@�C���� EVA4 �`���ł͂���܂���B");
	while(fgetc(fp_video)!= '\x1A' && feof(fp_video) == 0)
		;
	total_frames = fgetc(fp_video);						//frames
	total_frames = total_frames + (fgetc(fp_video) << 8);
	printf("frames = %d\n", total_frames);

	pw = fgetc(fp_video);						//fps
	fgetc(fp_video);								//0x00 -> end of header

	//EVA5 Header: EVA5 {0..n} 0x1A (n+1 bytes) {frames}(2 bytes) xdots (2 bytes) ydots (2 bytes) (bits)

	fwrite("EVA5\x1A", 1, 5, fp_output);		/* �^�C�g���@���@�R�����g */
	fputc(total_frames & 255, fp_output);			//frames 
	fputc(total_frames >> 8, fp_output);
	fputc(XDOT & 255, fp_output);			/* �������h�b�g��         */
	fputc(XDOT >> 8, fp_output);
	fputc(YDOT & 255, fp_output);			/* �c�����h�b�g��         */
	fputc(YDOT >> 8, fp_output);
	fputc(2, fp_output);				/* �F�r�b�g��             */
	fputc(0x18, fp_output);			/* �c����i�����c�~0x10�j */
	fputc(1,fp_output);				/* PCM Format		  */
	fputc(WFREQ & 255, fp_output);		/* PCM ���g��             */
	fputc(WFREQ >> 8, fp_output);
	pw = WFREQ / pw / 2;
	fputc((int)pw & 255, fp_output);		/* ��R�}����PCM�f�[�^��  */
	fputc((int)pw >> 8, fp_output);
	fputc(0, fp_output);				/* �\��                   */
	
	
	for(t = 0; t < total_frames; t++)
	{
		int	fl1, fl2, fl;
		
		printf("%d\r", t);
		
		memset(pbuf, '\0', sizeof(pbuf));				//zero, read and write same chunk size of video
		fread(pbuf, 1, pw, fp_audio);
		fwrite(pbuf, 1, pw, fp_output);
		
		fl1 = fgetc(fp_video); 							//Read next frame-size
		fl2 = fgetc(fp_video);
		fl = fl1+ (fl2 << 8);
		if(fread(fbuf, 1, fl, fp_video) != fl)
			err_exit("�t�@�C�������Ă��܂��B");
		
		fputc(fl1, fp_output);							//Write next frame-size
		fputc(fl2, fp_output);
		if(fwrite(fbuf, 1, fl, fp_output) != fl)		//Write frame data
			err_exit("�������ݏo���܂���B");
	}
	fclose(fp_video);
	fclose(fp_audio);
	fclose(fp_output);
	
	printf("\n");
}
