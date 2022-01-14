/* This file is part of mediaserver. A RTSP live server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#include "fmp4.h"


#include "base/define.h"
#include "base/test.h"
#include <thread>
#include "ffparse.h"
//#include "livethread.h"
 #include "Settings.h"

extern "C"
{
//#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
//#include <libavcodec/avcodec.h>
}

#define SERVER_HOST  "127.0.0.1"               
#define SERVER_PORT 8000



//#define VIDEOFILE  "/experiment/fmp4/kunal720.264"


//#define MAX_CHUNK_SIZE 10240*8
// maximum send buffer 262144  =1024 *256

//#define highWaterMark  8 * 1048576
//maximum buffer = 16 *1048576 where  1024*1024 =1048576




//#define IOBUFSIZE 40960
//40960*6



#include "http/websocket.h"
//std::string rtsp  = "rtsp://10.170.4.89:8554/testStream";
//std::string rtsp  = "rtsp://192.168.0.19:8554/testStream";

namespace base {
    
    fmp4::ReadMp4 *self;

    namespace fmp4 {

        ReadMp4::stParser::stParser(ReadMp4 *mp4this,  const char* audioFile, const char* videofile, int fpsType)
        {
            fragmp4_filter = new DummyFrameFilter("fragmp4", mp4this);//  fragmp4_filter = new DummyFrameFilter("fragmp4", mp4this, fpsType);
            fragmp4_muxer = new FragMP4MuxFrameFilter("fragmp4muxer", fragmp4_filter);
            info = new InfoFrameFilter("info", nullptr);
             txt = new TextFrameFilter("txt", mp4this );// txt = new TextFrameFilter("txt", mp4this, fpsType );
            
            #if FILEPARSER
            //ffparser = new FFParse(audioFile, videofile,  fragmp4_muxer, info, txt, fpsType );
	    ffparser = new FFParse(audioFile, videofile,  fragmp4_muxer, info, txt );	
            ffparser->start();
            #else
            ffparser = new LiveThread("live");
            
            ffparser->start();
            
          
            
            ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp2, 2, fragmp4_muxer, info); // Request livethread to write into filter info
            ffparser->registerStreamCall(*ctx);

            ffparser->playStreamCall(*ctx);
            
            

           #endif

        }
        
        ReadMp4::stParser::~stParser()
        {
          // ffparser->stop();
           // ffparser->join();
            delete ffparser;
        }
        
        
        ReadMp4::ReadMp4( std::string ip, int port, net::ServerConnectionFactory *factory ): net::HttpsServer(  ip, port,  factory, true) {

            self = this;

	   parser1 = new stParser( this, AUDIOFILE, VIDEOFILE, 1);
           //parser2 = new stParser( this, AUDIOFILE1, VIDEOFILE1 , 2);
            

        }

        ReadMp4::~ReadMp4() {
            SInfo << "~ReadMp4( )";
            delete parser1;
           // delete parser2;
        }
        
        
         void ReadMp4::on_close(net::Listener* connection)
         {
             
            /* net::HttpsConnection* cn = (net::HttpsConnection*)connection;
                if(cn)
                {
                    net::WebSocketConnection *con = ((net::HttpsConnection*)cn)->getWebSocketCon();
                    if(con)
                    {
                      SInfo << "on_close " ;
                    }
                }
             
             */
             
         }


        void ReadMp4::on_read(net::Listener* connection, const char* msg, size_t len) {

            //connection->send("arvind", 6 );
   
            
             std::string got = std::string(msg, len);
             STrace << "on_read " << got;
                
              #if FILEPARSER

              if( got == "reset")
              parser1->ffparser->reset();  
            
              #else

               if( got == "reset")
                    fragmp4_muxer->resetParser = true ;//  
             
             
              #endif

                
//                else if( got == "mute")
//                    ffparser->restart(true);
//                else if( got == "unmute")
//                    ffparser->restart(false);   
//                else if( got  == "hd")
//		    ffparser->resHD(true);
//	        else if (got == "cif")
//		   ffparser->resHD(false);


        }
    
        void ReadMp4::broadcast(const char * data, int size, bool binary, int fametype  )
        {
           // conn->send( data, size, binary    );
           // static int noCon =0;
            
//            if(noCon !=this->GetNumConnections())
//                
//            {
//                noCon = this->GetNumConnections();
//                SInfo << "No of Connectons " << noCon;
//            }

            for (auto* connection :  this->GetConnections())
            {
                net::HttpsConnection* cn = (net::HttpsConnection*)connection;
                if(cn)
                {
                    net::WebSocketConnection *con = ((net::HttpsConnection*)cn)->getWebSocketCon();
                    if(con)// if( con && con->fps_type == fps_type)
                     con->push(data ,size, binary, fametype);
                }
            }

        

        }
        
        void ReadMp4::run() {
             
        }
 
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    }
}
