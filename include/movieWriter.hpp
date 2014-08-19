#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>
#include <nix.hpp>

class movieWriter {

public:
  movieWriter(){};

  //movieWriter(const movieWriter &other);

  movieWriter(bool nix_io, const std::string &tagging_type, int movie_count, const cv::Size &image_size, 
	      int channel_count=1);

  ~movieWriter();

private:
  void open();

  std::string getDate() const;
  
  void writeTagTimes();
  
  void writeFrameTimes();

public:
  void create(bool nix_io, const std::string &tagging_type, int movie_count, const cv::Size &image_size,
	      int channel_count=1);

  bool writeFrame(const cv::Mat &frame, const boost::posix_time::time_duration &time_stamp);

  void tag(const boost::posix_time::time_duration &time_stamp);
  
  bool isOpen() const;
  
  void close();


private:
  bool use_nix;
  std::string tag_type;
  int index;
  std::string filename;
  std::ofstream ofs; 
  int channels;
  int codec = CV_FOURCC('M', 'J', 'P', 'G');
  cv::VideoWriter cvWriter;
  nix::File nix_file;
  nix::DataArray video_data, tag_positions, tag_extents;
  nix::MultiTag tags;
  nix::RangeDimension time_dim;
  nix::NDSize frame_size, offset, data_size;
  int frame_count, channel_index;
  std::vector<double> frame_times, tag_times;
};
