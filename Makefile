CC=g++
INC_DIR = /opt/include
LIB_DIR = /opt/lib
CFLAGS= -O0 -g -w 
DEPS = eva.h imageops.h video.h 
OBJ = eva.o imageops.o  avi2eva.o video.o 
LDFLAGS = -lavcodec -lavutil -lavdevice -lavformat -lswscale -lswresample -lpthread -pthread -lz -lm -ldl
PROGRAMS = ff2

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -I$(INC_DIR) -L$(LIB_DIR)

vid2eva: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -I$(INC_DIR) -L$(LIB_DIR) $(LDFLAGS)
	
eva4to5: 
	$(CC) -o $@ $^ $(CFLAGS) eva4to5.c

evacv: 
	$(CC) -o $@ $^ $(CFLAGS) evacv.c

all: avi2eva eva4to5 evacv

clean:
	rm -f *.o *~ vid2eva evacv eva4to5

