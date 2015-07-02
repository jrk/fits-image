test: test.cpp fits_image.h
	g++ -g -std=c++11 -I/usr/local/share/halide/tutorial -lHalide -lcfitsio `libpng-config --cflags --ldflags` test.cpp -o test
