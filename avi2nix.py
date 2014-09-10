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


def play_avi(filename, time_scale):
    frame_times = read_frame_times(filename)
    video = cv2.VideoCapture()
    video.open(filename)
    begin = time.time()
    intervals = []
    if frame_times:
        intervals = np.diff(frame_times)*1000.
        intervals = np.hstack((intervals, np.mean(intervals)))
    else:
        intervals.append(1000//video.get(cv2.cv.CV_CAP_PROP_FPS))
    success, frame = video.read()
    k = 0
    while success:
        start = time.time()
        cv2.imshow('frame', frame)
        wait_interval = np.max((1, int((intervals[k] - (time.time() - start) * 1000) * time_scale)))
        if cv2.waitKey(wait_interval) & 0xFF == ord('q'):
            break
        if frame_times:
            k+=1
        success, frame = video.read()
    video.release()
    cv2.destroyAllWindows()
    

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='guppy avi to nix converter with tagging option')
    parser.add_argument('file', type=str, help="filename of the file-to-convert")
    parser.add_argument('-g', '--gui', action="store_true", help="just converts, no display, no tagging")
    parser.add_argument('-o', '--output', type=str, default=None, help="specifies the name of the output file")
    parser.add_argument('-s','--speed', type=float, default=1.,
                        help="playback speed given as a scaling of the original framerate e.g. 2.0 for double, 0.5 for half etc.")
    args = parser.parse_args()

    if not os.path.exists(args.file):
        print('File does not exits!')
        exit()
    
    if args.gui:
        play_avi(args.file, args.speed)
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

