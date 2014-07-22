/*
 * Guppy.cpp
 *
 *  Created on: Oct 30, 2013
 *      Author: grewe
 */

#include "../include/guppy.hpp"

using namespace cv;

Guppy::Guppy(int cam_no, bool interlaced):interlaced(interlaced) {
  cap.open(cam_no);
  dimensions();
  exposure(100.0);
}

bool Guppy::isOpened() const {
  return cap.isOpened();
}

void Guppy::closeCam(){
  cap.release();
}

void Guppy::exposure(double exposure){
  cap.set(CV_CAP_PROP_EXPOSURE, exposure);
}

double Guppy::exposure(){
  return cap.get(CV_CAP_PROP_EXPOSURE);
}

void Guppy::dimensions(double width, double height){
  cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
}

double Guppy::frameWidth(){
  return cap.get(CV_CAP_PROP_FRAME_WIDTH);
}

double Guppy::frameHeight(){
  return cap.get(CV_CAP_PROP_FRAME_HEIGHT);
}

bool Guppy::getFrame(Mat &frame){
  Mat temp;
  bool success = cap.read(temp);
  if(success && interlaced){
    frame.create(temp.rows, temp.cols, temp.type());
    int row1 = 0;
    int row2 = temp.rows/2;
    for (int i = 0; i < temp.rows; i+=2){
      temp.row(row2).copyTo(frame.row(i));
      temp.row(row1).copyTo(frame.row(i+1));
      row2++;
      row1++;
    }
  } else {
    frame = temp;
  }
  return success;
}

Guppy::~Guppy() {
  if(isOpened()){
    closeCam();
  }
}

