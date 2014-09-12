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
    frames = []
    if frame_times:
        intervals = np.diff(frame_times)*1000.
        intervals = np.hstack((intervals, np.mean(intervals)))
    else:
        intervals.append(1000//video.get(cv2.cv.CV_CAP_PROP_FPS))
    success, frame = video.read()
    frames.append(frame)
    k = 0
    while success:
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
            print('start: ' + str(k))
        elif key & 0xFF == ord('e'):
            end_tags.append(k)
            print('end: ' + str(k))
        k+=1
        success, frame = video.read()
        frames.append(frame)
    
    video.release()
    cv2.destroyAllWindows()
    check_tags(start_tags, end_tags, k)
    return start_tags, end_tags, frames, frame_times


def grab_frames(filename):
    start_tags = [0]
    end_tags = []
    frame_times =  []
    frames = []
    video = cv2.VideoCapture(filename)
    frame_time = 1000//video.get(cv2.cv.CV_CAP_PROP_FPS)
    success, frame = video.read()
    frame_count = 0
    while success:
        frames.append(frame) 
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


def save_frames(frames, frame_times):

    pass


def save_tags(start_tags, end_tags, frames_times):

    pass


def nix_export(file_name, start_tags, end_tags, frame_times, frames) :
    nix_file = create_nix_file(file_name)
    save_frames(frames, frame_times)
    save_tags(start_tags, end_tags, frame_times)
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
        ans = raw_input('Output file %s already exists! Overwrite? [y/n/c]: ' % output_name)
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


#
#    nf = nix.File.open(args.file, nix.FileMode.ReadOnly)
#    block = nf.blocks[args.block or 0]
#    if not args.array:
#        args.array = findVideoArrayID(block)
#    if not args.array or not args.array in block.data_arrays:
#        print('DataArray does not exit!')
#        exit()#
#
#    da = block.data_arrays[args.array]
#    dim = da.dimensions[-1]
#    if not isinstance(dim, nix.core.RangeDimension):
#        print("Expected last dimension to be a RangeDimension!")
#        exit()
#    ticks = dim.ticks
#    intervals = np.diff(ticks)
#    intervals = np.hstack((intervals, np.mean(intervals)))##
#
#    height, width, channels, nframes = da.data.shape##
#
#    tag_mask = createTagMask(height, width, channels)
#    no_tag_mask = np.zeros((height, width, channels))
#    tag_positions = getTagPositions(block);
#    tag_on = getTagFrames(ticks, tag_positions)

