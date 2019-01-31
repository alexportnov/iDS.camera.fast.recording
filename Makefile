LIBS=-lueye_api -lopencv_core -lopencv_videoio -lopencv_highgui -lopencv_video

.DEFAULT_GOAL := all
all:
	g++ -O3 -Wall -I/usr/local/include/opencv4 -o framecap main.cpp $(LIBS)
