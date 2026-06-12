CXX = g++
CXXFLAGS = -fPIC -O2 -Wall
LDFLAGS = -shared
MAINFLAGS = -ldl

all: libaffine.so libdh.so main

libaffine.so: affine_lib.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o libaffine.so affine_lib.cpp

libdh.so: dh_lib.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o libdh.so dh_lib.cpp

main: main.cpp
	$(CXX) main.cpp -o main $(MAINFLAGS)

clean:
	rm -f libaffine.so libdh.so main

run: all
	./main
