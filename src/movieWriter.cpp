#include "../include/movieWriter.hpp"

using namespace std;
using namespace boost;
using namespace cv;
using namespace nix;

/*
  movieWriter::movieWriter(const movieWriter &other):nix_io(other.nix_io), tag_type(other.tag_type),
  index(other.index), channels(other.channels),
  frame_size(other.frame_size)
  {
  };
*/

movieWriter::movieWriter(bool nix_io, const string &tagging_type, int movie_count, const Size &image_size, int channel_count)
  :use_nix(nix_io), tag_type(tagging_type), index(movie_count), channels(channel_count) {
  if (channels == 1){
    frame_size =  {image_size.height, image_size.width, 1};
    channel_index = 2;
    offset = {0, 0, 0};
  } else {
    frame_size =  {image_size.height, image_size.width, channels, 1};
    channel_index = 3;
    offset = {0, 0, 0, 0};
  }
  open();
};


void movieWriter::create(bool nix_io, const string &tagging_type, int movie_count, const Size &image_size, int channel_count) {
  if ( isOpen()){
    close();
  }
  use_nix = nix_io;
  tag_type = tagging_type;
  index = movie_count;
  channels = channel_count;
  nix_file = nix::none;
  if (channels == 1){
    frame_size =  {image_size.height, image_size.width, 1};
    channel_index = 2;
    offset = {0, 0, 0};
  } else {
    frame_size =  {image_size.height, image_size.width, channels, 1};
    channel_index = 3;
    offset = {0, 0, 0, 0};
  }
  data_size =  frame_size;
  open();
};


void movieWriter::open(){
  frame_count = 0;
  filename = getDate() + "_";
  filename = createFilename(filename);
  if (use_nix){
    nix_file = File::open(filename, FileMode::Overwrite);
    cerr << "open file: " << filename << endl;

    Block recording_block = nix_file.createBlock( filename, "recording");
    string type = "nix.stamped_video_monochrom";
    if (channels == 3) {
      type = "nix.stamped_video_RGB";
    }

    video_data = recording_block.createDataArray("video", type, nix::DataType::UInt8,  frame_size);
    SampledDimension sd = video_data.appendSampledDimension(1.0);
    sd.label("height");
    sd = video_data.appendSampledDimension(1.0);
    sd.label("width");
    if ( channels == 3) {
      SetDimension dim = video_data.appendSetDimension();
      dim.labels({"R", "G", "B"});
    }
    time_dim = video_data.appendRangeDimension({0.0});
    time_dim.label("time");
    time_dim.unit("ms");

    tag_positions = recording_block.createDataArray("tag times", "nix.event.positions", nix::DataType::Int64, {1, 1});
    tag_positions.appendSetDimension();

    tag_extents = recording_block.createDataArray("tag extents", "nix.event.extents", nix::DataType::Int64, {1, 1});
    tag_extents.appendSetDimension();

    tags = recording_block.createMultiTag("tags",  tag_type, tag_positions);
    tags.extents(tag_extents);
    tags.addReference(video_data);
  } else {
    cv::Size size{(int) frame_size[1], (int) frame_size[0]};
    if (channels > 1) {
      cvWriter.open( filename,  codec, 25, size, true);
    } else {
      cvWriter.open( filename,  codec, 25, size, false);
    }
    ofs.open( filename.substr(0,filename.length()-4) + "_times.dat", ofstream::out);
  }
}


void movieWriter::close() {
  if (isOpen()) {
    if (use_nix) {
      writeFrameTimes();
      if (tag_times.size() > 0){
	writeTagTimes();
      }
      nix_file.close();
      video_data = nix::none;
      tag_positions = nix::none;
      tag_extents = nix::none;
      tags = nix::none;
      time_dim = nix::none;
      frame_times.clear();
      tag_times.clear();
    } else {
      if (ofs.is_open()) {
	ofs.close();
      }
    }
  }
}


bool movieWriter::writeFrame(const Mat &frame, const posix_time::time_duration &time_duration){
  if (! isOpen()) {
    return false;
  }
  if (use_nix) {
    //TODO metadata
    video_data.dataExtent( data_size);
    video_data.setData(nix::DataType::UInt8, frame.ptr(),  frame_size,  offset);
    frame_times.push_back(static_cast<double>(time_duration.total_milliseconds()));
    offset[ channel_index] =  data_size[channel_index];
    data_size[channel_index]++;
  } else {
    cvWriter.write(frame);
    ofs << time_duration << endl;
  }
  frame_count++;
  return true;
};


void movieWriter::writeFrameTimes() {
  time_dim.ticks(frame_times);
}


void movieWriter::writeTagTimes() {
  NDSize data_extent = {static_cast<int>( frame_size.size()), static_cast<int>( tag_times.size())};
  typedef multi_array<int, 2> array_type;
  array_type position_data(extents[ frame_size.size()][ tag_times.size()]);
  array_type extent_data(extents[ frame_size.size()][ tag_times.size()]);

  for (size_t i = 0; i !=  tag_times.size(); ++i) {
    position_data[ channel_index][i] =  tag_times[i];
  }
  for (size_t i = 0; i <  tag_times.size(); ++i) {
    for (size_t j = 0; j <  frame_size.size() - 1; ++j) {
      extent_data[j][i] =  frame_size[j];
    }
    extent_data[ channel_index][i] = 1;
  }
  tag_positions.dataExtent(data_extent);
  tag_positions.setData(position_data, {0, 0});
  tag_extents.dataExtent(data_extent);
  tag_extents.setData(extent_data,{0, 0});
}


void movieWriter::tag(const posix_time::time_duration &time_duration) {
  if (!use_nix) {
    cerr << "Tagging is not supported for avi output!\n";
    return;
  }
  tag_times.push_back(static_cast<double>(time_duration.total_milliseconds()));
}


bool movieWriter::isOpen() const {
  if ( use_nix) {
    return  nix_file.isOpen();
  } else {
    return ( cvWriter.isOpened() &&  ofs.is_open());
  }
}


string movieWriter::getDate() const {
  gregorian::date current_date(gregorian::day_clock::local_day());
  return to_iso_extended_string(current_date);
}


string movieWriter::createFilename(string base) {
  string extension = ".h5";
  if (!use_nix) {
    extension = ".avi";
  }
  string filename = base + to_string(index) + extension;
  boost::filesystem::path file_path(filename.c_str());
  while (boost::filesystem::exists(file_path)) {
    index ++;
    filename = base + to_string(index) + extension;
    file_path.remove_leaf() /= filename.c_str();
  }
  return filename;
}


movieWriter::~movieWriter(){
  if (isOpen()) {
    close();
  }
}
