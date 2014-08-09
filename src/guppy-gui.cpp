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
#include <nix.hpp>

using namespace std;
using namespace cv;
using namespace boost;
namespace opt = boost::program_options;

class movie_writer {
public:
  movie_writer(){};

  movie_writer(const movie_writer &other):nix_io(other.nix_io), tag_type(other.tag_type), index(other.index), frame_size(other.frame_size), channels(other.channels)
  {
    
  };

  movie_writer(bool nix_io, const string &tag_type, int movie_count, const Size &frame_size, int channels=1):nix_io(nix_io), tag_type(tag_type), index(movie_count), frame_size(frame_size), channels(channels) {
    this->open();
  };
  
  void create(bool nix_io, const string &tag_type, int movie_count, const Size &frame_size, int channels=1) {
    if (this->isOpen()){
      this->close();
    }
    this->nix_io = nix_io;
    this->tag_type = tag_type;
    this->index = movie_count;
    this->frame_size = frame_size;
    this->channels = channels;
    this->open();
  };

  bool writeFrame(const Mat &frame, const boost::posix_time::time_duration &time_stamp){
    if(!this->isOpen()) {
      return false;
    }
    if(nix_io) {
      nix::NDSize offset;
      nix::NDSize size;
      nix::NDSize data_size = data_array.dataExtent();
      //TODO polishing!!!
      //TODO tags
      //TODO make it own file/header...
      //TODO Test images
      if(this->channels > 1){
        offset = {0, 0, 0, this->frame_count};
	size =  {this->frame_size.height, this->frame_size.width, this->channels, 1};
	data_size[3]++;
      } else {
        offset = {0, 0, this->frame_count};
	size =  {this->frame_size.height, this->frame_size.width, 1};
	data_size[2]++;
      }
      data_array.dataExtent(data_size);
      data_array.setData(nix::DataType::UInt8, frame.ptr(), size, offset);
      vector<double> time = time_dim.ticks();
      if(frame_count > 1) {
	time.push_back((double)time_stamp.total_milliseconds());
	time_dim.ticks(time);
      }
      cerr << time.size() << endl;
    } else {
      this->oVideoWriter.write(frame);
      this->ofs << time_stamp << endl;
    }
    frame_count++;
    return true;
  };
  
  bool isOpen() {
    if(this->nix_io) {
      return this->nix_file.isOpen();
    }
    else {
      return (this->oVideoWriter.isOpened() && this->ofs.is_open());
    }
  };
  
  void close() {
    if(this->isOpen()) {
      if(this->nix_io) {
	this->nix_file.close();
      } else { 
	if(this->ofs.is_open()) {
	  this->ofs.close();
	}
      }
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
  int channels;
  int codec = CV_FOURCC('M', 'J', 'P', 'G');
  VideoWriter oVideoWriter;
  nix::File nix_file;
  nix::DataArray data_array, tag_array;
  nix::RangeDimension time_dim;
  int frame_count;

  void open(){
    this->frame_count = 0;
    this->filename = getDate() + "_" + to_string(index);
    if(nix_io){
      nix_file = nix::File::open(this->filename + ".h5", nix::FileMode::Overwrite);
      nix::Block recording_block = nix_file.createBlock(this->filename, "recording");
      nix::NDSize video_size{this->frame_size.height, this->frame_size.width, this->channels, 1};
      string type = "nix.stamped_video_monochrom";
      if(this->channels == 3) {
	type = "nix.stamped_video_RGB";
      }
      data_array = recording_block.createDataArray("video", type, nix::DataType::UInt8, video_size);
      nix::SampledDimension sd = data_array.appendSampledDimension(1.0);
      sd.label("height");
      sd = data_array.appendSampledDimension(1.0);
      sd.label("width");
      if(this->channels == 3) {
	nix::SetDimension dim = data_array.appendSetDimension();
	dim.labels({"R", "G", "B"});
      }
      time_dim = data_array.appendRangeDimension({0.0});
      time_dim.label("time");
      time_dim.unit("ms");

      //TODO create DataTag for tags, i.e, create DataArray with set to tag times.
    }
    else {
      if(channels > 1) {
	this->oVideoWriter.open(this->filename + ".avi", this->codec, 25, this->frame_size, true);
      } else {
	this->oVideoWriter.open(this->filename + ".avi", this->codec, 25, this->frame_size, false);
      }
      this->ofs.open(this->filename + "_times.dat", ofstream::out);
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
	mv.create(nix_io, tag_type, video_count, frameSize, frame.channels());
	start_time = boost::posix_time::microsec_clock::local_time();
	cerr << "recording started..." << endl;
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
