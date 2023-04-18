#ifndef VIDEO_H
#define VIDEO_H

#include <iostream>
#include <vector>
// FFmpeg
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/pixdesc.h>
    #include <libswscale/swscale.h>
}




class video
{

public:
    video(const char* infile);
    ~video();

    void* getFrameAt(int64_t timestamp);
    bool decodeAllFrames();
    bool decodeFrames(int fps);
    bool prepareScaler(int dst_width, int dst_height);
    void dumpInfo();

    
    int saveFrameToFile(uint8_t* frame, int width, int height);

    double getLength();
    double getFramerate();
    long getFrameCount();
    int getWidth();
    int getHeight();
    std::vector<uint8_t*> getFrames();

private:
    double framerate;
    double length;
    int width;
    int height;
    long total_frames;
    const char* infile;
    int vstrm_idx;
    AVFormatContext* inFormatCtx;
    const AVCodec* inVideoCodec;
    AVStream* inVideoStrm;
    AVCodecContext *decoderCtx;
    SwsContext* swsctx;

    std::vector<uint8_t*> frames;
};

#endif


