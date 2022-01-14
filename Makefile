CC=g++
CFLAGS=-o
TARGET=run
OBJS=main.cpp my_thread.cpp
LDLIBS=-lboost_system -lboost_thread -lpthread

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $@ $(OBJS) $(LDLIBS)

clean:
	rm $(TARGET)
