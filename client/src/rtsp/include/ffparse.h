/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fmp4.h
 * Author: root
 *
 * Created on June 8, 2021, 10:48 PM
 */

#ifndef FFParse_H
#define FFParse_H

#include "base/thread.h"
#include <string>
#include <vector>
//#include "frame.h"
#include "framefilter.h"
// #include "net/netInterface.h"
// #include "http/HttpsClient.h"
#include "H264Framer.h"


extern "C" {
 
//#include <libavutil/timestamp.h>
// #include <avformat.h>
// #include <avcodec.h>
// #include <channel_layout.h>    
// #include <mathematics.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"

}

 


namespace base {
namespace fmp4 {
 #if 0 
   // a wrapper around a single output AVStream
typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    //AVFrame *tmp_frame;

    //float t, tincr, tincr2;

   // struct SwsContext *sws_ctx;
   // struct SwrContext *swr_ctx;
    
    FILE *in_file;
//    OutputStream()
//    {
//        in_file = nullptr;
//    }
} OutputStream;
#endif


class DummyFrameFilter;
class FragMP4MuxFrameFilter;
class InfoFrameFilter;
class TextFrameFilter;
class ReadMp4;   

class Common
{

public:
    Common(FrameFilter* fragmp4_muxer, FrameFilter* info, FrameFilter* txt) : fragmp4_muxer(fragmp4_muxer), info(info), txt(txt)
    {
    }
    FrameFilter* fragmp4_muxer;
    FrameFilter* info;
    FrameFilter* txt;
};

 class FFParse : public Common
 {

 public:
     class VideoParse;
     class AudioParse : public Common, public Thread
     {
     public:
         AudioParse(const char* audioFile, FrameFilter* fragmp4_muxer, FrameFilter* info, FrameFilter* txt);

         ~AudioParse();


        // DummyFrameFilter *fragmp4_filter;
    
         /// Audio Begin 

         FILE* fileAudio;
         bool parseAACHeader(int stream_index);
         void parseAACContent();

         bool initAAC();

         AVCodecContext* audioContext = NULL;
         const AVCodec* audiocodec = NULL;

         void reset();

         std::atomic< bool > resetParser{ false };

         void run() override;

         BasicFrame basicaudioframe;

         uint64_t startTime{ 0 };


         VideoParse* video{ nullptr };
     };


     class VideoParse : public Common, public Thread
     {
     public:
         VideoParse(const char* videofile, FrameFilter* fragmp4_muxer, FrameFilter* info, FrameFilter* txt);

         ~VideoParse();

         bool foundsps{ false };
         bool foundpps{ false };

         int fps{ 0 };
         int width{ 0 };
         int height{ 0 };

         int fpsType;

         H264Framer obj;

         /// Video Begin 
         bool parseH264Header(int stream_index);

         void parseH264Content();
         FILE* fileVideo;
        
         /// Video End
        // void reopen();

         long get_nal_size(uint8_t* buf, long size, uint8_t** poutbuf, int* poutbuf_size);

         void run() override;

         BasicFrame basicvideoframe;

         AudioParse* audio{ nullptr };
        

     };
    
     FFParse(  const char* audioFile, const char*  videofile, FrameFilter *fragmp4_muxer , FrameFilter *info , FrameFilter *txt  );
     
     ~FFParse( );

     void start();

        
    void reset( );

    
      
 private:
     AudioParse* audio{ nullptr };
     VideoParse* video{ nullptr };
  

    
    //std::string fileName;


    
 #if 0   
    /* Add an output stream. */
  void add_stream(OutputStream *ost, AVFormatContext *oc,
                       AVCodec **codec,
                       enum AVCodecID codec_id);


     AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples);
     void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg);
     AVFrame *get_audio_frame(OutputStream *ost);
     int write_audio_frame(AVFormatContext *oc, OutputStream *ost);
    void close_stream(AVFormatContext *oc, OutputStream *ost);
#endif
    

 };
 
}
}

#endif /* FFParse_H */

