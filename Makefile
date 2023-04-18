CC=g++
INC_DIR = /opt/include
LIB_DIR = /opt/lib
CFLAGS= -O0 -g -w 
DEPS = eva.h mono.h stretch.h video.h
OBJ = eva.o mono.o stretch.o avi2eva.o video.o
LDFLAGS = -lavcodec -lavutil -lavdevice -lavformat -lswscale -lswresample -lpthread -pthread -lz -lm -ldl
PROGRAMS = ff2

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -I$(INC_DIR) -L$(LIB_DIR)

avi2eva: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -I$(INC_DIR) -L$(LIB_DIR) $(LDFLAGS)
	
clean:
	rm -f *.o *~ avi2eva

