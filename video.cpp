#include "video.h"
#include <math.h>




Video::Video(const char* path)
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
    vDecoderCtx = avcodec_alloc_context3(inVideoCodec); 

    ret = avcodec_parameters_to_context(vDecoderCtx, inVideoStrm->codecpar);

    if(ret < 0)
        std::cerr << "fail to tarnsfer oarameters ti context" << std::endl;


    ret = avcodec_open2(vDecoderCtx, inVideoCodec, nullptr);
    
    if (ret < 0) {
        std::cerr << "fail to avcodec_open2: ret=" << ret;
        return;
    }

    width = inVideoStrm->codecpar->width;
    height = inVideoStrm->codecpar->height;
    framerate = av_q2d(inVideoStrm->r_frame_rate);
    length = av_rescale_q(inVideoStrm->duration, inVideoStrm->time_base, av_make_q(1, 1000)) / 1000;
    total_frames = inVideoStrm->nb_frames;
    


    sampleRate = 15600;

        // Find the audio stream
    inAudioCodec = nullptr;
    ret = av_find_best_stream(inFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &inAudioCodec, 0);
    if (ret < 0)
    {
        std::cout << "Could not find any audio stream in the file" << std::endl;  
        return;      
    }
    astrm_idx = ret;
    inAudioStrm = inFormatCtx->streams[astrm_idx];


    // open audio decoder context    
    aDecoderCtx = avcodec_alloc_context3(inAudioCodec);
    avcodec_parameters_to_context(aDecoderCtx, inFormatCtx->streams[astrm_idx]->codecpar);
    ret = avcodec_open2(aDecoderCtx, inAudioCodec, nullptr);
    
    if (ret < 0) {
        std::cerr << "fail to avcodec_open2 (audio): ret=" << ret;
        return;
    }

    std::cout << "This stream has " << inAudioStrm->codecpar->channels << " channels and a sample rate of " << inAudioStrm->codecpar->sample_rate << "Hz" << std::endl;
    std::cout << "The data is in the format " << av_get_sample_fmt_name(aDecoderCtx->sample_fmt) << std::endl;


    
}

Video::~Video() {
    avcodec_close(vDecoderCtx);
    avcodec_close(aDecoderCtx);
    avformat_close_input(&inFormatCtx);

}
void* Video::getFrameAt(int64_t timestamp)
{
    AVFrame* dstFrame = av_frame_alloc();
    AVFrame* decframe = av_frame_alloc();
    AVPacket pkt;
    int ret = 0;
    


    av_seek_frame(inFormatCtx, -1,timestamp, 1);


    ret = av_read_frame(inFormatCtx, &pkt);

    if (ret < 0 && ret != AVERROR_EOF) {
        std::cerr << "fail to av_read_frame: ret=" << ret;        
    }
    while (ret == 0 && pkt.stream_index != vstrm_idx)
    {
        ret = av_read_frame(inFormatCtx, &pkt);
    }    
    
    ret = avcodec_send_packet(vDecoderCtx, &pkt);
    if(ret >= 0)
    {
        ret = avcodec_receive_frame(vDecoderCtx, decframe);
        if(ret < 0)
        {
            printf("Error, receive frame");
        return NULL;
        }
    }
    else
    {   
        printf("Error, send packet");
        return NULL;
    }

    sws_scale_frame(swsctx, dstFrame, decframe);

    av_packet_unref(&pkt);
    av_frame_free(&decframe);
    //av_frame_free(&dstFrame);

    return dstFrame->data[0];
    
}

void* Video::getAudioFrameAt(int64_t timestamp)
{
    AVFrame* dstFrame = av_frame_alloc();
    AVFrame* decframe = av_frame_alloc();
    AVPacket pkt;

    av_seek_frame(inFormatCtx, -1,timestamp, 1);
    av_read_frame(inFormatCtx, &pkt);

    avcodec_send_packet(aDecoderCtx, &pkt);
    avcodec_receive_frame(aDecoderCtx, decframe);


    //sws_scale_frame(swsctx, dstFrame, decframe);

    av_packet_unref(&pkt);
    av_frame_free(&decframe);
    //av_frame_free(&dstFrame);
        
    return decframe->data[0];
}

bool Video::decodeAllFrames() {

    
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

//                printf("wrong stream: %i (should be %i) ", pkt.stream_index, vstrm_idx );
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
        ret = avcodec_send_packet(vDecoderCtx, &pkt);
//        if(ret != 0)printf("Send packet ret-value: %i\n", ret);
        ret = avcodec_receive_frame(vDecoderCtx, decframe);
//        if(ret != 0) printf("Receive frame ret-value: %i\n", ret);

        if(ret < 0)
        {
            //printf("Error decoding frame: %i\n", nb_frames);
            goto next_packet;
        }
            


        // convert / scale frame 
        sws_scale_frame(swsctx, dstframe, decframe);        
        frames.push_back(dstframe->data[0]);   

//        std::cout << nb_frames << '\r' << std::flush;  // dump progress
        ++nb_frames;
    next_packet:
        av_packet_unref(&pkt);
    } while (!end_of_stream );

    av_frame_free(&decframe);
    av_frame_free(&dstframe);

//    std::cout << nb_frames << " frames decoded" << std::endl;

    
}



bool Video::decodeAudioFrames() {

    
    unsigned nb_frames = 0;
    bool end_of_stream = false;
    AVPacket pkt;    
    av_init_packet(&pkt);

    int ret = 0;
    
    AVFrame* dstframe;
    AVFrame*  decframe;

    av_seek_frame(inFormatCtx, astrm_idx, 0, 1);
    
    int err = 0;
    while ( false /*(err = av_read_frame(inFormatCtx, &pkt)) != AVERROR_EOF */) {
        if(err != 0) {
            // Something went wrong.
            printf("Read error.", err);
            break; // Don't return, so we can clean up nicely.
        }
        // Does the packet belong to the correct stream?
        if(pkt.stream_index != astrm_idx) {
            // Free the buffers used by the frame and reset all fields.
            av_packet_unref(&pkt);
            continue;
        }
        // We have a valid packet => send it to the decoder.
        if((err = avcodec_send_packet(aDecoderCtx, &pkt)) == 0) {
              
            // The packet was sent successfully. We don't need it anymore.
            // => Free the buffers used by the frame and reset all fields.
            av_packet_unref(&pkt);
        } else {
            // Something went wrong.
            // EAGAIN is technically no error here but if it occurs we would need to buffer
            // the packet and send it again after receiving more frames. Thus we handle it as an error here.
            printf("send: %s\n", av_err2str(err));
            break; // Don't return, so we can clean up nicely.
        }

        // Receive and handle frames.
        // EAGAIN means we need to send before receiving again. So thats not an error.
        /*
        if((err = receiveAndHandle(codecCtx, frame)) != AVERROR(EAGAIN)) {
            // Not EAGAIN => Something went wrong.
            printError("Receive error.", err);
            break; // Don't return, so we can clean up nicely.
        }
        */
    }

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

//printf("idx: %i\n", pkt.stream_index);
            if (ret == 0 && pkt.stream_index != astrm_idx)
            {

//                printf("wrong stream: %i (should be %i) ", pkt.stream_index, astrm_idx );
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

        // decode audio frame
        ret = avcodec_send_packet(aDecoderCtx, &pkt);
//        printf("send: %s\n", av_err2str(ret));
        ret = avcodec_receive_frame(aDecoderCtx, decframe);
//        printf("receive: %i\n", av_err2str(ret));
        if(ret < 0)
        {
            goto next_packet;
        }
            


        // convert / scale frame 
//        sws_scale_frame(swsctx, dstframe, decframe);        
        audio_frames.push_back(decframe->data[0]);    

        std::cout << nb_frames << '\r' << std::flush;  // dump progress
        ++nb_frames;
    next_packet:
        av_packet_unref(&pkt);
    } while ( !end_of_stream );

    av_frame_free(&decframe);
    av_frame_free(&dstframe);

//    std::cout << nb_frames << " frames decoded" << std::endl;

    
}
bool Video::decodeFrames(int fps) {

    int64_t step = (double)(1.0/fps) * AV_TIME_BASE;

    printf("step: %ld\n", step);

    for(uint64_t t = 0; t < length * AV_TIME_BASE; t+=step)
    {
//        printf("current stamp: %ld\n", t);
        frames.push_back((uint8_t*)getFrameAt(t));
    }

    return true;

}
bool Video::prepareScaler(int dst_width, int dst_height)
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
void Video::dumpInfo()
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

void Video::decodeAudio()
{
    AVPacket readingPacket;
    av_init_packet(&readingPacket);
    AVFrame* frame = av_frame_alloc();

    // Read the packets in a loop
    while (av_read_frame(inFormatCtx, &readingPacket) == 0)
    {
        if (readingPacket.stream_index == inAudioStrm->index)
        {
            AVPacket decodingPacket = readingPacket;

            // Audio packets can have multiple audio frames in a single packet
            while (decodingPacket.size > 0)
            {
                // Try to decode the packet into a frame
                // Some frames rely on multiple packets, so we have to make sure the frame is finished before
                // we can use it

                int result = avcodec_send_packet(aDecoderCtx, &decodingPacket);

                result = avcodec_receive_frame(aDecoderCtx, frame);
                

                //int gotFrame = 0;
                //int result = avcodec_decode_audio4(aDecoderCtx, frame, &gotFrame, &decodingPacket);

                if (result >= 0) // && gotFrame)
                {
                    decodingPacket.size -= result;
                    decodingPacket.data += result;

                    // We now have a fully decoded audio frame
                    dumpAudioInfo(aDecoderCtx, frame);
                }
                else
                {
                    decodingPacket.size = 0;
                    decodingPacket.data = nullptr;
                }
            }
        }

        // You *must* call av_free_packet() after each call to av_read_frame() or else you'll leak memory
        av_packet_unref(&readingPacket);
    }


    // Some codecs will cause frames to be buffered up in the decoding process. If the CODEC_CAP_DELAY flag
    // is set, there can be buffered up frames that need to be flushed, so we'll do that
    if (aDecoderCtx->codec->capabilities & AV_CODEC_CAP_DELAY)
    {
        av_init_packet(&readingPacket);
        // Decode all the remaining frames in the buffer, until the end is reached
        while(avcodec_send_packet(aDecoderCtx, &readingPacket) == 0)
        {
            avcodec_receive_frame(aDecoderCtx, frame);
            // We now have a fully decoded audio frame
            dumpAudioInfo(aDecoderCtx, frame);
        }
    }    

    
}
double Video::getLength(){ return length; }
double Video::getFramerate(){ return framerate; }
long Video::getFrameCount(){ return total_frames;}
int Video::getWidth(){ return width; }
int Video::getHeight(){ return height; }
std::vector<uint8_t*> Video::getFrames(){ return frames; }
std::vector<uint8_t*> Video::getAudioFrames(){ return audio_frames; }
int Video::getSampleRate(){ return sampleRate; }


void* Video::getAudio()
{
    
    int pw = (sampleRate / 2 / framerate);    
    int length = total_frames * pw;
    void* abuff = malloc(pw);
    //printf("l: %i \ntotal_frames: %i\naudio chunk: %i\n", length, total_frames, pw);
    memset(abuff, 0x00, pw);

    return abuff;
}

int Video::saveFrameToFile(uint8_t* frame, int width, int height)
{
        FILE* fileHandle;
        int y, writeError;
        char filename[32];
        static long frameNumber = 0;

        sprintf(filename, "frame_ppm_%05ld.ppm", frameNumber);
    
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

void Video::dumpAudioInfo(const AVCodecContext* codecContext, const AVFrame* frame)
{
    // See the following to know what data type (unsigned char, short, float, etc) to use to access the audio data:
    // http://ffmpeg.org/doxygen/trunk/samplefmt_8h.html#af9a51ca15301871723577c730b5865c5
    std::cout << "Audio frame info:\n"
              << "  Sample count: " << frame->nb_samples << '\n'
              << "  Channel count: " << codecContext->channels << '\n'
              << "  Format: " << av_get_sample_fmt_name(codecContext->sample_fmt) << '\n'
              << "  Bytes per sample: " << av_get_bytes_per_sample(codecContext->sample_fmt) << '\n'
              << "  Is planar? " << av_sample_fmt_is_planar(codecContext->sample_fmt) << '\n';


    //std::cout << "frame->linesize[0] tells you the size (in bytes) of each plane\n";

    return;
    if (codecContext->channels > AV_NUM_DATA_POINTERS && av_sample_fmt_is_planar(codecContext->sample_fmt))
    {
        std::cout << "The audio stream (and its frames) have too many channels to fit in\n"
                  << "frame->data. Therefore, to access the audio data, you need to use\n"
                  << "frame->extended_data to access the audio data. It's planar, so\n"
                  << "each channel is in a different element. That is:\n"
                  << "  frame->extended_data[0] has the data for channel 1\n"
                  << "  frame->extended_data[1] has the data for channel 2\n"
                  << "  etc.\n";
    }
    else
    {
        std::cout << "Either the audio data is not planar, or there is enough room in\n"
                  << "frame->data to store all the channels, so you can either use\n"
                  << "frame->data or frame->extended_data to access the audio data (they\n"
                  << "should just point to the same data).\n";
    }

    std::cout << "If the frame is planar, each channel is in a different element.\n"
              << "That is:\n"
              << "  frame->data[0]/frame->extended_data[0] has the data for channel 1\n"
              << "  frame->data[1]/frame->extended_data[1] has the data for channel 2\n"
              << "  etc.\n";

    std::cout << "If the frame is packed (not planar), then all the data is in\n"
              << "frame->data[0]/frame->extended_data[0] (kind of like how some\n"
              << "image formats have RGB pixels packed together, rather than storing\n"
              << " the red, green, and blue channels separately in different arrays.\n";
}