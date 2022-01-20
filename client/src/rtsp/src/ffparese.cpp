/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#include "ffparse.h"
#include <thread>

#include "tools.h"
#include "fmp4.h"


#define AUDIOFILE  "./hindi.pcm"               
#define VIDEOFILE  "./test.264"

//#define VIDEOFILE  "./cat5.264"

#define AUDIOFILE1  "./hindi.pcm"               
#define VIDEOFILE1  "./buny45fps.264"


#define AUDIOFILE2  "./hindi.pcm"               
#define VIDEOFILE2  "./goal.264"  


#define WAITTILLFIND 10000

//#define _DEBUG_1 1
//#define AUDIOSLEEP 21000
//#define AUDIOSLEEP 15300
//#define AUDIOSLEEP 23220

namespace base {
    namespace fmp4 {


        // based on https://ffmpeg.org/doxygen/trunk/remuxing_8c-example.html


        void FFParse::start() {


                          
            fragmp4_muxer->deActivate();
            //
            //// Audio only  
//            audio->parseAACHeader(0);
//            audio->start();
//                        
//            return;
            //          
                       // video only
             if(fpsType == 1)
             video->start();
            
             if(fpsType == 2)
             {
                 audio->parseAACHeader(0);
                audio->start();
             }
            
             return;

            //#define LIVE 1  // for different framerate
            // video and audio  with two separate threads
//            video->audio = audio;
//            audio->video = video;
//
//            video->start();
//            audio->start();

//            return;

            // video and audio  without thread
             both->start();
             return;
            //



            return;


        }
    

        FFParse::FFParse(  const char* audioFile, const char* videofile,  FrameFilter *fragmp4_muxer , FrameFilter *info , FrameFilter *txt, int fpsType ) : Common(fragmp4_muxer, info, txt, fpsType) {

            audio = new AudioParse(audioFile, fragmp4_muxer, info, txt, fpsType);

            video = new VideoParse(videofile, fragmp4_muxer, info, txt, fpsType);
            
            both = new BothParse(audioFile, videofile, fragmp4_muxer, info, txt, fpsType);
         
        }


        FFParse::~FFParse() {
            SInfo << "~FFParse( )";
            if(audio)
             delete audio;
           
            if(video)
             delete video;
            
            if(both)
              delete both;
        }

        
        

        FFParse::AudioParse::AudioParse(const char* audioFile, FrameFilter* fragmp4_muxer, FrameFilter* info, FrameFilter* txt, int fpsType ) : Common(fragmp4_muxer, info, txt, fpsType)
        {

            fileAudio = fopen(audioFile, "rb");
            if (fileAudio) {
                av_log(NULL, AV_LOG_INFO, "open file success \n");
            }
            else {
                SError << "can't open file! " << audioFile;
                return;
            }

            
            if (!initAAC())
            {
                SError << "can't open audio engine! ";
            }
            
            
          
       
           audiotimebase.num = 1;
           audiotimebase.den = SAMPLINGRATE;  /// 1000000*1024/ 44100 = 23219.954648526  // almost half of video when 25 frames per secconds
           
           
            audiosize = av_samples_get_buffer_size(NULL, audioContext->channels,audioContext->frame_size,audioContext->sample_fmt, 1);

        }



      
        FFParse::AudioParse::~AudioParse() {
            SInfo << "~AudioParse( )";
            this->join();

            avcodec_close(audioContext);
            avcodec_free_context(&audioContext);

            fclose(fileAudio);
           // fclose(fileVideo);
        }



        FFParse::VideoParse::VideoParse(const char* videofile, FrameFilter* fragmp4_muxer, FrameFilter* info, FrameFilter* txt, int fpsType ) : Common(fragmp4_muxer, info, txt, fpsType)
        {

            fileVideo = fopen(videofile, "rb");
            if (fileVideo) {
                av_log(NULL, AV_LOG_INFO, "open file success \n");
            }
            else {
                SError << "can't open file! " << videofile;
            }
            
           videotimebase.num = 1;
           videotimebase.den = 25;   /// 1000000/25 = 40000

            basicvideoframe.media_type = AVMEDIA_TYPE_VIDEO;
            basicvideoframe.codec_id = AV_CODEC_ID_H264;
            basicvideoframe.stream_index = 0;

        }

        FFParse::VideoParse::~VideoParse() {
            SInfo << "~VideoParse( )";

            this->join();
           // fclose(fileAudio);
            fclose(fileVideo);
        }
       
     
      
         
        bool FFParse::AudioParse::initAAC()
        {

          //  av_dict_set(&opt, "movflags", "empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof", 0);
            
            AVCodecContext *c = nullptr ;
            AVDictionary *opt = NULL;
            //codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
            audiocodec = avcodec_find_encoder_by_name("libfdk_aac"); //Specify the use of file encoding type
            if (!audiocodec) {
                fprintf(stderr, "Codec not found\n");
               return false ;
            }

            c = avcodec_alloc_context3(audiocodec);
            if (!c) {
                fprintf(stderr, "Could not allocate audio codec context\n");
                return  false;
            }

            /* put sample parameters */
            c->bit_rate = 64000;

            /* check that the encoder supports s16 pcm input */
            c->sample_fmt = AV_SAMPLE_FMT_S16;


            /* select other audio parameters supported by the encoder */

            c->sample_rate = SAMPLINGRATE;//  

            c->channel_layout = AV_CH_LAYOUT_STEREO;//select_channel_layout(codec);
            c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
            c->profile = FF_PROFILE_AAC_LOW;
            
            //if (oc->oformat->flags & AVFMT_GLOBALHEADER)
             c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            
               
            /* open it */
            if (avcodec_open2(c, audiocodec, &opt) < 0) {
                fprintf(stderr, "Could not open codec\n");
               return false;
            }
             
     
             audioContext = c;

             
             return true ;
        }
 

        
        bool FFParse::AudioParse::parseAACHeader(int stream_index) {
          
            SetupFrame        setupframe;  ///< This frame is used to send subsession information
          
            basicaudioframe.media_type           =AVMEDIA_TYPE_AUDIO;
            basicaudioframe.codec_id             =AV_CODEC_ID_AAC;
            basicaudioframe.stream_index     =stream_index;
            // prepare setup frame
            setupframe.sub_type             =SetupFrameType::stream_init;
            setupframe.media_type           =AVMEDIA_TYPE_AUDIO;
            setupframe.codec_id             =AV_CODEC_ID_AAC;   // what frame types are to be expected from this stream
            setupframe.stream_index     =stream_index;
            //setupframe.mstimestamp          = CurrentTime_milliseconds();
            // send setup frame
            
            //info->run(&setupframe);
            fragmp4_muxer->run(&setupframe);
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            
                          
          //  startTime=  setupframe.mstimestamp;
                
             int extrasize = audioContext->extradata_size;             
             basicaudioframe.payload.resize(extrasize);
             memcpy( basicaudioframe.payload.data(),  audioContext->extradata, extrasize) ;
             basicaudioframe.codec_id = audiocodec->id;
             //basicaudioframe.mstimestamp = startTime ;
             fragmp4_muxer->run(&basicaudioframe);
             basicaudioframe.payload.resize(basicaudioframe.payload.capacity());

             
             return true;
        }
        
        
        void FFParse::AudioParse::parseAACContent()
        {
            
            AVPacket audiopkt;
            
            int ret, got_output;
             
            AVFrame *frame;
             
                    /* frame containing input raw audio */
            frame = av_frame_alloc();
            if (!frame) {
                fprintf(stderr, "Could not allocate audio frame\n");
                return ;
            }

            frame->nb_samples     = audioContext->frame_size;
            frame->format         = audioContext->sample_fmt;
            frame->channel_layout = audioContext->channel_layout;

            /* allocate the data buffers */
            ret = av_frame_get_buffer(frame, 0);
            if (ret < 0) {
                fprintf(stderr, "Could not allocate audio data buffers\n");
               return ;
            }
    

            uint8_t* frame_audobuf = (uint8_t *)av_malloc(audiosize);

            
            uint64_t adelay = (uint64_t)(1000000*AUDIOSAMPLE)/(uint64_t)SAMPLINGRATE; 
            
            while(!stopped())
            {   
                uint64_t currentTime = CurrentTime_microseconds();

               if (fread(frame_audobuf, 1, audiosize, fileAudio) <= 0){
                    printf("Failed to read raw data! \n");
                    break;


                }else if(feof(fileAudio)){
                    
                     if (fseek(fileAudio, 0, SEEK_SET))
                    return;
                    continue;
                    
                }

               av_init_packet(&audiopkt);
                audiopkt.data = NULL; // packet data will be allocated by the encoder
                audiopkt.size = 0;


               ret = av_frame_make_writable(frame);
                if (ret < 0)
                    return ;

               frame->data[0] = frame_audobuf;  //PCM Data
                //pFrame->pts=i*100;
               // encode(encodeContext,pFrame,&pkt);
                //i++;

                //if (c->oformat->flags & AVFMT_GLOBALHEADER)
	       //c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


                ret = avcodec_encode_audio2(audioContext, &audiopkt, frame, &got_output);
                if (ret < 0) {
                    fprintf(stderr, "Error encoding audio frame\n");
                    return ;
                }
                if (got_output && (!video  || video && video->foundsps && video->foundpps))
                {
                  
                    basicaudioframe.copyFromAVPacket(&audiopkt);
                  
                 //   basicaudioframe.mstimestamp = startTime + framecount;
                    
                    if (resetParser && !video)
                    {
                        fragmp4_muxer->sendMeta();
                        resetParser = false;
                    }
                    
                   // video->mut_sync.lock();   
                    while ( video && (av_compare_ts(video->vframecount, video->videotimebase,  aframecount* AUDIOSAMPLE, audiotimebase) <= 0))
                    {
        
                       std::this_thread::sleep_for(std::chrono::microseconds(WAITTILLFIND ));
                    }
                    
                    
                    {
                       //video->mut_sync.unlock();   
                       //aframecount = aframecount + AUDIOSAMPLE ;

                       ++aframecount;
                       fragmp4_muxer->run(&basicaudioframe);
                       
                        basicaudioframe.payload.resize(basicaudioframe.payload.capacity());
                 
                       av_packet_unref(&audiopkt);
                       
                     //  SInfo << "Audio Frame " << aframecount;
                       uint64_t deltamicro = CurrentTime_microseconds() - currentTime;
                       std::this_thread::sleep_for(std::chrono::microseconds(adelay - deltamicro));
                    
                    }

                    
                }
            }
                    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
            
           // fclose(fout);
            
           // av_dict_free(&opt);
            av_free(frame_audobuf);
            av_frame_free(&frame);
            // if you open any thing at ffmpeg do not forget to close it
            //avcodec_close(audioContext);
           // avcodec_free_context(&audioContext);
        }
        
        
        long FFParse::VideoParse::get_nal_size(uint8_t *buf, long size,  uint8_t **poutbuf, int *poutbuf_size) {
            long pos = 3;
            while ((size - pos) > 3) {
                if (buf[pos] == 0 && buf[pos + 1] == 0 && buf[pos + 2] == 1)
                {
                    *poutbuf = buf;
                    *poutbuf_size  = pos;
                    return pos;
                }
                if (buf[pos] == 0 && buf[pos + 1] == 0 && buf[pos + 2] == 0 && buf[pos + 3] == 1)
                {
                    *poutbuf = buf;
                    *poutbuf_size  = pos;
                    return pos;
                }
                pos++;
            }
            
             *poutbuf_size  = 0;
            return size;
        }

        
        
        void FFParse::reset()
        {
            if(audio)
            audio->reset();
        }

        void FFParse::AudioParse::reset()
        {
            resetParser = true;
        }

        void FFParse::AudioParse::run()
        {
            parseAACContent();
        }

        
        void FFParse::VideoParse::reopen()
        {
            fragmp4_muxer->deActivate();

            
            fclose(fileVideo);
            fileVideo = nullptr;
            hd = (++hd)%2;
            if (hd)
            {
               
                SInfo << "Open HD";
                const char* videofile = VIDEOFILE;

                fileVideo = fopen(videofile,"rb");
                if(fileVideo){
                    av_log(NULL,AV_LOG_INFO,"open file success \n");
                }else{
                    SError << "can't open file! " <<  videofile;
                }
            }
            else
            {
              
          
                const char* videofile =VIDEOFILE1;
                SInfo << "Open SD";
 

                fileVideo = fopen(videofile,"rb");
                if(fileVideo){
                    av_log(NULL,AV_LOG_INFO,"open file success \n");
                }else{
                    av_log(NULL,AV_LOG_ERROR,"can't open file! \n");
                }
            }
            
        }

        void FFParse::VideoParse::run()
        {
            #if LIVE
            parseH264Content_sleep();
            #else
             parseH264Content();
            
            #endif    
        }

        bool FFParse::VideoParse::parseH264Header(int stream_index)
        {
            int ret = 0;
           // AVCodec *codec = NULL;
          //  AVCodecContext *cdc_ctx = NULL;
            fragmp4_muxer->deActivate();
            foundsps = false;
            foundpps = false;

            AVPacket *pkt = NULL;

            SetupFrame        setupframe;  ///< This frame is used to send subsession information
          

            basicvideoframe.media_type          =AVMEDIA_TYPE_VIDEO;
            basicvideoframe.codec_id            =AV_CODEC_ID_H264;
            basicvideoframe.stream_index     =stream_index;
            // prepare setup frame
            setupframe.sub_type             =SetupFrameType::stream_init;
            setupframe.media_type           =AVMEDIA_TYPE_VIDEO;
            setupframe.codec_id             =AV_CODEC_ID_H264;   // what frame types are to be expected from this stream
            setupframe.stream_index     =stream_index;
            setupframe.mstimestamp          = CurrentTime_milliseconds();
            // send setup frame
            
            //info->run(&setupframe);
            fragmp4_muxer->run(&setupframe);

  
           

            //cur_size = fread(in_buffer, 1, in_buffer_size, fileVideo);
            //cur_ptr = in_buffer;

    

        
           
 //           avcodec_close(cdc_ctx);
//            avcodec_free_context(&cdc_ctx);
            return  true;

        }
        
        
        
        
        
        
         
       
       
            
          void FFParse::VideoParse::parseH264Content_sleep() {
            
            AVPacket *videopkt = NULL;
             int ret = 0;
            if ((videopkt = av_packet_alloc()) == NULL) {
                fprintf(stderr, "av_packet_alloc failed.\n");
                //goto ret3;
                return;
            }


            if (fseek(fileVideo, 0, SEEK_END))
                return;
            long videofileSize = (long) ftell(fileVideo);
            if (videofileSize < 0)
                return;
            if (fseek(fileVideo, 0, SEEK_SET))
                return;

            //SInfo << "H264 file Size " << videofileSize;


            // av_init_packet(pkt);

            const int in_videobuffer_size = videofileSize;
            unsigned char *in_videobuffer = (unsigned char*) malloc(in_videobuffer_size + AV_INPUT_BUFFER_PADDING_SIZE);
            unsigned char *cur_videoptr = nullptr;
            int cur_videosize=0;

            int gop = 0;
            uint64_t delay = 1000000/15; // default
            
            timescale = 1000000/15;
            
            videotimebase.den = 15;
                    
            while (!stopped()) 
            {
                
                
//                 static uint64_t  previous=0;
//
//                if (previous)
//                {
//                    uint64_t delta = CurrentTime_microseconds() - previous;
//
//                    if (!timescale)
//                    {
//                        timescale = delta;
//                    }
//                    else
//                        timescale = (timescale + delta) / 2;
//
//
//                      // UE_LOG(PixelStreamer, Log, TEXT("pushVideoFrame fps= %lld"), 1000000 / vidoefps);
//
//                    //std::cout << "videofps "    <<    fps << std::endl;
//
//                }
//
//                previous = CurrentTime_microseconds();
        
            
                
                
                uint64_t currentTime =  CurrentTime_microseconds();
                if (cur_videosize > 0) 
                {

                    ret = get_nal_size(cur_videoptr, cur_videosize, &videopkt->data, &videopkt->size);
                    if (ret < 4) {
                        cur_videoptr += 1;
                        cur_videosize -= 1;
                        continue;
                    }


                    // avcodec_decode_video2

                    cur_videoptr += ret;
                    cur_videosize -= ret;

                    if (videopkt->size == 0)
                        continue;

                    //Some Info from AVCodecParserContext

                    //SInfo << "    PTS=" << pkt->pts << ", DTS=" << pkt->dts << ", Duration=" << pkt->duration << ", KeyFrame=" << ((pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0) << ", Corrupt=" << ((pkt->flags & AV_PKT_FLAG_CORRUPT) ? 1 : 0) << ", StreamIdx=" << pkt->stream_index << ", PktSize=" << pkt->size;
                    // BasicFrame        basicframe;
                    basicvideoframe.copyFromAVPacket(videopkt);
                    basicvideoframe.mstimestamp =  vframecount;
                    basicvideoframe.fillPars();

       

                    if ( foundsps && foundpps && ( basicvideoframe.h264_pars.frameType == H264SframeType::i &&  basicvideoframe.h264_pars.slice_type == H264SliceType::idr)) //AUD Delimiter
                    {
                        fragmp4_muxer->sendMeta();
                        
                       
                        
                    #if(_DEBUG_1)

                        SInfo << " Key " << basicvideoframe.payload.size();
                        if (gop)
                        {
                            SInfo << "Gop " << gop;
                        }
                        gop = 1;
                    #endif
                    }

                    if (basicvideoframe.h264_pars.slice_type == H264SliceType::sps ||  basicvideoframe.h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
                    {
                                       
                       unsigned num_units_in_tick, time_scale;


                        if(  basicvideoframe.h264_pars.slice_type == H264SliceType::sps  )
                        {    
                            obj.analyze_seq_parameter_set_data(videopkt->data + 4, videopkt->size - 4, num_units_in_tick, time_scale);
                            
                            int actualfps  = 1000000 / timescale;
                            obj.fps = actualfps;
                            if (fps != obj.fps ||   width != obj.width || height != obj.height)
                            {
                                
                                
                                parseH264Header(0);

                                if(audio)
                                {
                                   videotimebase.den = obj.fps;   /// 1000000/25 = 40000
                                   audio->parseAACHeader(1);
                                }

                               SError << "reset parser, with fps " << obj.fps << " width "  <<   obj.width << " height"  << obj.height;
                            }


                            if (!foundpps)
                            {
                                    fps = obj.fps;
                                    height = obj.height;
                                    width = obj.width;

                                    basicvideoframe.fps = obj.fps;
                                    basicvideoframe.height = obj.height;
                                    basicvideoframe.width = obj.width;

                                    // SInfo <<  " Got SPS fps "  << fps << " width "  << width  <<  " height " << height ;

                                    //info->run(&basicvideoframe);
                                    fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                                    basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                                    
                                    delay= 1000000/fps ;
                                    
                            }
                           
                           foundsps  = true;
                           
                        }

                        if( !foundpps &&  basicvideoframe.h264_pars.slice_type == H264SliceType::pps  )
                        {    
                           
                           // SInfo <<  " Got PPS fps ";
                            
                           //info->run(&basicvideoframe);
                            fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                            basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                            
                            foundpps  = true;

                           
                        }
                       
                        continue;

                    }
                    else if (!((basicvideoframe.h264_pars.slice_type == H264SliceType::idr) ||   (basicvideoframe.h264_pars.slice_type == H264SliceType::nonidr))) {
                    //info->run(&basicvideoframe);
                        basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
			 continue;
                    }
                    else if (foundsps && foundpps  )
                    {
#if(_DEBUG_1)
                        if (basicvideoframe.h264_pars.frameType == H264SframeType::p)
                        {
                            ++gop;
                            SInfo << " P frame " << basicvideoframe.payload.size();
                        }
                        else if (basicvideoframe.h264_pars.frameType == H264SframeType::b)
                        {
                            ++gop;
                            SInfo << " B frame " << basicvideoframe.payload.size();
                        }
                        else if (basicvideoframe.h264_pars.frameType == H264SframeType::i && basicvideoframe.h264_pars.slice_type != H264SliceType::idr)
                        {
                            SInfo << " I frame " << basicvideoframe.payload.size() << " gop " << gop;
                            gop = 1;
                            
                        }
                      
                        
#endif
                        //mut_sync.lock();      
                        //info->run(&basicvideoframe);
                     
                              // video->mut_sync.lock();   
                        while ( audio && (av_compare_ts(vframecount, videotimebase,  audio->aframecount*AUDIOSAMPLE, audio->audiotimebase) > 0))
                        {

                       
                           std::this_thread::sleep_for(std::chrono::microseconds(WAITTILLFIND));
                        }
                        
                        fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                        basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                        vframecount++;
                       // mut_sync.unlock();  
                        
                        uint64_t deltamicro =CurrentTime_microseconds() - currentTime;
                        std::this_thread::sleep_for(std::chrono::microseconds(delay- deltamicro));
                     
                       //Sleep(67);

                     }
		}
                else
                {
                    reopen();
                    
                    vframecount = 0;
                    if(audio)
                    {
                      audio->aframecount = 0;
                    }
                       
                    if (fseek(fileVideo, 0, SEEK_SET))
                    return;

                    cur_videosize = fread(in_videobuffer, 1, in_videobuffer_size, fileVideo);

                    STrace << "Read H264 filee " << cur_videosize;

                    if (cur_videosize == 0)
                        break;
                    cur_videoptr = in_videobuffer;
                    
                    continue;
                    
                }
            }

	    av_packet_free(&videopkt);	
            free(in_videobuffer);

       }
        
  
        
        void FFParse::VideoParse::parseH264Content() {
            
            AVPacket *videopkt = NULL;
             int ret = 0;
            if ((videopkt = av_packet_alloc()) == NULL) {
                fprintf(stderr, "av_packet_alloc failed.\n");
                //goto ret3;
                return;
            }


            if (fseek(fileVideo, 0, SEEK_END))
                return;
            long videofileSize = (long) ftell(fileVideo);
            if (videofileSize < 0)
                return;
            if (fseek(fileVideo, 0, SEEK_SET))
                return;

            //SInfo << "H264 file Size " << videofileSize;


            // av_init_packet(pkt);

            const int in_videobuffer_size = videofileSize;
            unsigned char *in_videobuffer = (unsigned char*) malloc(in_videobuffer_size + AV_INPUT_BUFFER_PADDING_SIZE);
            unsigned char *cur_videoptr = nullptr;
            int cur_videosize=0;

            int gop = 0;
            uint64_t delay = 1000000/25; // default
                    
            while (!stopped()) 
            {
                uint64_t currentTime =  CurrentTime_microseconds();
                if (cur_videosize > 0) 
                {

                    ret = get_nal_size(cur_videoptr, cur_videosize, &videopkt->data, &videopkt->size);
                    if (ret < 4) {
                        cur_videoptr += 1;
                        cur_videosize -= 1;
                        continue;
                    }


                    // avcodec_decode_video2

                    cur_videoptr += ret;
                    cur_videosize -= ret;

                    if (videopkt->size == 0)
                        continue;

                    //Some Info from AVCodecParserContext

                    //SInfo << "    PTS=" << pkt->pts << ", DTS=" << pkt->dts << ", Duration=" << pkt->duration << ", KeyFrame=" << ((pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0) << ", Corrupt=" << ((pkt->flags & AV_PKT_FLAG_CORRUPT) ? 1 : 0) << ", StreamIdx=" << pkt->stream_index << ", PktSize=" << pkt->size;
                    // BasicFrame        basicframe;
                    basicvideoframe.copyFromAVPacket(videopkt);
                    basicvideoframe.mstimestamp =  vframecount;
                    basicvideoframe.fillPars();

       

                    if ( foundsps && foundpps && ( basicvideoframe.h264_pars.frameType == H264SframeType::i &&  basicvideoframe.h264_pars.slice_type == H264SliceType::idr)) //AUD Delimiter
                    {
                        fragmp4_muxer->sendMeta();
                        
                       
                        
                    #if(_DEBUG_1)

                        SInfo << " Key " << basicvideoframe.payload.size();
                        if (gop)
                        {
                            SInfo << "Gop " << gop;
                        }
                        gop = 1;
                    #endif
                    }

                    if (basicvideoframe.h264_pars.slice_type == H264SliceType::sps ||  basicvideoframe.h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
                    {
                                       
                       unsigned num_units_in_tick, time_scale;


                        if(  basicvideoframe.h264_pars.slice_type == H264SliceType::sps  )
                        {    
                            obj.analyze_seq_parameter_set_data(videopkt->data + 4, videopkt->size - 4, num_units_in_tick, time_scale);

                            if (fps != obj.fps || width != obj.width || height != obj.height)
                            {
                                
                                parseH264Header(0);

                                if(audio)
                                {
                                   videotimebase.den = obj.fps;   /// 1000000/25 = 40000
                                   audio->parseAACHeader(1);
                                }

                               SError << "reset parser, with fps " << obj.fps << " width "  <<   obj.width << " height"  << obj.height;
                            }


                            if (!foundpps)
                            {
                                    fps = obj.fps;
                                    height = obj.height;
                                    width = obj.width;

                                    basicvideoframe.fps = obj.fps;
                                    basicvideoframe.height = obj.height;
                                    basicvideoframe.width = obj.width;

                                    // SInfo <<  " Got SPS fps "  << fps << " width "  << width  <<  " height " << height ;

                                    //info->run(&basicvideoframe);
                                    fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                                    basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                                    
                                    delay= 1000000/fps ;
                                    
                            }
                           
                           foundsps  = true;
                           
                        }

                        if( !foundpps &&  basicvideoframe.h264_pars.slice_type == H264SliceType::pps  )
                        {    
                           
                           // SInfo <<  " Got PPS fps ";
                            
                           //info->run(&basicvideoframe);
                            fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                            basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                            
                            foundpps  = true;

                           
                        }
                       
                        continue;

                    }
                    else if (!((basicvideoframe.h264_pars.slice_type == H264SliceType::idr) ||   (basicvideoframe.h264_pars.slice_type == H264SliceType::nonidr))) {
                    //info->run(&basicvideoframe);
                        basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
			 continue;
                    }
                    else if (foundsps && foundpps  )
                    {
#if(_DEBUG_1)
                        if (basicvideoframe.h264_pars.frameType == H264SframeType::p)
                        {
                            ++gop;
                            SInfo << " P frame " << basicvideoframe.payload.size();
                        }
                        else if (basicvideoframe.h264_pars.frameType == H264SframeType::b)
                        {
                            ++gop;
                            SInfo << " B frame " << basicvideoframe.payload.size();
                        }
                        else if (basicvideoframe.h264_pars.frameType == H264SframeType::i && basicvideoframe.h264_pars.slice_type != H264SliceType::idr)
                        {
                            SInfo << " I frame " << basicvideoframe.payload.size() << " gop " << gop;
                            gop = 1;
                            
                        }
                      
                        
#endif
                        //mut_sync.lock();      
                        //info->run(&basicvideoframe);
                     
                              // video->mut_sync.lock();   
                        while ( audio && (av_compare_ts(vframecount, videotimebase,  audio->aframecount*AUDIOSAMPLE, audio->audiotimebase) > 0))
                        {

                       
                           std::this_thread::sleep_for(std::chrono::microseconds(WAITTILLFIND));
                        }
                        
                        fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                        basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                        vframecount++;
                       // mut_sync.unlock();  
                        
                        uint64_t deltamicro =CurrentTime_microseconds() - currentTime;
                        std::this_thread::sleep_for(std::chrono::microseconds(delay- deltamicro));
                     
                       //Sleep(67);

                     }
		}
                else
                {
                    reopen();
                    
                    vframecount = 0;
                    if(audio)
                    {
                      audio->aframecount = 0;
                    }
                       
                    if (fseek(fileVideo, 0, SEEK_SET))
                    return;

                    cur_videosize = fread(in_videobuffer, 1, in_videobuffer_size, fileVideo);

                    STrace << "Read H264 filee " << cur_videosize;

                    if (cur_videosize == 0)
                        break;
                    cur_videoptr = in_videobuffer;
                    
                    continue;
                    
                }
            }

	    av_packet_free(&videopkt);	
            free(in_videobuffer);

       }
        
        
          
        FFParse::BothParse::BothParse(  const char* audioFile, const char* videofile,  FrameFilter *fragmp4_muxer , FrameFilter *info , FrameFilter *txt, int fpsType ) : AudioParse(audioFile, fragmp4_muxer, info, txt, fpsType), VideoParse(videofile, fragmp4_muxer, info, txt,fpsType)
        {
         
        }
        
   
        void FFParse::BothParse::run()
        {
            parseMuxContent();
        } 
       
        
        FFParse::BothParse::~BothParse() {
            SInfo << "~BothParse( )";
            VideoParse::join();
           // fclose(fileVideo);
            
        }
        
        
        void FFParse::BothParse::start()
        {
            VideoParse::start() ;
        }
         
        
       void FFParse::BothParse::parseMuxContent() 
       {
           
            AVPacket *videopkt = NULL;
           int ret = 0;
           if ((videopkt = av_packet_alloc()) == NULL) {
               fprintf(stderr, "av_packet_alloc failed.\n");
               //goto ret3;
               return;
           }


           if (fseek(fileVideo, 0, SEEK_END))
               return;
           long videofileSize = (long) ftell(fileVideo);
           if (videofileSize < 0)
               return;
           if (fseek(fileVideo, 0, SEEK_SET))
               return;

           SInfo << "H264 file Size " << videofileSize;


           // av_init_packet(pkt);

           const int in_videobuffer_size = videofileSize;
           unsigned char *in_videobuffer = (unsigned char*) malloc(in_videobuffer_size + AV_INPUT_BUFFER_PADDING_SIZE);
           unsigned char *cur_videoptr = nullptr;;
           int cur_videosize=0;

           
           
           ////////////////////////////////////
           AVPacket audiopkt;
          
           int got_output;
            
           AVFrame *frame;
            
                   /* frame containing input raw audio */
           frame = av_frame_alloc();
           if (!frame) {
               fprintf(stderr, "Could not allocate audio frame\n");
               return ;
           }

           frame->nb_samples     = audioContext->frame_size;
           frame->format         = audioContext->sample_fmt;
           frame->channel_layout = audioContext->channel_layout;

           /* allocate the data buffers */
           ret = av_frame_get_buffer(frame, 0);
           if (ret < 0) {
               fprintf(stderr, "Could not allocate audio data buffers\n");
              return ;
           }
   

           uint8_t* frame_audobuf = (uint8_t *)av_malloc(audiosize);
           ///////////////////////////////

          
          int nCount= 0;
          int gop = 0;
          
          uint64_t vdelay = 1000000/25; // default
          
          uint64_t adelay = (uint64_t)(1000000*AUDIOSAMPLE)/(uint64_t)SAMPLINGRATE; 
           
          uint64_t vtime =0;
          
          uint64_t atime = 0;
          
          
           while (!VideoParse::stopped() )
           {
               
                if ( av_compare_ts(vframecount, videotimebase,  aframecount*AUDIOSAMPLE, audiotimebase) <= 0)
                {
                    
                   if (cur_videosize > 0)
                   {

                       ret = get_nal_size(cur_videoptr, cur_videosize, &videopkt->data, &videopkt->size);
                       if (ret < 4) {
                           cur_videoptr += 1;
                           cur_videosize -= 1;
                           continue;
                       }


                       // avcodec_decode_video2

                       cur_videoptr += ret;
                       cur_videosize -= ret;

                       if (videopkt->size == 0)
                           continue;

                       //Some Info from AVCodecParserContext

                       //SInfo << "    PTS=" << pkt->pts << ", DTS=" << pkt->dts << ", Duration=" << pkt->duration << ", KeyFrame=" << ((pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0) << ", Corrupt=" << ((pkt->flags & AV_PKT_FLAG_CORRUPT) ? 1 : 0) << ", StreamIdx=" << pkt->stream_index << ", PktSize=" << pkt->size;
                       // BasicFrame        basicframe;
                       basicvideoframe.copyFromAVPacket(videopkt);
                       basicvideoframe.mstimestamp =   vframecount;
                       basicvideoframe.fillPars();

                       
                        if ( foundsps && foundpps && ( basicvideoframe.h264_pars.frameType == H264SframeType::i &&  basicvideoframe.h264_pars.slice_type == H264SliceType::idr)) //AUD Delimiter
                        {
                            VideoParse::fragmp4_muxer->sendMeta();



                        #if(_DEBUG_1)
                            SInfo << " Key " << basicvideoframe.payload.size();
                            if (gop)
                            {
                                SInfo << "Gop " << gop;
                            }
                            gop = 1;
                        #endif
                        }

                        if (basicvideoframe.h264_pars.slice_type == H264SliceType::sps ||  basicvideoframe.h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
                        {

                           unsigned num_units_in_tick, time_scale;


                            if(  basicvideoframe.h264_pars.slice_type == H264SliceType::sps  )
                            {    
                                obj.analyze_seq_parameter_set_data(videopkt->data + 4, videopkt->size - 4, num_units_in_tick, time_scale);

                                if (fps != obj.fps || width != obj.width || height != obj.height)
                                {
                                    videotimebase.den =  obj.fps;
                                    
                                    parseH264Header(0);
                                        
                                    parseAACHeader(1);
                                        
                                    SInfo << "reset parser, with fps " << obj.fps << " width "  <<   obj.width << " height"  << obj.height;
                                }

                                //uint8_t *p = videopkt->data +4;

                                if (!foundpps)
                                {
                                    fps = obj.fps;
                                    height = obj.height;
                                    width = obj.width;

                                    basicvideoframe.fps = obj.fps;
                                    basicvideoframe.height = obj.height;
                                    basicvideoframe.width = obj.width;

                                     SInfo <<  " Got SPS fps "  << fps << " width "  << width  <<  " height " << height ;

                                        //info->run(&basicvideoframe);
                                    VideoParse::fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                                    basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                                    
                                     vdelay= 1000000/fps ;
                                }

                               foundsps  = true;

                            }

                            if( !foundpps &&  basicvideoframe.h264_pars.slice_type == H264SliceType::pps  )
                            {    

                                // SInfo <<  " Got PPS fps ";
                                //info->run(&basicvideoframe);
                                VideoParse::fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                                basicvideoframe.payload.resize(basicvideoframe.payload.capacity());

                                foundpps  = true;

                            }
                           
                           continue;


                        }
                        else if (!((basicvideoframe.h264_pars.slice_type == H264SliceType::idr) ||   (basicvideoframe.h264_pars.slice_type == H264SliceType::nonidr))) {
                        //info->run(&basicvideoframe);
                            //basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                             continue;
                        }
                        else if (foundsps && foundpps  )
                        {
    #if(_DEBUG_1)
                            if (basicvideoframe.h264_pars.frameType == H264SframeType::p)
                            {
                                ++gop;
                                SInfo << " P frame " << basicvideoframe.payload.size();
                            }
                            else if (basicvideoframe.h264_pars.frameType == H264SframeType::b)
                            {
                                ++gop;
                                SInfo << " B frame " << basicvideoframe.payload.size();
                            }
                            else if (basicvideoframe.h264_pars.frameType == H264SframeType::i && basicvideoframe.h264_pars.slice_type != H264SliceType::idr)
                            {
                                SInfo << " I frame " << basicvideoframe.payload.size() << " gop " << gop;
                                gop = 1;

                            }

    #endif
                            if(vtime > 0)
                            {
                               uint64_t vdelta =  CurrentTime_microseconds() -vtime;
                             
                               if( vdelay > vdelta )
                               std::this_thread::sleep_for(std::chrono::microseconds(vdelay - vdelta));
                               
                            }
                            vtime =  CurrentTime_microseconds();


                            //info->run(&basicvideoframe);
                            VideoParse::fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                            basicvideoframe.payload.resize(basicvideoframe.payload.capacity());

                            vframecount++;

                         }
                   
                   }
                   else 
                   {
                       vframecount = 0;
                       aframecount = 0;
                       reopen();
                       if (fseek(fileVideo, 0, SEEK_SET))
                       return;

                       cur_videosize = fread(in_videobuffer, 1, in_videobuffer_size, fileVideo);

                       STrace << "Read H264 filee " << cur_videosize;

                       if (cur_videosize == 0)
                           break;
                       cur_videoptr = in_videobuffer;
                       continue;
                   }
               }

               else if (foundsps && foundpps  )       //if (tdelta  >= a)//audio 
               {
                    if (fread(frame_audobuf, 1, audiosize, fileAudio) <= 0){
                   printf("Failed to read raw data! \n");
                   break;


                   }else if(feof(fileAudio)){

                        if (fseek(fileAudio, 0, SEEK_SET))
                       return;
                       continue;

                   }

                   av_init_packet(&audiopkt);
                   audiopkt.data = NULL; // packet data will be allocated by the encoder
                   audiopkt.size = 0;


                   ret = av_frame_make_writable(frame);
                    if (ret < 0)
                        return ;

                   frame->data[0] = frame_audobuf;  //PCM Data
                    //pFrame->pts=i*100;
                   // encode(encodeContext,pFrame,&pkt);
                    //i++;

                    //if (c->oformat->flags & AVFMT_GLOBALHEADER)
                   //c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


                   ret = avcodec_encode_audio2(audioContext, &audiopkt, frame, &got_output);
                   if (ret < 0) {
                       fprintf(stderr, "Error encoding audio frame\n");
                       return ;
                   }
                   if (got_output && basicaudioframe.stream_index) 
                   {

                       basicaudioframe.copyFromAVPacket(&audiopkt);
                       basicaudioframe.mstimestamp =  aframecount;

//                     if( resetParser ) 
//                     {
//                              fragmp4_muxer->sendMeta();
//                              resetParser =false;
//                     }
                       
                       if(atime > 0)
                       {
                           uint64_t adelta =  CurrentTime_microseconds() - atime;

                           if( adelay > adelta )
                           std::this_thread::sleep_for(std::chrono::microseconds(adelay - adelta));

                       }
                       atime =  CurrentTime_microseconds();

                            
                       ++aframecount;
                       VideoParse::fragmp4_muxer->run(&basicaudioframe);

                       basicaudioframe.payload.resize(basicaudioframe.payload.capacity());

                       av_packet_unref(&audiopkt);

                       //std::this_thread::sleep_for(std::chrono::microseconds(21000));

                   }
                   else
                   {
                       continue;
                   }
               }//audio 

             //  uint64_t deltaTimeMillis =CurrentTime_microseconds() - currentTime;
              // std::this_thread::sleep_for(std::chrono::microseconds(14500 - deltaTimeMillis));
           } //end while

           free(in_videobuffer);
           
           av_free(frame_audobuf);
           av_packet_free(&videopkt);

           av_frame_free(&frame);
           // if you open any thing at ffmpeg do not forget to close it
           avcodec_close(audioContext);
           avcodec_free_context(&audioContext);

         
       }
       
      #if 0
       
        void FFParse::VideoParse::parseH264Content_live() {
            
            AVPacket *videopkt = NULL;
             int ret = 0;
            if ((videopkt = av_packet_alloc()) == NULL) {
                fprintf(stderr, "av_packet_alloc failed.\n");
                //goto ret3;
                return;
            }


            if (fseek(fileVideo, 0, SEEK_END))
                return;
            long videofileSize = (long) ftell(fileVideo);
            if (videofileSize < 0)
                return;
            if (fseek(fileVideo, 0, SEEK_SET))
                return;

            const int in_videobuffer_size = videofileSize;
            unsigned char *in_videobuffer = (unsigned char*) malloc(in_videobuffer_size + AV_INPUT_BUFFER_PADDING_SIZE);
            unsigned char *cur_videoptr = nullptr;
            int cur_videosize=0;

          
                    
            while (!stopped()) 
            {
                uint64_t currentTime =  CurrentTime_microseconds();
                if (cur_videosize > 0) 
                {

                    ret = get_nal_size(cur_videoptr, cur_videosize, &videopkt->data, &videopkt->size);
                    if (ret < 4) {
                        cur_videoptr += 1;
                        cur_videosize -= 1;
                        continue;
                    }


                    cur_videoptr += ret;
                    cur_videosize -= ret;

                    if (videopkt->size == 0)
                        continue;
                    
                    
              
                    both->pushVideoFrame(  videopkt->data, videopkt->size);
                    
                    std::this_thread::sleep_for(std::chrono::microseconds( 1000000/25 ));
                    
        

                    //Some Info from AVCodecParserContext

                    //SInfo << "    PTS=" << pkt->pts << ", DTS=" << pkt->dts << ", Duration=" << pkt->duration << ", KeyFrame=" << ((pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0) << ", Corrupt=" << ((pkt->flags & AV_PKT_FLAG_CORRUPT) ? 1 : 0) << ", StreamIdx=" << pkt->stream_index << ", PktSize=" << pkt->size;
                    // BasicFrame        basicframe;
                   // basicvideoframe.copyFromAVPacket(videopkt);
                   // basicvideoframe.mstimestamp =  vframecount;
                   // basicvideoframe.fillPars();

		}
                else
                {
                   // reopen();
                    if (fseek(fileVideo, 0, SEEK_SET))
                    return;

                    cur_videosize = fread(in_videobuffer, 1, in_videobuffer_size, fileVideo);

                    STrace << "Read H264 filee " << cur_videosize;

                    if (cur_videosize == 0)
                        break;
                    cur_videoptr = in_videobuffer;
                    
                    continue;
                    
                }
            }

	    av_packet_free(&videopkt);	
            free(in_videobuffer);

       }
       
       void FFParse::AudioParse::parseAACContent_live()
        {
            
            AVPacket audiopkt;
         
     
            uint8_t* frame_audobuf = (uint8_t *)av_malloc(audiosize);

            uint64_t adelay = (uint64_t)(1000000*AUDIOSAMPLE)/(uint64_t)SAMPLINGRATE; 
            while(!stopped())
            {   
                uint64_t currentTime = CurrentTime_microseconds();

               if (fread(frame_audobuf, 1, audiosize, fileAudio) <= 0){
                    printf("Failed to read raw data! \n");
                    break;

                }else if(feof(fileAudio)){
                    
                     if (fseek(fileAudio, 0, SEEK_SET))
                    return;
                    continue;
                    
                }
                
                both->pushAudioFrame( frame_audobuf,  audiosize);
                    
                std::this_thread::sleep_for(std::chrono::microseconds(adelay));

                //av_init_packet(&audiopkt);
               // audiopkt.data = NULL; // packet data will be allocated by the encoder
               // audiopkt.size = 0;


            }
                    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
            
           // fclose(fout);
            
           // av_dict_free(&opt);
            av_free(frame_audobuf);
           /// av_frame_free(&frame);
            // if you open any thing at ffmpeg do not forget to close it
            //avcodec_close(audioContext);
           // avcodec_free_context(&audioContext);
        }
        
         
        
       void  FFParse::BothParse::parseMuxContent_live() 
        {
             
                 
           AVPacket *videopkt = NULL;
           int ret = 0;
           if ((videopkt = av_packet_alloc()) == NULL) {
               fprintf(stderr, "av_packet_alloc failed.\n");
               //goto ret3;
               return;
           }


           unsigned char *cur_videoptr = nullptr;;
           int cur_videosize=0;

           
    
           AVPacket audiopkt;
          
           int got_output;
            
           AVFrame *frame;
            
                   /* frame containing input raw audio */
           frame = av_frame_alloc();
           if (!frame) {
               fprintf(stderr, "Could not allocate audio frame\n");
               return ;
           }

           frame->nb_samples     = audioContext->frame_size;
           frame->format         = audioContext->sample_fmt;
           frame->channel_layout = audioContext->channel_layout;

           /* allocate the data buffers */
           ret = av_frame_get_buffer(frame, 0);
           if (ret < 0) {
               fprintf(stderr, "Could not allocate audio data buffers\n");
              return ;
           }
   
          
           uint8_t* frame_audobuf = (uint8_t *)av_malloc(audiosize);
           ///////////////////////////////

          
          int nCount= 0;
          int gop = 0;
          
          uint64_t vdelay = 1000000/25; // default
          
          uint64_t adelay = (uint64_t)(1000000*AUDIOSAMPLE)/(uint64_t)SAMPLINGRATE; 
           
          uint64_t vtime =0;
          
          uint64_t atime = 0;
          
          bool haveframe = false;
          
           std::vector<uint8_t> vtmp;
          
          while (!VideoParse::stopped() )
          {
               
                if ( av_compare_ts(vframecount, videotimebase,  aframecount*AUDIOSAMPLE, audiotimebase) <= 0)
                {
                  
                   cur_videosize = 0;
                    mtxVideo.lock();
                    if (videolist.size() )
                    {
                        std::list<std::vector<uint8_t>>::iterator vItr = videolist.begin();
                        vtmp = *vItr;
                        videolist.erase(vItr);
                        cur_videoptr = vtmp.data();
                        cur_videosize = vtmp.size();
                       
                    }
                    mtxVideo.unlock();

                    
                   if (cur_videosize > 0)
                   {

//                       ret = get_nal_size(cur_videoptr, cur_videosize, &videopkt->data, &videopkt->size);
//                       if (ret < 4) {
//                           cur_videoptr += 1;
//                           cur_videosize -= 1;
//                           continue;
//                       }
//
//
//                       // avcodec_decode_video2
//
//                       cur_videoptr += ret;
//                       cur_videosize -= ret;
//
//                       if (videopkt->size == 0)
//                           continue;
                       videopkt->data = cur_videoptr;
                       videopkt->size =cur_videosize;

                       //Some Info from AVCodecParserContext

                       //SInfo << "    PTS=" << pkt->pts << ", DTS=" << pkt->dts << ", Duration=" << pkt->duration << ", KeyFrame=" << ((pkt->flags & AV_PKT_FLAG_KEY) ? 1 : 0) << ", Corrupt=" << ((pkt->flags & AV_PKT_FLAG_CORRUPT) ? 1 : 0) << ", StreamIdx=" << pkt->stream_index << ", PktSize=" << pkt->size;
                       // BasicFrame        basicframe;
                       basicvideoframe.copyFromAVPacket(videopkt);
                       basicvideoframe.mstimestamp =   vframecount;
                       basicvideoframe.fillPars();

                       
                       if ( foundsps && foundpps && ( basicvideoframe.h264_pars.frameType == H264SframeType::i &&  basicvideoframe.h264_pars.slice_type == H264SliceType::idr)) //AUD Delimiter
                       {
                            VideoParse::fragmp4_muxer->sendMeta();



                        #if(_DEBUG_1)
                            SInfo << " Key " << basicvideoframe.payload.size();
                            if (gop)
                            {
                                SInfo << "Gop " << gop;
                            }
                            gop = 1;
                        #endif
                        }

                        if (basicvideoframe.h264_pars.slice_type == H264SliceType::sps ||  basicvideoframe.h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
                        {

                           unsigned num_units_in_tick, time_scale;


                            if(  basicvideoframe.h264_pars.slice_type == H264SliceType::sps  )
                            {    
                                obj.analyze_seq_parameter_set_data(videopkt->data + 4, videopkt->size - 4, num_units_in_tick, time_scale);

                                if (fps != obj.fps || width != obj.width || height != obj.height)
                                {
                                    videotimebase.den =  obj.fps;
                                    
                                    parseH264Header(0);
                                        
                                    parseAACHeader(1);
                                        
                                    SInfo << "reset parser, with fps " << obj.fps << " width "  <<   obj.width << " height"  << obj.height;
                                }

                                //uint8_t *p = videopkt->data +4;

                                if (!foundpps)
                                {
                                    fps = obj.fps;
                                    height = obj.height;
                                    width = obj.width;

                                    basicvideoframe.fps = obj.fps;
                                    basicvideoframe.height = obj.height;
                                    basicvideoframe.width = obj.width;

                                     SInfo <<  " Got SPS fps "  << fps << " width "  << width  <<  " height " << height ;

                                    //VideoParse::info->run(&basicvideoframe);
                                    VideoParse::fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                                    basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                                    
                                     vdelay= 1000000/fps ;
                                }

                               foundsps  = true;

                            }

                            if( !foundpps &&  basicvideoframe.h264_pars.slice_type == H264SliceType::pps  )
                            {    

                                // SInfo <<  " Got PPS fps ";
                                //VideoParse::info->run(&basicvideoframe);
                                VideoParse::fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                                basicvideoframe.payload.resize(basicvideoframe.payload.capacity());

                                foundpps  = true;

                            }
                           
                           continue;


                        }
                        else if (!((basicvideoframe.h264_pars.slice_type == H264SliceType::idr) ||   (basicvideoframe.h264_pars.slice_type == H264SliceType::nonidr))) {
                        //info->run(&basicvideoframe);
                            //basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                             continue;
                        }
                        else if (foundsps && foundpps  )
                        {
    #if(_DEBUG_1)
                            if (basicvideoframe.h264_pars.frameType == H264SframeType::p)
                            {
                                ++gop;
                                SInfo << " P frame " << basicvideoframe.payload.size();
                            }
                            else if (basicvideoframe.h264_pars.frameType == H264SframeType::b)
                            {
                                ++gop;
                                SInfo << " B frame " << basicvideoframe.payload.size();
                            }
                            else if (basicvideoframe.h264_pars.frameType == H264SframeType::i && basicvideoframe.h264_pars.slice_type != H264SliceType::idr)
                            {
                                SInfo << " I frame " << basicvideoframe.payload.size() << " gop " << gop;
                                gop = 1;

                            }

    #endif
//                            if(vtime > 0)
//                            {
//                               uint64_t vdelta =  CurrentTime_microseconds() -vtime;
//                             
//                               if( vdelay > vdelta )
//                               std::this_thread::sleep_for(std::chrono::microseconds(vdelay - vdelta));
//                               
//                            }
//                            vtime =  CurrentTime_microseconds();


                           // VideoParse::info->run(&basicvideoframe);
                            VideoParse::fragmp4_muxer->run(&basicvideoframe); // starts the frame filter chain
                            basicvideoframe.payload.resize(basicvideoframe.payload.capacity());
                            
                             SInfo << "video  " << vframecount ;   

                            vframecount++;
                            
                            

                         }
                   
                   }
                   else 
                   {
                      // vframecount = 0;
                      // aframecount = 0;
                       SInfo <<  " Sleep Video 14000";     
		       std::this_thread::sleep_for(std::chrono::microseconds(14000));
                       continue;
                   }
               }

               else if (foundsps && foundpps  )       //if (tdelta  >= a)//audio 
               {
                    haveframe = false;
                     
                    mtxSnd.lock();
                    if (RecordingBuffer.size() >=  audiosize)
                    {
                        memcpy(frame_audobuf, &RecordingBuffer[0], audiosize);
                        RecordingBuffer.erase(RecordingBuffer.begin(), RecordingBuffer.begin() + audiosize);
                        //RecordingBuffer.RemoveAt(0, audiosize, true);
                       // UE_LOG(PixelStreamer, Log, TEXT(" samples %d, avgofSoundframe %d "),  RecordingBuffer.size(), avgofSoundframe);
                        haveframe = true;
                    }

                    mtxSnd.unlock();
                    
                   
                    if(!haveframe)
                    {
                        SInfo <<  " Sleep Audio 14000";  
                        std::this_thread::sleep_for(std::chrono::microseconds(14000));
                        continue;    
                    
                    }
                   
                   
                   av_init_packet(&audiopkt);
                   audiopkt.data = NULL; // packet data will be allocated by the encoder
                   audiopkt.size = 0;


                   ret = av_frame_make_writable(frame);
                    if (ret < 0)
                        return ;

                   frame->data[0] = frame_audobuf;  //PCM Data
                    //pFrame->pts=i*100;
                   // encode(encodeContext,pFrame,&pkt);
                    //i++;

                    //if (c->oformat->flags & AVFMT_GLOBALHEADER)
                   //c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


                   ret = avcodec_encode_audio2(audioContext, &audiopkt, frame, &got_output);
                   if (ret < 0) {
                       fprintf(stderr, "Error encoding audio frame\n");
                       return ;
                   }
                   if (got_output && basicaudioframe.stream_index) 
                   {

                       basicaudioframe.copyFromAVPacket(&audiopkt);
                       basicaudioframe.mstimestamp =  aframecount;

//                     if( resetParser ) 
//                     {
//                              fragmp4_muxer->sendMeta();
//                              resetParser =false;
//                     }
                       
                       if(atime > 0)
                       {
                           uint64_t adelta =  CurrentTime_microseconds() - atime;

                           if( adelay > adelta )
                           std::this_thread::sleep_for(std::chrono::microseconds(adelay - adelta));

                       }
                       atime =  CurrentTime_microseconds();

                       SInfo << "audio " << aframecount ;   
                       ++aframecount;
                       VideoParse::fragmp4_muxer->run(&basicaudioframe);

                       basicaudioframe.payload.resize(basicaudioframe.payload.capacity());

                       av_packet_unref(&audiopkt);

                       //std::this_thread::sleep_for(std::chrono::microseconds(21000));

                   }
                   else
                   {
                       
                        SError << "audio not possile state " << aframecount ;   
                       continue;
                   }
               }//audio 

             //  uint64_t deltaTimeMillis =CurrentTime_microseconds() - currentTime;
              // std::this_thread::sleep_for(std::chrono::microseconds(14500 - deltaTimeMillis));
           } //end while

           
           av_free(frame_audobuf);
           av_packet_free(&videopkt);

           av_frame_free(&frame);
           // if you open any thing at ffmpeg do not forget to close it
           avcodec_close(audioContext);
           avcodec_free_context(&audioContext);

         
            
            
            
            
            
            
        }
        
      

       
        void FFParse::BothParse::pushAudioFrame( const unsigned char *buff, int size)
        {

       
              mtxSnd.lock();
             //   uint8* tmp = (uint8* )PCM16.GetData();
                RecordingBuffer.insert(RecordingBuffer.end(), buff, buff + size);
//                if (RecordingBuffer.size() >= audiosize)
//                   cv.notify_all();
              mtxSnd.unlock();
              
            if (RecordingBuffer.size() >= 7*audiosize)
             {
                // UE_LOG(PixelStreamer, Error, TEXT("Audio Frame buffer very high %d, avgofSoundframe %d" ), RecordingBuffer.size() , avgofSoundframe );
                  SInfo << "Autio Frame buffer very high "  <<  RecordingBuffer.size()  ;
             }
                
        }
       
        void FFParse::BothParse::pushVideoFrame( const unsigned char *buff, int size)
        {

            
            static uint64_t  previous=0;
             

    
            if (previous)
            {
                uint64_t delta = CurrentTime_microseconds() - previous;

                if (!timescale)
                {
                    timescale = delta;
                }
                else
                    timescale = (timescale + delta) / 2;

     
                int fps  = 1000000 / timescale;

                  // UE_LOG(PixelStreamer, Log, TEXT("pushVideoFrame fps= %lld"), 1000000 / vidoefps);
                   
                //std::cout << "videofps "    <<    fps << std::endl;

            }

            previous = CurrentTime_microseconds();
               
       


            //if (total > (40000 + 900) || total < 40000 - 900)
            //{
                //UE_LOG(PixelStreamer, Error, TEXT("Frame rate should be around 1/40000(=25 fps), right now it is %d"),  total);
                //return;
            ///}



            if (videolist.size() > 7)
            {
                //UE_LOG(PixelStreamer, Error, TEXT("Frame buffer very high %d, frame rate is around %d"), videolist.size() , avgofVideorame);
                 SInfo << "Video Frame buffer very high  " << videolist.size() << " timescale  " <<  timescale  << " fps " << fps;
            }

            std::vector<uint8_t>vbuffer;

            //std::unique_lock<std::mutex> lock(mtxEvent);
            mtxVideo.lock();
            vbuffer.insert(vbuffer.end(), buff, buff + size);
            videolist.push_back(vbuffer);
             // cv.notify_all();
            mtxVideo.unlock();

          
        }

       #endif
        

      }// ns mp4

}// nsbase


                    /*Only input video data*/
//                    if ((ret = av_parser_parse2(parser, cdc_ctx, &pkt->data, &pkt->size,
//                            cur_ptr, cur_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0)) < 0) {
//                        fprintf(stderr, "av_parser_parse2 failed.\n");
//                        //goto ret8;
//                        return;
//                    }

//                    printf("[Packet]Size:%6d\t", pkt->size);
//                    switch (parser->pict_type) {
//                        case AV_PICTURE_TYPE_I: printf("Type:I\t");
//                            break;
//                        case AV_PICTURE_TYPE_P: printf("Type:P\t");
//                            break;
//                        case AV_PICTURE_TYPE_B: printf("Type:B\t");
//                            break;
//                        default: printf("Type:Other\t");
//                            break;
//                    };
//                    printf("Number:%4d\n", parser->output_picture_number);

//                    ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
//                    if (ret < 0) {
//                        printf("Decode Error.\n");
//                        return ret;
//                    }
//                    if (got_picture) {
//                    }
