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
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>

using namespace std;
using namespace cv;
using namespace boost;
namespace opt = boost::program_options;

class movie_writer {
public:
  movie_writer(){};

  movie_writer(const movie_writer &other):nix_io(other.nix_io), tag_type(other.tag_type), index(other.index), frame_size(other.frame_size)
  {
    
  };

  movie_writer(bool nix_io, const string &tag_type, int movie_count, const Size &frame_size):nix_io(nix_io), tag_type(tag_type), index(movie_count), frame_size(frame_size) {
    this->open();
  };
  
  void create(bool nix_io, const string &tag_type, int movie_count, const Size &frame_size) {
    if (this->isOpen()){
      this->close();
    }
    this->nix_io = nix_io;
    this->tag_type = tag_type;
    this->index = movie_count;
    this->frame_size = frame_size;
    this->open();
  };

  bool writeFrame(const Mat &frame, const boost::posix_time::ptime &time_stamp){
    if(!this->isOpen()) {
      return false;
    }
    this->oVideoWriter.write(frame);
    this->ofs << time_stamp << endl;
    return true;
  };
  
  bool isOpen() {
    //cerr << "isOpen video: " << oVideoWriter.isOpened() << endl;
    //    cerr << "isOpen stream: " << this->ofs.is_open() << endl;
    return (this->oVideoWriter.isOpened() && this->ofs.is_open());
  };
  
  void close() {
    if(this->ofs.is_open()) {
      this->ofs.close();
    }
  };

  ~movie_writer(){};
private:
  bool nix_io;
  string tag_type;
  int index;
  string filename;
  ofstream ofs; 
  Size frame_size;
  int codec = CV_FOURCC('M', 'J', 'P', 'G');
  VideoWriter oVideoWriter;
  
  void open(){
    this->filename = getDate() + "_" + to_string(index);
    if(nix_io){
      cerr << "nix_io" << endl;
    }
    else {
      cerr << "open():" << this->filename << endl; 
      cerr << this->codec << endl;
      cerr << this->frame_size << endl;
      this->oVideoWriter.open(this->filename + ".avi", this->codec, 25, this->frame_size, true);
      this->ofs.open(this->filename + "_times.dat", ofstream::out);
      cerr << "open() writer:" << this->oVideoWriter.isOpened() << endl;
      cerr << "open() stream:" << this->ofs.is_open() << endl;
    }
  }

  string getDate(){
    boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
    return to_iso_extended_string(current_date);
  }
    
};


int main(int ac, char* av[]) {
  bool interlace = false;
  bool nix_io = false;
  string tag_type;
  movie_writer mv;
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
  boost::posix_time::ptime t1;
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
	mv.create(nix_io, tag_type, video_count, frameSize);
	cerr << "recording started..." << endl;
	recording = true;
	cerr << mv.isOpen() << nix_io << endl;
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
      mv.writeFrame(frame, t1);
    }
    imshow("MyVideo", frame); 
  }
  cam.closeCam();
  return 0;
}
