#include <windows.h>


int		nDstHPer, nDstVPer;
int		nSrcHPer, nSrcVPer;
int		nDstWidth, nDstHeight;
int		nSrcWidth, nSrcHeight;


int gcd(int a, int b)
{
	int		c;

	if(a < 0) a = -a;
	if(b < 0) b = -b;
	if(a < b){
		c = a;
		a = b;
		b = c;
	}
	while(b>0){
		c = a;
		a = b;
		b = c % a;
	}
	return a;
}


void makePer(int *a, int *b)
{
	int		c = gcd(*a, *b);
	*a /= c;
	*b /= c;
	return;
}


void SetStretchPer(int nDstWidth, int nDstHeight, int nSrcWidth, int nSrcHeight)
{
	::nDstWidth = nDstWidth;
	::nDstHeight = nDstHeight;
	::nSrcWidth = nSrcWidth;
	::nSrcHeight = nSrcHeight;

	nDstHPer = nDstWidth;
	nSrcHPer = nSrcWidth;
	makePer(&nSrcHPer, &nDstHPer);

	nDstVPer = nDstHeight;
	nSrcVPer = nSrcHeight;
	makePer(&nSrcVPer, &nDstVPer);
}


void StretchHorz(LPBYTE pDst, LPBYTE pSrc)
{
	int		i, j;
	int		sumR, sumG, sumB;
	int		cnt;
	int		n = nSrcHeight;
	int		nn;

	while(n--){
		cnt = 0;
		for(i = 0; i < nDstWidth; i++){
			sumR = sumB = sumG = 0;
			nn = nSrcHPer;
			if(cnt){
				cnt = nDstHPer - cnt;
				sumR += pSrc[0] * cnt;
				sumG += pSrc[1] * cnt;
				sumB += pSrc[2] * cnt;
				nn -= cnt;
				pSrc += 3;
			}
			while(nn >= nDstHPer){
				sumR += pSrc[0] * nDstHPer;
				sumG += pSrc[1] * nDstHPer;
				sumB += pSrc[2] * nDstHPer;
				nn -= nDstHPer;
				pSrc += 3;
			}
			cnt = nn;
			if(cnt){
				sumR += pSrc[0] * cnt;
				sumG += pSrc[1] * cnt;
				sumB += pSrc[2] * cnt;
			}

			*pDst++ = (BYTE)(sumR / nSrcHPer);
			*pDst++ = (BYTE)(sumG / nSrcHPer);
			*pDst++ = (BYTE)(sumB / nSrcHPer);
		}
	}
}


void StretchVert(LPBYTE pDst, LPBYTE pSrc)
{
	int		i, j;
	int		sumR, sumG, sumB;
	int		cnt;
	LPBYTE	p;
	int		nSrcPitch = nDstWidth*3;
	int		nDstPitch = nDstWidth*3;
	int		n = nDstWidth;
	int		nn;

	while(n--){
		cnt = 0;
		p = pSrc;
		for(i = 0; i < nDstHeight; i++){
			sumR = sumB = sumG = 0;
			nn = nSrcVPer;
			if(cnt){
				cnt = nDstVPer - cnt;
				sumR += p[0] * cnt;
				sumG += p[1] * cnt;
				sumB += p[2] * cnt;
				nn -= cnt;
				p += nSrcPitch;
			}
			while(nn >= nDstVPer){
				sumR += p[0] * nDstVPer;
				sumG += p[1] * nDstVPer;
				sumB += p[2] * nDstVPer;
				nn -= nDstVPer;
				p += nSrcPitch;
			}
			cnt = nn;
			if(cnt){
				sumR += p[0] * cnt;
				sumG += p[1] * cnt;
				sumB += p[2] * cnt;
			}

			*(pDst + i * nDstPitch + 0) = (BYTE)(sumR / nSrcVPer);
			*(pDst + i * nDstPitch + 1) = (BYTE)(sumG / nSrcVPer);
			*(pDst + i * nDstPitch + 2) = (BYTE)(sumB / nSrcVPer);
		}
		pDst += 3;
		pSrc += 3;
	}
}


BOOL Stretch(LPBYTE pDst, LPBYTE pSrc)
{
	LPVOID	pTmp;

	pTmp = malloc(nDstWidth * nSrcHeight * 3);
	if(pTmp == NULL) return FALSE;

	StretchHorz((LPBYTE)pTmp, (LPBYTE)pSrc);
	StretchVert((LPBYTE)pDst, (LPBYTE)pTmp);

	free(pTmp);
	return TRUE;
}
