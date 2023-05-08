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




class Video
{

public:
    Video(const char* infile);
    ~Video();

    void* getFrameAt(int64_t timestamp);
    bool decodeAllFrames();
    bool decodeAudioFrames();
    bool decodeFrames(int fps);
    bool prepareScaler(int dst_width, int dst_height);
    void* getAudio();
    void dumpInfo();
    void decodeAudio();
    void* getAudioFrameAt(int64_t timestamp);
    void dumpAudioInfo(const AVCodecContext* codecContext, const AVFrame* frame);
    int saveFrameToFile(uint8_t* frame, int width, int height);

    double getLength();
    double getFramerate();
    long getFrameCount();
    int getWidth();
    int getHeight();
    int getSampleRate();
    std::vector<uint8_t*> getFrames();
    std::vector<uint8_t*> getAudioFrames();

private:
    double framerate;
    double length;
    int width;
    int height;
    long total_frames;
    const char* infile;
    int vstrm_idx;
    int astrm_idx;
    AVFormatContext* inFormatCtx;
    const AVCodec* inVideoCodec;
    const AVCodec* inAudioCodec;
    AVStream* inVideoStrm;
    AVStream* inAudioStrm;
    AVCodecContext *vDecoderCtx;
    AVCodecContext *aDecoderCtx;
    SwsContext* swsctx;
    int sampleRate;

    std::vector<uint8_t*> frames;
    std::vector<uint8_t*> audio_frames;
};

#endif


