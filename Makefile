C=g++
CFLAGS=-c -Wall
OBJS=main.o ini.o config.o mcu.o beacon.o util.o sensor.o

all: minigate

install: all
	/usr/bin/install -D --mode=755 minigate /usr/local/bin/minigate

minigate: $(OBJS)
	$(C) $(OBJS) -lwiringPi -lcrypt -lrt -pthread -o minigate

main.o: main.cpp
	$(C) $(CFLAGS) -o main.o main.cpp

config.o: config.cpp
	$(C) $(CFLAGS) -o config.o config.cpp

mcu.o: mcu.cpp
	$(C) $(CFLAGS) -o mcu.o mcu.cpp

beacon.o: beacon.cpp
	$(C) $(CFLAGS) -o beacon.o beacon.cpp

util.o: util.cpp
	$(C) $(CFLAGS) -o util.o util.cpp

sensor.o: sensor.cpp
	$(C) $(CFLAGS) -o sensor.o sensor.cpp

clean:
	rm -f minigate $(OBJS)
