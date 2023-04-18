#include "video.h"
#include <math.h>




video::video(const char* path)
{

    int ret;
    infile = path;
    // open input file context
    inFormatCtx = nullptr;
    ret = avformat_open_input(&inFormatCtx, path, nullptr, nullptr);
    if (ret < 0) {
        std::cerr << "fail to avforamt_open_input(\"" << path << "\"): ret=" << ret;
        return;
    }
    // retrive input stream information
    ret = avformat_find_stream_info(inFormatCtx, nullptr);
    if (ret < 0) {
        std::cerr << "fail to avformat_find_stream_info: ret=" << ret;
        return;
    }

    // find primary video stream
    inVideoCodec = nullptr;
    ret = av_find_best_stream(inFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &inVideoCodec, 0);
    
    if (ret < 0) {
        std::cerr << "fail to av_find_best_stream: ret=" << ret;
        return;
    }
    vstrm_idx = ret;
    inVideoStrm = inFormatCtx->streams[vstrm_idx];

    // open video decoder context    
    decoderCtx = avcodec_alloc_context3(inVideoCodec);
    ret = avcodec_open2(decoderCtx, inVideoCodec, nullptr);
    
    if (ret < 0) {
        std::cerr << "fail to avcodec_open2: ret=" << ret;
        return;
    }

    width = inVideoStrm->codecpar->width;
    height = inVideoStrm->codecpar->height;
    framerate = av_q2d(inVideoStrm->r_frame_rate);
    length = av_rescale_q(inVideoStrm->duration, inVideoStrm->time_base, av_make_q(1, 1000)) / 1000;
    total_frames = inVideoStrm->nb_frames;
}

video::~video() {
    avcodec_close(decoderCtx);
    avformat_close_input(&inFormatCtx);

}
void* video::getFrameAt(int64_t timestamp)
{
    AVFrame* dstFrame = av_frame_alloc();
    AVFrame* decframe = av_frame_alloc();
    AVPacket pkt;

    av_seek_frame(inFormatCtx, -1,timestamp, 1);
    av_read_frame(inFormatCtx, &pkt);

    avcodec_send_packet(decoderCtx, &pkt);
    avcodec_receive_frame(decoderCtx, decframe);

    sws_scale_frame(swsctx, dstFrame, decframe);

    av_packet_unref(&pkt);
    av_frame_free(&decframe);
    //av_frame_free(&dstFrame);
        
    return dstFrame->data[0];
}

bool video::decodeAllFrames() {

    
    unsigned nb_frames = 0;
    bool end_of_stream = false;
    AVPacket pkt;    
    int ret = 0;
    
    AVFrame* dstframe;
    AVFrame*  decframe;
    
    do {

        dstframe = av_frame_alloc();
        decframe = av_frame_alloc();    

        if (!end_of_stream) {
            // read packet from input file
            ret = av_read_frame(inFormatCtx, &pkt);
            

            if (ret < 0 && ret != AVERROR_EOF) {
                std::cerr << "fail to av_read_frame: ret=" << ret;
                return 2;
            }
            if (ret == 0 && pkt.stream_index != vstrm_idx)
            {
                printf("wrong stream");
                goto next_packet;
            }

            end_of_stream = (ret == AVERROR_EOF);
        }
        if (end_of_stream) {
            printf("End of stream\n");
            // null packet for bumping process
            av_init_packet(&pkt);
            pkt.data = nullptr;
            pkt.size = 0;
        }

        // decode video frame
        ret = avcodec_send_packet(decoderCtx, &pkt);
        ret = avcodec_receive_frame(decoderCtx, decframe);
        
        if(ret)
        {
            printf("Error decoding frame: %i\n", nb_frames);
            goto next_packet;
        }
            


        // convert / scale frame 
        sws_scale_frame(swsctx, dstframe, decframe);        
        frames.push_back(dstframe->data[0]);    

        std::cout << nb_frames << '\r' << std::flush;  // dump progress
        ++nb_frames;
    next_packet:
        av_packet_unref(&pkt);
    } while (!end_of_stream );

    av_frame_free(&decframe);
    av_frame_free(&dstframe);

//    std::cout << nb_frames << " frames decoded" << std::endl;

    
}
bool video::decodeFrames(int fps) {

    int64_t step = (double)(1.0/fps) * AV_TIME_BASE;

    printf("step: %ld\n", step);

    for(uint64_t t = 0; t < length * AV_TIME_BASE; t+=step)
    {
//        printf("current stamp: %ld\n", t);
        frames.push_back((uint8_t*)getFrameAt(t));
    }

    return true;

}
bool video::prepareScaler(int dst_width, int dst_height)
{
    // initialize sample scaler
    const AVPixelFormat dst_pix_fmt = AV_PIX_FMT_RGB24;

    swsctx = sws_getCachedContext(
        nullptr, inVideoStrm->codecpar->width, inVideoStrm->codecpar->height, (AVPixelFormat)inVideoStrm->codecpar->format,
        dst_width, dst_height, dst_pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (!swsctx) {
        std::cerr << "fail to sws_getCachedContext";
        return false;
    }
    std::cout << "output: " << dst_width << 'x' << dst_height << ',' << av_get_pix_fmt_name(dst_pix_fmt) << std::endl;
}
void video::dumpInfo()
{
    // print input video stream informataion
    std::cout
        << "infile: " << infile << "\n"
        << "format: " << inFormatCtx->iformat->name << "\n"
        << "vcodec: " << inVideoCodec->name << "\n"
        << "size:   " << inVideoStrm->codecpar->width << 'x' << inVideoStrm->codecpar->height << "\n"
        << "frame rate:    " << av_q2d(inVideoStrm->r_frame_rate) << " [fps]\n"
        << "length: " << av_rescale_q(inVideoStrm->duration, inVideoStrm->time_base, av_make_q(1, 1000)) / 1000. << " [sec]\n"
        << "pixfmt: " <<  av_get_pix_fmt_name((AVPixelFormat) inVideoStrm->codecpar->format) << "\n"
        << "frame:  " << inVideoStrm->nb_frames << "\n"
        << std::flush;
}

double video::getLength(){ return length; }
double video::getFramerate(){ return framerate; }
long video::getFrameCount(){ return total_frames;}
int video::getWidth(){ return width; }
int video::getHeight(){ return height; }
std::vector<uint8_t*> video::getFrames(){ return frames; }


int video::saveFrameToFile(uint8_t* frame, int width, int height)
{
         FILE* fileHandle;
        int y, writeError;
        char filename[32];
        static int frameNumber = 0;

        sprintf(filename, "frame%05d.ppm", frameNumber);
    
        fileHandle = fopen(filename, "wb");

        if (fileHandle == NULL)
        {
            printf("Unable to open %s...\n", filename);
            return -1;
        }
        fprintf(fileHandle, "P6\n%d %d\n%d\n", width, height, 255);
        fwrite(frame, 1, width*height*3, fileHandle);
        fclose(fileHandle);
        frameNumber++;

    return 1;
}

