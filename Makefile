CC=g++
CFLAGS=-I.
DEPS = eva.h mono.h stretch.h
OBJ = eva.o mono.o stretch.o avi2eva.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

avi2eva: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
clean:
	rm -f *.o *~ avi2eva

