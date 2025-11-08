CXX = g++
CXXFLAGS = -std=c++17 -Wall
LDFLAGS = -lncurses

SRC = src/main.cpp src/sysinfo.cpp
INC = -Iinclude

all:
	$(CXX) $(SRC) $(INC) $(LDFLAGS) -o system_monitor_tool

run: all
	./system_monitor_tool

clean:
	rm -f system_monitor_tool
