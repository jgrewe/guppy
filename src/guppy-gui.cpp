//============================================================================
// Name        : guppy.cpp
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
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/program_options.hpp>

using namespace std;
using namespace cv;
//using namespace boost;
//namespace opt = boost::program_options;

string getDate(){
  boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
  return to_iso_extended_string(current_date);
}
/*
void setOptions(opt::options_description &desc) {
  desc.add_options()
    ("help", "produce help message")
    ("interlaced", opt::value<bool>(), "if true converts images")
    ("nix-io", opt::value<bool>(), "write output data to nix files")
    ;
}
*/
/*
void readOptions(int ac, char** av[], opt::option_description &desc, opt::variables_map &vm) {
  opt::store(opt::parse_command_line(ac, av, desc), vm);
  opt::notify(vm);
}
*/
int main(int ac, char* av[]) {
  //opt::options_description desc("Options");
  //setOptions(desc);
  //opt::variables_map vm;
  //opt::store(opt::parse_command_line(ac, av, desc, 0), vm);
  //opt::notify(vm);
  //readOptions(ac, av, desc, vm);
  int video_count = 0;	
  std::string filename;
  ofstream ofs;
  boost::posix_time::ptime t1;
  Guppy cam(0, true);
  cam.exposure(250);
  if (!cam.isOpened()) {
    cout << "Cannot open camera!" << endl;
    return -1;
  }
  int codec = CV_FOURCC('M', 'J', 'P', 'G');
  //Size frameSize(752, 580);
  VideoWriter oVideoWriter;
  namedWindow("MyVideo",CV_WINDOW_AUTOSIZE);
  Mat frame;
  bool recording = false;
  while (true) {
    bool bSuccess = cam.getFrame(frame);
    t1 = boost::posix_time::microsec_clock::local_time();
    if (!bSuccess) {
      cout << "Cannot read a frame from camera!!" << endl;
      break;
    }
    int key = waitKey(10);
    if (key % 256 == 27) {
      cout << "exit!" << endl;
      break;
    }
    else if (key % 256 == 32) {
      if (!recording) {
	Size frameSize(frame.cols, frame.rows);
	filename = getDate() + "_" + to_string(video_count);
	oVideoWriter.open(filename + ".avi", codec, 25, frameSize, false);
	ofs.open(filename + "_times.dat", ofstream::out);
	cout << "recording started..." << endl;
	recording = true;
	if ( !oVideoWriter.isOpened() || !ofs.is_open()) {
	  cout << "ERROR: Failed to write the video or times" << endl;
	  return -1;
	}
	video_count ++;
      } else {
	recording = false;
	if(ofs.is_open()) {
	  ofs.close();
	}
	cout << "recording stopped!" << endl;
      }
    }
    if(recording && oVideoWriter.isOpened() && ofs.is_open()) {
      oVideoWriter.write(frame);
      ofs << t1 << endl;
    }
    imshow("MyVideo", frame); 
  }
  cam.closeCam();
  return 0;
}
