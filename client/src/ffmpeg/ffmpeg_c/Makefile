
CC = gcc
TARGET_FILE_NAME = decode_video  
FFMPEG_FOLDER = ffmpeg_codecs

CFLAGS = -g -Wall -c -fPIC
CFLAGS_TARGET := -g -Wno-inconsistent-missing-override -Wno-uninitialized -Wno-attributes -Wno-fpermissive -Wno-deprecated-declarations -fPIC 
INC = -I../$(FFMPEG_FOLDER) -I../$(FFMPEG_FOLDER)/decoder -I../$(FFMPEG_FOLDER)/decoder/x86

OUT_DIR=./

#//TARGETS=$(TARGET)
.DEFAULT_GOAL=all
.PHONY: all
#all: $(TARGETS)

#SOURCES = decode_video.c

# define list of objects
SOURCES = $(patsubst %.c,%.o,$(wildcard ../$(FFMPEG_FOLDER)/*.c ../$(FFMPEG_FOLDER)/decoder/*.c ../$(FFMPEG_FOLDER)/decoder/x86/*.c))
OBJS=$(SOURCES:.c=.o)
LDLIBS    := -lpthread -lm -ldl -lfdk-aac -no-pie

# Enumerating of every *.cpp as *.o and using that as dependency

all:$(OBJS) 
	$(CC) $(INC) $(CFLAGS_TARGET) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(TARGET_FILE_NAME) 
	#$(patsubst %.c,%.o,$(wildcard ../$(FFMPEG_FOLDER)/*.c ../$(FFMPEG_FOLDER)/decoder/*(!template).c)) -o $(TARGET_FILE_NAME) 
	#$(CC) $(INC) $(CFLAGS_TARGET) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(TARGET_FILE_NAME) 

#Compiling every *.c to *.o
%.o: %.c 
	$(CC) -c $(INC) $(CFLAGS) -o $@  $<
	
dirmake:
	@mkdir -p $(OUT_DIR)
	
clean:
	rm -f *.o
	rm -f *.d
	rm -f ./decoder/*.o
	rm -f ./decoder/*.d	
	rm -f ./decoder/x86/*.o
	rm -f ./decoder/x86/*.d	
	rm -f $(TARGET_FILE_NAME)

rebuild: clean build

purge: clean
	rm -f $(TARGET_FILE_NAME)