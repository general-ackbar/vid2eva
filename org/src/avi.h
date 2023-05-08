#include <windows.h>

BOOL CloseAVI();
BOOL OpenAVI(LPSTR lpszFile);
void AVISetVideoSize(int nWidth, int nHeight);
int AVIGetVideoFrame();
int AVIGetVideoRate();
int AVIGetVideoScale();
int AVIGetVideoWidth();
int AVIGetVideoHeight();
BOOL AVIIsEndAudio();
BOOL AVIReadVideo(LPVOID lpDst, int nFrame);
BOOL AVIReadAudio(LPVOID lpDst, int nSize);
