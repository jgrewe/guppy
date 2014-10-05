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
        return times
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
                    i += 1
            else:
                if i + 1 < len(start):
                    end.append(start[i+1])
                    i += 1
                else:
                    end.append(frame_count)
                    i += 1
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


def export_avi(filename, nix_file, show_gui, speed=1.):
    start_tags = []
    end_tags = []
    intervals = []  
    frame_times = []  
    frame_time = 0.0
    temp_times = read_frame_times(filename)
    video = cv2.VideoCapture()
    video.open(filename)

    if temp_times is not None:
        intervals = np.diff(temp_times)*1000.
        intervals = np.hstack((intervals, np.mean(intervals)))
        frame_times = np.asarray(temp_times) * 1000
    else:
        intervals.append(1000//video.get(cv2.cv.CV_CAP_PROP_FPS))

    success, frame = video.read()
    axis = 2
    if success and frame.shape[-1] == 3:
        axis = 3
    k = 0
    while success:
        start = time.time()
        if temp_times is not None:
            frame_time = frame_times[k]
        else:
            frame_time = k * intervals[0]
        write_frame(nix_file, frame, k)
        if show_gui:
            cv2.imshow('frame', frame)
            if frame_times is not None:
                wait_interval = np.max((1, int((intervals[k] - (time.time() - start) * 1000) * speed)))
            else:
                wait_interval = np.max((1, int((intervals[0] - (time.time() - start) * 1000) * speed)))
            key = cv2.waitKey(wait_interval)
            if key & 0xFF == ord('q'):
                break
            elif key & 0xFF == ord('s'):
                start_tags.append(k)
            elif key & 0xFF == ord('e'):
                end_tags.append(k)
        k+=1
        success, frame = video.read()
    
    if temp_times is None:
        frame_times = np.arange(0, k+1)*intervals[0]
    else:
        frame_times = frame_times[0:k+1]
    video.release()
    cv2.destroyAllWindows()
    write_frame_times(nix_file, frame_times)
    check_tags(start_tags, end_tags, k)
    save_tags(nix_file, start_tags, end_tags, frame_times)


def create_nix_file(file_name, frame_size, data_type="nix.stamped_video"):
    nix_file = nix.File.open(file_name, nix.FileMode.Overwrite)
    block_name = file_name.split('/')[-1].split('.')[-2]
    block = nix_file.create_block(block_name, 'recording')
    shape = tuple(list(frame_size)+[1])
    video_data = block.create_data_array("video", data_type, nix.DataType.UInt8, shape)
    sd = video_data.append_sampled_dimension(1.0)
    sd.label = "width"
    sd = video_data.append_sampled_dimension(1.0)
    sd.label = "height"

    if frame_size[-1] == 3:
        dim = video_data.append_set_dimension()
        dim.labels = ["R", "G", "B"]
    else:
        print("warning: unknown channel labels!")
    time_dim = video_data.append_range_dimension([0.0])
    time_dim.label = "time"
    time_dim.unit = "ms"

    tag_positions = block.create_data_array("tag times", "nix.event.positions", nix.DataType.Float, (1, 1))
    tag_positions.append_set_dimension()

    tag_extents = block.create_data_array("tag extents", "nix.event.extents", nix.DataType.Float, (1, 1))
    tag_extents.append_set_dimension()

    tags = block.create_multi_tag("tags", "nix.event", tag_positions)
    tags.extents = tag_extents
    tags.references.append(video_data)
    return nix_file


def write_frame(nix_file, frame, frame_number):
    block = nix_file.blocks[0]
    video_array = None
    for d_a in block.data_arrays:
        if d_a.type == 'nix.stamped_video':
            video_array = d_a
            break;
    if frame_number == 0:
        video_array.data.write_direct(frame)
    else:
        old_shape = list(video_array.data_extent)
        old_shape[-1] += 1
        video_array.data_extent = tuple(old_shape)
        offset = [0] * len(old_shape)
        offset[-1] = frame_number
        temp = frame[..., np.newaxis]
        video_array.data[:,:,:,frame_number] = temp


def write_frame_times(nix_file, frame_times):
    block = nix_file.blocks[0]
    video_array = None
    for d_a in block.data_arrays:
        if d_a.type == 'nix.stamped_video':
            video_array = d_a
            break;
    time_dim = video_array.dimensions[-1]
    if not isinstance(time_dim, nix.RangeDimension):
        raise ValueError("could not find the time dimension!")
    time_dim.ticks = frame_times


def save_tags(nix_file, start_tags, end_tags, frame_times, tag_rois=None):
    if len(start_tags) != len(end_tags):
        raise ValueError('start_tags and end_tags do not have the same number of entries!')

    if tag_rois and tag_rois.shape[-1] != len(end_tags):
        raise ValueError('Number of ROIs does not match the number of tags!')
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


def get_frame_dimensions(filename):
    video = cv2.VideoCapture(filename)
    success, frame = video.read()
    video.release()
    return frame.shape


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='guppy avi to nix converter with tagging option')
    parser.add_argument('file', type=str,
                        help="filename of the file-to-convert")
    parser.add_argument('-g', '--gui', action="store_true",
                        help="with video playback with the option to tag selections")
    parser.add_argument('-o', '--output', type=str, default=None,
                        help="specifies the name of the output file")
    parser.add_argument('-s', '--speed', type=float, default=1.,
                        help="playback speed given as a scaling of the original frame timing e.g. 2.0 "
                             "for double (slow motion), 0.5 for half etc.")
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
                print('TODO: make sure the path exists!')

    frame_size = get_frame_dimensions(args.file)

    nix_file = create_nix_file(output_name, frame_size=frame_size)
    export_avi(args.file, nix_file, args.gui, args.speed)



