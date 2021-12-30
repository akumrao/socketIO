################################################################################
### CUSTOMIZE BELOW HERE #######################################################

FFMPEG_ROOT = /workspace/ffmpeg


CC	= gcc
CXX     = g++
TARGET=mediaserver # define the name of the executable
SOURCES    = $(wildcard *.cpp ../src/*.cpp ../ffmpeg/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp ../../base/src/*.cpp ../../json/src/*.cpp ../../crypto/src/*.cpp ../../net/src/*.cpp ../../http/src/*.cpp  ../../http_parser/*.cpp )

CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized -Wno-attributes -Wno-fpermissive -Wno-deprecated-declarations
 

CPPFLAGS   := -I../../rtsp/include/ -I../ffmpeg/ -I../ffmpeg/compat/atomics/gcc/ -I../../json/include/ -I../../crypto/include/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../net/uv_multiplex/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/  -I../../rtsp/ \
              -DHTTPSSL
#LDFLAGS   := -L/export/views/video/ffmpeg/Obj/

#/export/views/video/ffmpeg/libavutil/libavutil.a /export/views/video/ffmpeg/libavcodec/libavcodec.a
LDLIBS    := -lpthread -lm -ldl  -lssl -lcrypto -lz -lfdk-aac  

#SLIBS     := $(OUT_FILE_NAME) 

SLIBS     := $(OUT_FILE_NAME) /usr/local/lib/libx264.a



# define list of objects
OBJSC=$(SOURCES:.cpp=.o)
OBJS=$(OBJSC:.cpp=.o)

# the target is obtained linking all .o files
all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJS) $(SLIBS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)
	rm -f $(OUT_FILE_NAME)


clean:
	rm -f  $(OBJS)

