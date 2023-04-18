#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

bool CloseEVA();
bool CreateEVA(const char * lpszFilename, bool bEnh);
bool AppendEVA();
bool ConvEva(void* lpSurface, int nWidth, int nHeight, void* lpSound, int nSize, bool bDither, bool bMono);
