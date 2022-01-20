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

#ifndef FMP4_H
#define FMP4_H



#include "base/thread.h"
#include <string>
#include <vector>


#include "net/netInterface.h"
#include "http/HttpClient.h"
#include "http/HttpServer.h" 
#include "muxer.h"



#define FILEPARSER 1

namespace base {
namespace fmp4 {
    
class LiveThread;
class DummyFrameFilter;
class FragMP4MuxFrameFilter;
class InfoFrameFilter;
class TextFrameFilter;
class ReadMp4;  
class LiveConnectionContext;
class FFParse;

 class ReadMp4: public Thread, public net::HttpsServer 
 {
     
     
 public:
     ReadMp4( std::string ip, int port, net::ServerConnectionFactory *factory );
     
     ~ReadMp4( );
     
    // void websocketConnect();
     
      //void send(const char * data, int size, bool binary);
     
     int fmp4( const char *in_filename, const char *out_filename =nullptr, bool fragmented_mp4_options=true);
          
   //virtual void start() override
   // virtual void stop() override;
   
     void run() override;
     
     std::vector<uint8_t> outputData;
     bool looping{true};
     
     
 private:
     
     struct stParser
     {
        #if FILEPARSER
        FFParse  *ffparser;
      #else
        LiveThread  *ffparser;
      #endif
         
        DummyFrameFilter *fragmp4_filter{nullptr};
        FrameFilter *fragmp4_muxer{nullptr};;
        FrameFilter *info{nullptr};;
        FrameFilter *txt{nullptr};
        
        stParser(ReadMp4 *mp4this,  const char* audioFile, const char* videofile, int fpsType);
        ~stParser();
       
     };
     
     
     stParser *parser1;
     stParser *parser2;
    // stParser *parser2;

    // std::string fileName;
     
 public:
     //// 1 ftype, 2 moov , 3 first moof( idr frame), 4 P or B frames cane be dropped 
     void broadcast(const char * data, int size, bool binary,  int frametype   );
     void on_read(net::Listener* connection, const char* msg, size_t len) ;
     
     void on_close(net::Listener* connection);
     
     
//    virtual void onConnect(    int socketID                        );
//    virtual void onMessage(    int socketID, const string& data    );
//    virtual void onDisconnect( int socketID                        );
//    virtual void   onError(    int socketID, const string& message );
//     
     

 };
 
}
}

#endif /* FMP4_H */

