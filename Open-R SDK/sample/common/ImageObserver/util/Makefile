#
# Copyright 2002 Sony Corporation 
#
# Permission to use, copy, modify, and redistribute this software for
# non-commercial use is hereby granted.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#

CXX=g++
CFLAGS=
LIBS=

all: cdt2bmp layerh2bmp

%.o: %.cc
	$(CXX) $(CFLAGS) -o $@ -c $^

layerh2bmp: layerh2bmp.o BMP.o
	$(CXX) -o $@ layerh2bmp.o BMP.o $(LIBS)

cdt2bmp: cdt2bmp.o BMP.o
	$(CXX) -o $@ cdt2bmp.o BMP.o $(LIBS)

clean:
	rm -f layerh2bmp.exe layerh2bmp cdt2bmp.exe cdt2bmp *.o *.bmp

sample: cdt2bmp
	./cdt2bmp cdt00.bmp 0 230 150 190 120
	./cdt2bmp cdt01.bmp 1 230 150 190 120
	./cdt2bmp cdt02.bmp 2 230 150 190 120
	./cdt2bmp cdt03.bmp 3 230 150 190 120
	./cdt2bmp cdt04.bmp 4 230 150 190 120
	./cdt2bmp cdt05.bmp 5 230 150 190 120
	./cdt2bmp cdt06.bmp 6 230 150 190 120
	./cdt2bmp cdt07.bmp 7 230 150 190 120
	./cdt2bmp cdt08.bmp 8 230 150 190 120
	./cdt2bmp cdt09.bmp 9 230 150 190 120
	./cdt2bmp cdt10.bmp 10 230 150 190 120
	./cdt2bmp cdt11.bmp 11 230 150 190 120
	./cdt2bmp cdt12.bmp 12 230 150 190 120
	./cdt2bmp cdt13.bmp 13 230 150 190 120
	./cdt2bmp cdt14.bmp 14 230 150 190 120
	./cdt2bmp cdt15.bmp 15 230 150 190 120
	./cdt2bmp cdt16.bmp 16 230 150 190 120
	./cdt2bmp cdt17.bmp 17 230 150 190 120
	./cdt2bmp cdt18.bmp 18 230 150 190 120
	./cdt2bmp cdt19.bmp 19 230 150 190 120
	./cdt2bmp cdt20.bmp 20 230 160 190 120
	./cdt2bmp cdt21.bmp 21 230 160 190 120
	./cdt2bmp cdt22.bmp 22 230 160 190 120
	./cdt2bmp cdt23.bmp 23 230 160 190 120
	./cdt2bmp cdt24.bmp 24 230 160 190 120
	./cdt2bmp cdt25.bmp 25 230 160 190 120
	./cdt2bmp cdt26.bmp 26 230 160 190 120
	./cdt2bmp cdt27.bmp 27 230 160 190 120
	./cdt2bmp cdt28.bmp 28 230 160 190 120
	./cdt2bmp cdt29.bmp 29 230 160 190 120
	./cdt2bmp cdt30.bmp 30 230 160 190 120
	./cdt2bmp cdt31.bmp 31 230 160 190 120
