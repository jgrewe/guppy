guppy-gui: guppy movieWriter src/guppy-gui.cpp
	 g++ -I/usr/include/opencv2 -I/usr/include/libboost -I/usr/local/nix -O0 -g3 -Wall -c -o gui.o src/guppy-gui.cpp -std=c++11
	 g++ -L/usr/lib -o "guppy-gui"  guppy.o gui.o movieWriter.o -lopencv_core -lopencv_highgui -lopencv_imgproc -lboost_date_time -lboost_program_options -lnix

movieWriter: src/movieWriter.cpp include/movieWriter.hpp
	 g++ -I/usr/include/opencv2 -I/usr/include/libboost -I/usr/local/nix -O0 -g3 -Wall -c -o movieWriter.o src/movieWriter.cpp -std=c++11

guppy: src/guppy.cpp include/guppy.hpp
	 g++ -I/usr/include/opencv2 -I/usr/include/libboost -O0 -g3 -Wall -c -o guppy.o src/guppy.cpp -std=c++11


