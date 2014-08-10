#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>
#include <nix.hpp>

using namespace std;
using namespace cv;

class movieWriter {

public:
  movieWriter(){};

  movieWriter(const movieWriter &other);

  movieWriter(bool nix_io, const string &tag_type, int movie_count, const Size &frame_size, int channels=1);

  ~movieWriter(){};

private:
  void open();

  string getDate() const;

public:
  void create(bool nix_io, const string &tag_type, int movie_count, const Size &frame_size, int channels=1);

  bool writeFrame(const Mat &frame, const boost::posix_time::time_duration &time_stamp);
  
  bool isOpen() const;
  
  void close();


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
    
};
