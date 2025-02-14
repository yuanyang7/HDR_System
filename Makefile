CC = g++
CFLAGS = -g -Wall
LDFLAGS = `pkg-config --cflags --libs opencv`
OBJECTS = hdr.o
LINK_TARGET = hdr

all: $(LINK_TARGET)
	./$(LINK_TARGET)

hdr.o : hdr.cpp
	$(CC) -c $(CFLAGS) hdr.cpp

$(LINK_TARGET) : $(OBJECTS)
	$(CC) -o $(LINK_TARGET) $(LDFLAGS) $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(LINK_TARGET)
	echo CLEAN DONE.
