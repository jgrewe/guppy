#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import print_function, division
import nix
import cv2
import argparse
import os
import time
import numpy as np
from IPython import embed

    
def read_frame_times(filename):
    times_file = filename[:-4] + '_times.dat'
    times = []
    if os.path.exists(times_file):
        with open(times_file) as f:
            for l in f.readlines():
                times.append(float(l.rstrip()[6:]))
        return times;
    else:
        return None


def check_tags(start, end, frame_count):
    if len(start) == 0 and len(end) == 0:
        start.append(0)
        end.append(frame_count)

    if len(start) > len(end):
        i = 1
        while i < len(start):
            if i < len(end):
                if start[i] > end[i]:
                    start.insert(i, end[i-1])
                else:
                    i +=1
            else:
                if i+1 < len(start):
                    end.append(start[i+1])
                    i +=1
                else:
                    end.append(frame_count)
                    i = i + 1
        if len(end) < len(start):
            end.append(frame_count)
    else:
        i = 0
        while i < len(end):
            if i < len(start):
                if end[i] < start[i] and i > 0:
                    start.insert(i, end[i-1])
                i += 1
            else:
                if i is 0:
                    start.append(0)
                else:
                    start.append(end[i-1])
                i += 1     
    if start[-1] > end[-1]:
        end.append(frame_count)
    

def play_avi(filename, time_scale):
    start_tags = []
    end_tags = []
    frame_times = read_frame_times(filename)
    video = cv2.VideoCapture()
    video.open(filename)
    begin = time.time()
    intervals = []
    frames = None
    if frame_times:
        intervals = np.diff(frame_times)*1000.
        intervals = np.hstack((intervals, np.mean(intervals)))
    else:
        intervals.append(1000//video.get(cv2.cv.CV_CAP_PROP_FPS))
    success, frame = video.read()
    axis = 2
    if success and frame.shape[-1] == 3:
        axis = 3
    k = 0
    while success:
        if k == 0:
            frames = frame[...,np.newaxis]
        else:
            frames = np.concatenate((frames, frame[...,np.newaxis]),axis=axis)
        start = time.time()
        cv2.imshow('frame', frame)
        if frame_times:
            wait_interval = np.max((1, int((intervals[k] - (time.time() - start) * 1000) * time_scale)))
        else:
            wait_interval = np.max((1, int((intervals[0] - (time.time() - start) * 1000) * time_scale)))
            
        key = cv2.waitKey(wait_interval)
        if key & 0xFF == ord('q'):
            break
        elif key & 0xFF == ord('s'):
            start_tags.append(k)
        elif key & 0xFF == ord('e'):
            end_tags.append(k)
        k+=1
        success, frame = video.read()
            
    video.release()
    cv2.destroyAllWindows()
    check_tags(start_tags, end_tags, k)
    return start_tags, end_tags, frame_times, frames


def grab_frames(filename):
    start_tags = [0]
    end_tags = []
    frame_times =  []
    frames = None
    video = cv2.VideoCapture(filename)
    frame_time = 1000//video.get(cv2.cv.CV_CAP_PROP_FPS)
    success, frame = video.read()
    frame_count = 0
    axis = 2
    if success and frame.shape[-1] == 3:
        axis = 3
    while success:
        if not frames:
            frames = frame[...,np.newaxis]
        else:
            frames = np.concatenate((frames, frame[...,np.newaxis]),axis=axis)
        frame_times.append(frame_count * frame_time)
        frame_count += 1
        success, frame = video.read()
    video.release()
    end_tags = [frame_count]
    return start_tags, end_tags, frame_times, frames


def create_nix_file(file_name):
    nix_file = nix.File.open(file_name, nix.FileMode.Overwrite)
    block_name = file_name.split('/')[-1].split('.')[-2]
    nix_file.create_block(block_name, 'recording')
    return nix_file


def save_frames(nix_file, frames, frame_times):
    data_type = "nix.stamped_video_monochrom";
    if frames[0].shape[-1]  == 3:
      data_type = "nix.stamped_video_RGB";
    block = nix_file.blocks[0]

    video_data = block.create_data_array("video", data_type, nix.DataType.UInt8,  frames.shape);
    sd = video_data.append_sampled_dimension(1.0);
    sd.label = "height"
    sd = video_data.append_sampled_dimension(1.0);
    sd.label= "width"
    if  frames.shape[-1] == 3:
         dim = video_data.append_set_dimension();
         dim.labels({"R", "G", "B"});
    time_dim = video_data.append_range_dimension(frame_times);
    time_dim.label = "time"
    time_dim.unit = "ms"

    video_data.data.write_direct(frames)
    tag_positions = block.create_data_array("tag times", "nix.event.positions", nix.DataType.Float, (1, 1))
    tag_positions.append_set_dimension()

    tag_extents = block.create_data_array("tag extents", "nix.event.extents", nix.DataType.Float, (1, 1))
    tag_extents.append_set_dimension()

    tags = block.create_multi_tag("tags", "nix.event", tag_positions)
    tags.extents = tag_extents
    tags.references.append(video_data)


def save_tags(nix_file, start_tags, end_tags, frames_times, tag_rois=None):
    if len(start_tags) != len(end_tags):
        raise ValueError('start_tags and end_tags do not have the same number of entries!')
        
    if tag_rois and tag_rois.shape[-1] != len(end_tags):
        raise ValueError('There are ROIs than there are tags!')
    block = nix_file.blocks[0]
    tags = block.multi_tags[0]
    video_size = tags.references[0].data_extent
    position_data = np.zeros((len(video_size), len(start_tags)))
    extent_data = np.zeros((len(video_size), len(start_tags)))

    if not tag_rois:
        tag_rois = np.ones((len(video_size)-1, len(start_tags)))
        for i in range(len(start_tags)):
            tag_rois[:,i] = video_size[:-1]
    
    for i, s_t in enumerate(start_tags):
        position_data[len(video_size) - 1][i] = frame_times[s_t]

    for i, (s_t, e_t) in enumerate(zip(start_tags, end_tags)):
        for j, siz in enumerate(tag_rois[:, i]):
            extent_data[j][i] = siz
        extent_data[len(video_size) - 1][i] = frame_times[e_t] - frame_times[s_t]

    tags.positions.data_extent = position_data.shape
    tags.extents.data_extent = extent_data.shape
    tags.positions.data.write_direct(position_data)
    tags.extents.data.write_direct(extent_data)


def nix_export(file_name, start_tags, end_tags, frame_times, frames) :
    nix_file = create_nix_file(file_name)
    save_frames(nix_file, frames, frame_times)
    save_tags(nix_file, start_tags, end_tags, frame_times)
    if nix_file:
        nix_file.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='guppy avi to nix converter with tagging option')
    parser.add_argument('file', type=str, help="filename of the file-to-convert")
    parser.add_argument('-g', '--gui', action="store_true", help="with video playback with the option to tag selections")
    parser.add_argument('-o', '--output', type=str, default=None, help="specifies the name of the output file")
    parser.add_argument('-s','--speed', type=float, default=1.,
                        help="playback speed given as a scaling of the original framerate e.g. 2.0 for double, 0.5 for half etc.")
    args = parser.parse_args()
    
    if not os.path.exists(args.file):
        print('File does not exits!')
        exit()
    output_name = arg.output if args.output else args.file.split('.')[-2] + '.h5'
    if os.path.exists(output_name):
        ans = raw_input('Output file %s already exists! Overwrite? ([y]/n/c): ' % output_name)
        if ans == 'c':
            print('\tExport cancelled!')
            exit()
        elif ans == 'n':
            output_name = raw_input('Please give a new file name: ')
            if '/' in output_name or '\\' in output_name:
                print ('TODO: make sure the path exists!')
    
    if args.gui:
        start_tags, end_tags, frame_times, frames = play_avi(args.file, args.speed)
    else:
        start_tags, end_tags, frame_times, frames = grab_frames(args.file)
    nix_export(output_name, start_tags, end_tags, frame_times, frames)

