#include "../include/movieWriter.hpp"


movieWriter::movieWriter(const movieWriter &other):nix_io(other.nix_io), tag_type(other.tag_type), index(other.index), frame_size(other.frame_size), channels(other.channels)
{
    
};

movieWriter::movieWriter(bool nix_io, const string &tag_type, int movie_count, const Size &frame_size, int channels):nix_io(nix_io), tag_type(tag_type), index(movie_count), frame_size(frame_size), channels(channels) {
  this->open();
};
  
void movieWriter::create(bool nix_io, const string &tag_type, int movie_count, const Size &frame_size, int channels) {
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

bool movieWriter::writeFrame(const Mat &frame, const boost::posix_time::time_duration &time_stamp){
  if(!this->isOpen()) {
    return false;
  }
  if(nix_io) {
    nix::NDSize offset;
    nix::NDSize size;
    nix::NDSize data_size = data_array.dataExtent();
    //TODO polishing!!!
    //TODO tags
    //TODO Test images
    //TODO metadata
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
    if(frame_count > 0) {
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
  
bool movieWriter::isOpen() const {
  if(this->nix_io) {
    return this->nix_file.isOpen();
  }
  else {
    return (this->oVideoWriter.isOpened() && this->ofs.is_open());
  }
};
  
void movieWriter::close() {
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

void movieWriter::open(){
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

string movieWriter::getDate() const {
  boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
  return to_iso_extended_string(current_date);
}
 
