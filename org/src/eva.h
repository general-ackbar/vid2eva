#include <windows.h>

BOOL CloseEVA();
BOOL CreateEVA(LPCSTR lpszFilename, BOOL bEnh);
BOOL AppendEVA();
BOOL ConvEva(LPVOID lpSurface, int nWidth, int nHeight, LPVOID lpSound, int nSize, BOOL bDither, BOOL bMono);
