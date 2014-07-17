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
#include <opencv2/highgui/highgui.hpp>
#include "Guppy.hpp"
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>

using namespace std;
using namespace cv;

string getDate(){
  boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
  return to_iso_extended_string(current_date);
}

int main() {
  int video_count = 0;
  Guppy cam(0);
  cam.exposure(250);
  if (!cam.isOpened()) {
    cout << "Cannot open camera!" << endl;
    return -1;
  }
  int codec = CV_FOURCC('M', 'J', 'P', 'G');
  // Size frameSize(752, 580);
  VideoWriter oVideoWriter;
  namedWindow("MyVideo",CV_WINDOW_AUTOSIZE);
  Mat frame;
  bool recording = false;
  while (true) {
    bool bSuccess = cam.getFrame(frame);
    
    if (!bSuccess) {
      cout << "Cannot read a frame from camera!!" << endl;
      break;
    }
    int key = waitKey(50);
    if (key % 256 == 27) {
      cout << "exit!" << endl;
      break;
    }
    else if (key % 256 == 32) {
      if (!recording) {
	Size frameSize(frame.cols, frame.rows);
	oVideoWriter.open(getDate() + "_" + to_string(video_count) + ".avi", codec, 20, frameSize, false);
	cout << "recording started..." << endl;
	recording = true;
	if ( !oVideoWriter.isOpened() )
	  {
	    cout << "ERROR: Failed to write the video" << endl;
	    return -1;
	  }
	video_count ++;
      } else {
	recording = false;
	cout << "recording stopped!" << endl;
      }
    }
    if(recording && oVideoWriter.isOpened()) {
      oVideoWriter.write(frame); 
    }
    imshow("MyVideo", frame); 
  }
  cam.closeCam();
  return 0;
}
