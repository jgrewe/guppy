#include "../include/movieWriter.hpp"


movieWriter::movieWriter(const movieWriter &other):nix_io(other.nix_io), tag_type(other.tag_type), index(other.index), channels(other.channels), frame_size(other.frame_size)
{
    
};


movieWriter::movieWriter(bool nix_io, const string &tag_type, int movie_count, const Size &frame_size, int channels):nix_io(nix_io), tag_type(tag_type), index(movie_count), channels(channels) {
if(channels == 1){
    this->frame_size =  {frame_size.height, frame_size.width, 1};
    this->channel_index = 2;
    this->offset = {0, 0, 0};
  } else {
    this->frame_size =  {frame_size.height, frame_size.width, channels, 1};
    this->channel_index = 3;
    this->offset = {0, 0, 0, 0};
  }  
  this->open();
};
  

void movieWriter::create(bool nix_io, const string &tag_type, int movie_count, const Size &frame_size, int channels) {
  if (this->isOpen()){
    this->close();
  }
  this->nix_io = nix_io;
  this->tag_type = tag_type;
  this->index = movie_count;
  this->channels = channels;
  
  if(channels == 1){
    this->frame_size =  {frame_size.height, frame_size.width, 1};
    this->channel_index = 2;
    this->offset = {0, 0, 0};
  } else {
    this->frame_size =  {frame_size.height, frame_size.width, channels, 1};
    this->channel_index = 3;
    this->offset = {0, 0, 0, 0};
  }
  this->data_size = this->frame_size;
  this->open();
};


bool movieWriter::writeFrame(const Mat &frame, const boost::posix_time::time_duration &time_duration){
  if(!this->isOpen()) {
    return false;
  }
  if(nix_io) {
    //TODO polishing!!!
    //TODO tags
    //TODO Test images
    //TODO metadata
    //TODO problem with repeated recordings, a nix problem?!
    video_data.dataExtent(this->data_size);
    video_data.setData(nix::DataType::UInt8, frame.ptr(), this->frame_size, this->offset);
    vector<double> time = time_dim.ticks();
    if(frame_count > 0) {
      time.push_back((double)time_duration.total_milliseconds());
      time_dim.ticks(time);
    }
    this->offset[this->channel_index] = this->data_size[channel_index];
    this->data_size[channel_index]++;
  } else {
    this->cvWriter.write(frame);
    this->ofs << time_duration << endl;
  }
  frame_count++;
  return true;
};

  
bool movieWriter::isOpen() const {
  if(this->nix_io) {
    return this->nix_file.isOpen();
  }
  else {
    return (this->cvWriter.isOpened() && this->ofs.is_open());
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
    string type = "nix.stamped_video_monochrom";
    if(this->channels == 3) {
      type = "nix.stamped_video_RGB";
    }
    video_data = recording_block.createDataArray("video", type, nix::DataType::UInt8, this->frame_size);
    nix::SampledDimension sd = video_data.appendSampledDimension(1.0);
    sd.label("height");
    sd = video_data.appendSampledDimension(1.0);
    sd.label("width");
    if(this->channels == 3) {
      nix::SetDimension dim = video_data.appendSetDimension();
      dim.labels({"R", "G", "B"});
    }
    time_dim = video_data.appendRangeDimension({0.0});
    time_dim.label("time");
    time_dim.unit("ms");

    //TODO create DataTag for tags, i.e, create DataArray with set to tag times.
  }
  else {
    cv::Size size{(int)this->frame_size[1], (int)this->frame_size[0]};
    if(channels > 1) {
      this->cvWriter.open(this->filename + ".avi", this->codec, 25, size, true);
    } else {
      this->cvWriter.open(this->filename + ".avi", this->codec, 25, size, false);
    }
    this->ofs.open(this->filename + "_times.dat", ofstream::out);
  }
}


string movieWriter::getDate() const {
  boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
  return to_iso_extended_string(current_date);
}
 
