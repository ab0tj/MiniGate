CXX=g++
CXXFLAGS=-c -Wall
OBJS=main.o ini.o config.o mcu.o beacon.o util.o sensor.o
DEPS = ${OBJS:.o=.d}

all: minigate

install: all
	/usr/bin/install -D --mode=755 minigate /usr/local/bin/minigate

minigate: $(OBJS)
	$(CXX) -o $@ $^ -lwiringPi -lcrypt -lrt -pthread

gpiostat: gpiostat.cpp
	$(CXX) gpiostat.cpp -O2 -lwiringPi -lcrypt -lrt -pthread -o gpiostat

-include $(DEPS)

clean:
	rm -f minigate $(OBJS) $(DEPS)
