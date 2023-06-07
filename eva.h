#ifndef EVA_H
#define EVA_H

#include <vector>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include "imageops.h"


#define XSIZE 96
#define YSIZE 92
#define FRAMESIZE XSIZE/4 * YSIZE

class Eva
{

public:
    Eva(const char* infile);
    Eva(int fps);
    Eva();
    ~Eva();

    uint8_t *getFrameAt(int index);
    uint8_t *encodeFrame(uint8_t *pSrc, int width, int height, DitherMode ditherMode);
    void dumpInfo();
    void appendFrame(uint8_t* pFrame);
    void exportEva3(const char* file);
    void exportEva4(const char* file);

    
    double getLength();
    void setFrameRate(int fps);
    int getFramerate();
    long getFrameCount();
    int getWidth();
    int getHeight();
    std::vector<uint8_t*> getFrames();


private:
    int framerate;
    int total_frames;
    double length;
    int width;
    int height;

    std::vector<uint8_t*> frames;
    std::vector<uint8_t*> encodedFrames;


    uint8_t *encodeFrameDithered(uint8_t *pSrc, int width, int height, DitherMode ditherMode);
    uint8_t *encodeFrameUndithered(uint8_t *pSrc, int width, int height);

};

#endif


