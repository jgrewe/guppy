//============================================================================
// Name        : guppy-gui.cpp
// Author      : Jan Grewe
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
/*
  g++ -I/usr/include/opencv2 -I/usr/include/libboost -O0 -g3 -Wall -c -o test.o test.cpp -std=c++11
  g++ -L/usr/lib -o "guppy_t"  Guppy.o test.o   -lopencv_core -lopencv_highgui -lopencv_imgproc -lboost_date_time
*/
#include <iostream>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include "../include/guppy.hpp"
#include "../include/movieWriter.hpp"
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>

using namespace std;
using namespace cv;
using namespace boost;
namespace opt = boost::program_options;


int main(int ac, char* av[]) {
  bool interlace = false;
  bool nix_io = false;
  string tag_type;
  movieWriter mv;
  opt::options_description desc("Options");
  desc.add_options()
    ("help", "produce help")
    ("interlaced", opt::value<bool>(&interlace)->default_value(false), "if set images are converted before writing. e.g with some fire wire cameras")
    ("nix-io", opt::value<bool>(&nix_io)->default_value(false), "write output data to nix files")
    ("tag-type", opt::value<string>(&tag_type)->default_value("nix.behavioral_event"), "The type of tag stored when \"t\" is pressed during recording (only applicable with nix-io)")
    ;

  opt::variables_map vm;
  opt::store(opt::parse_command_line(ac, av, desc), vm);
  opt::notify(vm);
  if(vm.count("help")){
    cerr << desc << "\n";
    return 1;
  }
  int video_count = 0;	
  boost::posix_time::ptime start_time, t1;
  boost::posix_time::time_duration td;
  Guppy cam(0, interlace);
  cam.exposure(250);
  if (!cam.isOpened()) {
    cerr << "Cannot open camera!" << endl;
    return -1;
  }
  //Size frameSize(752, 580);
  namedWindow("MyVideo",CV_WINDOW_AUTOSIZE);
  Mat frame;
  bool recording = false;
  while (true) {
    bool bSuccess = cam.getFrame(frame);
    t1 = boost::posix_time::microsec_clock::local_time();
    
    if (!bSuccess) {
      cerr << "Cannot read a frame from camera!!" << endl;
      break;
    }
    int key = waitKey(10);
    if (key % 256 == 27) {
      cerr << "exit!" << endl;
      break;
    }
    else if (key % 256 == 32) {
      if (!recording) {
	Size frameSize(frame.cols, frame.rows);
	boost::posix_time::ptime c = boost::posix_time::microsec_clock::local_time();
	mv.create(nix_io, tag_type, video_count, frameSize, frame.channels());
	start_time = boost::posix_time::microsec_clock::local_time();
	cerr << "recording started..." << (start_time - c) << endl;
	recording = true;
	if (!mv.isOpen()) {
	  cerr << "ERROR: Failed to write the video or times" << endl;
	  return -1;
	}
	video_count ++;
      } else {
	recording = false;
	mv.close();
	cerr << "recording stopped!" << endl;
      }
    }
    if(recording) {
      td = t1 - start_time;
      cerr << "start time: " << start_time << endl;
      cerr << "\t elapsed time: " << td << endl;
      mv.writeFrame(frame, td);
    }
    imshow("MyVideo", frame); 
  }
  cam.closeCam();
  return 0;
}
