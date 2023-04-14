#include "defs.h"


BOOL Mono(LPBYTE pDst, int nWidth, int nHeight)
{
	int		u,v,y;

	for(v = 0; v < nHeight; v++){
		for(u = 0; u < nWidth; u++){
			y = ((int)*(pDst+0) * 153 + (int)*(pDst+1) * 27 + (int)*(pDst+2) * 76) / 256;
			*pDst++ = (BYTE)(y & 255);
			*pDst++ = (BYTE)(y & 255);
			*pDst++ = (BYTE)(y & 255);
		}
	}
	return TRUE;
}
