#include <windows.h>


BYTE	EvaBuff[0x3C00];
BYTE	EvaBuffTmp[0x3C00];
HANDLE	hFileEva;
BOOL	bEnhance;


int		nDitherPattern[4][4] = {
			{ 0,  4,  0,  4},
			{ 6,  2,  6,  2},
			{ 1,  5,  1,  5},
			{ 7,  3,  7,  3}
		};


BOOL convYJKMonoDither(LPBYTE lpDst, LPVOID lpSurface, int nWidth, int nHeight)
{
	int		u, v;
	LPBYTE	lpSrc;
	int		nGap = nWidth - 128;
	int		nBorder0, nBorder1;

	nBorder0 = (106 - nHeight) / 2;
	if(nBorder0 < 0) nBorder0 = 0;
	nBorder1 = nBorder0 + nHeight;
	if(nBorder1 > 106) nBorder1 = 106;

	lpSrc = (LPBYTE)lpSurface + (nGap / 2) * 3;
	if(nBorder0 == 0) lpSrc += ((nHeight - 106) / 2) * 3;
	lpSrc += ((nHeight-1) * nWidth * 3);
	nGap *= 3;

	for(v = 0; v < 106; v++){
		if(v < nBorder0 || v >= nBorder1){
			ZeroMemory(lpDst, 128);
			lpDst+=128;
		} else {
			for(u = 0; u < 128; u++){
				*lpDst++ = (BYTE)((*lpSrc + nDitherPattern[v&3][u&3]) & 0xF8);
				lpSrc+=3;
			}
			lpSrc += nGap;
			lpSrc -= nWidth * 6;
		}
	}
	return TRUE;
}


BOOL convYJKMono(LPBYTE lpDst, LPVOID lpSurface, int nWidth, int nHeight)
{
	int		u, v;
	LPBYTE	lpSrc;
	int		nGap = nWidth - 128;
	int		nBorder0, nBorder1;

	nBorder0 = (106 - nHeight) / 2;
	if(nBorder0 < 0) nBorder0 = 0;
	nBorder1 = nBorder0 + nHeight;
	if(nBorder1 > 106) nBorder1 = 106;

	lpSrc = (LPBYTE)lpSurface + (nGap / 2) * 3;
	if(nBorder0 == 0) lpSrc += ((nHeight - 106) / 2) * 3;
	lpSrc += ((nHeight-1) * nWidth * 3);
	nGap *= 3;

	for(v = 0; v < 106; v++){
		if(v < nBorder0 || v >= nBorder1){
			ZeroMemory(lpDst, 128);
			lpDst+=128;
		} else {
			for(u = 0; u < 128; u++){
				*lpDst++ = (BYTE)(*lpSrc & 0xF8);
				lpSrc+=3;
			}
			lpSrc += nGap;
			lpSrc -= nWidth * 6;
		}
	}
	return TRUE;
}


BOOL convYJKDither(LPBYTE lpDst, LPVOID lpSurface, int nWidth, int nHeight)
{
	int		u, v, i;
	int		r[4],g[4],b[4],y[4];
	int		i0,i1,i2,i3;
	int		yy, j, k;
	LPBYTE	lpSrc;
	int		nGap = nWidth - 128;
	int		nBorder0, nBorder1;

	nBorder0 = (106 - nHeight) / 2;
	if(nBorder0 < 0) nBorder0 = 0;
	nBorder1 = nBorder0 + nHeight;
	if(nBorder1 > 106) nBorder1 = 106;

	lpSrc = (LPBYTE)lpSurface + (nGap / 2) * 3;
	if(nHeight > 106){
		lpSrc += (((nHeight - 106) / 2) + 106-1)* 3 * nWidth;
	} else {
		lpSrc += ((nHeight-1) * nWidth * 3);
	}
	nGap *= 3;

	for(v = 0; v < 106; v++){
		if(v < nBorder0 || v >= nBorder1){
			ZeroMemory(lpDst, 128);
			lpDst+=128;
		} else {
			for(u = 0; u < 128; u+=4){
				/* conv */
				yy = j = k = 0;
				for(i=0;i<4;i++){
					b[i] = *lpSrc++;
					g[i] = *lpSrc++;
					r[i] = *lpSrc++;
					y[i] = b[i] / 2 + r[i] / 4 + g[i] / 8;
					yy += y[i] + nDitherPattern[v&3][i];
					j  += r[i];
					k  += g[i];
				}

				j  = (j - yy) / 4;
				k  = (k - yy) / 4;

				for(i = 0; i < 4; i++){
					int y0=(((b[i]+r[i]*2+g[i]*4)*4)-6*j-15*k)/29;
					int i0,i1,i2,i3;
					i3 = (g[i]*153 + r[i]*76 + b[i]*27) / 256;
					i0 = i3 ? r[i]*256/i3 : 256;
					i1 = i3 ? g[i]*256/i3 : 256;
					i2 = i3 ? b[i]*256/i3 : 256;
					i3 = i0+i1+i2;
					int y1 = i3 ? ((b[i] * 4 + j * 2 + k)*i2 / 5 + (r[i] - j)*i0 + (g[i] - k)*i1) / i3 : 255;
					i0 = r[i]-(b[i]+g[i])/2+255;
					y[i] = (y1-y0)*i0/511+y0;
					y[i] += nDitherPattern[v&3][i];
					y[i] /= 8;
					if(y[i] < 0) y[i] = 0;
					if(y[i] > 31) y[i] = 31;
				}

				j = j >> 3;
				k = k >> 3;

				/* write */
				*lpDst++ = (BYTE)(((y[0] & 31) << 3) | ( k       & 7));
				*lpDst++ = (BYTE)(((y[1] & 31) << 3) | ((k >> 3) & 7));
				*lpDst++ = (BYTE)(((y[2] & 31) << 3) | ( j       & 7));
				*lpDst++ = (BYTE)(((y[3] & 31) << 3) | ((j >> 3) & 7));
			}
			lpSrc += nGap;
			lpSrc -= nWidth * 6;
		}
	}
	return TRUE;
}


BOOL convYJK(LPBYTE lpDst, LPVOID lpSurface, int nWidth, int nHeight)
{
	int		u, v, i;
	int		r[4],g[4],b[4],y[4];
	int		i0,i1,i2,i3;
	int		yy, j, k;
	LPBYTE	lpSrc;
	int		nGap = nWidth - 128;
	int		nBorder0, nBorder1;

	nBorder0 = (106 - nHeight) / 2;
	if(nBorder0 < 0) nBorder0 = 0;
	nBorder1 = nBorder0 + nHeight;
	if(nBorder1 > 106) nBorder1 = 106;

	lpSrc = (LPBYTE)lpSurface + (nGap / 2) * 3;
	if(nHeight > 106){
		lpSrc += (((nHeight - 106) / 2) + 106-1)* 3 * nWidth;
	} else {
		lpSrc += ((nHeight-1) * nWidth * 3);
	}
	nGap *= 3;

	for(v = 0; v < 106; v++){
		if(v < nBorder0 || v >= nBorder1){
			ZeroMemory(lpDst, 128);
			lpDst+=128;
		} else {
			for(u = 0; u < 128; u+=4){
				/* conv */
				yy = j = k = 0;
				for(i=0;i<4;i++){
					b[i] = *lpSrc++;
					g[i] = *lpSrc++;
					r[i] = *lpSrc++;
					y[i] = b[i] / 2 + r[i] / 4 + g[i] / 8;
					yy += y[i];
					j  += r[i];
					k  += g[i];
				}

				j  = (j - yy) / 4;
				k  = (k - yy) / 4;

				for(i = 0; i < 4; i++){
					i3 = (g[i]*153 + r[i]*76 + b[i]*27) / 256;
					i0 = i3 ? r[i]*256/i3 : 256;
					i1 = i3 ? g[i]*256/i3 : 256;
					i2 = i3 ? b[i]*256/i3 : 256;
					i3 = i0+i1+i2;
					y[i] = i3 ? ((b[i] * 4 + j * 2 + k)*i2 / 5 + (r[i] - j)*i0 + (g[i] - k)*i1) / i3 : 255;
					y[i] /= 8;
					if(y[i] < 0) y[i] = 0;
					if(y[i] > 31) y[i] = 31;
				}

				j = j >> 3;
				k = k >> 3;

				/* write */
				*lpDst++ = (BYTE)(((y[0] & 31) << 3) | ( k       & 7));
				*lpDst++ = (BYTE)(((y[1] & 31) << 3) | ((k >> 3) & 7));
				*lpDst++ = (BYTE)(((y[2] & 31) << 3) | ( j       & 7));
				*lpDst++ = (BYTE)(((y[3] & 31) << 3) | ((j >> 3) & 7));
			}
			lpSrc += nGap;
			lpSrc -= nWidth * 6;
		}
	}
	return TRUE;
}


BOOL convPCM(LPBYTE lpDst, LPVOID lpSound, int nSize)
{
	CopyMemory(lpDst, lpSound, nSize);
	return TRUE;
}





BOOL CloseEVA()
{
	if(hFileEva != INVALID_HANDLE_VALUE) CloseHandle(hFileEva);
	hFileEva = INVALID_HANDLE_VALUE;
	return TRUE;
}


BOOL CreateEVA(LPCSTR lpszFilename, BOOL bEnh)
{
	CloseEVA();
	hFileEva = CreateFile(lpszFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	bEnhance = bEnh;
	return hFileEva != INVALID_HANDLE_VALUE;
}


BOOL AppendEVA()
{
	DWORD	dwWritten;

	if(hFileEva == INVALID_HANDLE_VALUE) return FALSE;
	if(!WriteFile(hFileEva, EvaBuff, 0x3C00, &dwWritten, NULL)) dwWritten = 0;
	return dwWritten == 0x3C00;
}


BOOL ConvEva(LPVOID lpSurface, int nWidth, int nHeight, LPVOID lpSound, int nSize, BOOL bDither, BOOL bMono)
{
	LPBYTE	lpBuff;

	lpBuff = bEnhance ? EvaBuffTmp : EvaBuff;

	/* Zero */
	ZeroMemory(lpBuff, 0x3c00);

	/* picture */
	if(lpSurface){
		if(bMono){
			if(bDither){
				if(!convYJKMonoDither(lpBuff + 0, lpSurface, nWidth, nHeight)) return FALSE;
			} else {
				if(!convYJKMono(lpBuff + 0, lpSurface, nWidth, nHeight)) return FALSE;
			}
		} else {
			if(bDither){
				if(!convYJKDither(lpBuff + 0, lpSurface, nWidth, nHeight)) return FALSE;
			} else {
				if(!convYJK(lpBuff + 0, lpSurface, nWidth, nHeight)) return FALSE;
			}
		}
	} else {
		ZeroMemory(lpBuff, 0x3500);
	}

	/* sound */
	if(nSize >= 0x3BFE - 0x3500) return FALSE;
	if(lpSound){
		if(!convPCM(lpBuff + 0x3500, lpSound, nSize)) return FALSE;
	} else {
		memset(lpBuff + 0x3500, 0x80, nSize);
	}

	/* size */
	lpBuff[0x3BFE] = (BYTE)(nSize & 255);
	lpBuff[0x3BFF] = (BYTE)((nSize >> 8) & 255);

	/* Enhance */
	if(bEnhance){
		LPBYTE	p = EvaBuff;
		LPBYTE	pPcm = lpBuff + 0x3500;
		LPBYTE	pVideo = lpBuff;
		for(int i = 0; i < 106; i++){
			for(int j = 0; j < 11; j++){
				*p++ = *pPcm++;
				CopyMemory(p, pVideo, 11);
				pVideo += 11;
				p += 11;
			}
			*p++ = *pPcm++;
			CopyMemory(p, pVideo, 7);
			pVideo += 7;
			p += 7;
		}
		if(nSize > 1272){
			CopyMemory(p, pPcm, nSize - 1272);
		}
		EvaBuff[0x3BFE] = (BYTE)(nSize & 255);
		EvaBuff[0x3BFF] = (BYTE)((nSize >> 8) & 255);
	}

	return TRUE;
}
