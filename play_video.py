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


def findVideoArrayID(block):
    for a in block.data_arrays:
        if "nix.stamped_video" in a.type:
            return a.id
    return None


def getTagPositions(block):
    tag = None
    for t in block.multi_tags:
        if "nix.event" in t.type:
            tag = t
    if not tag:
        return None

    tag_times = tag.positions.data[tag.positions.data.shape[0]-1,:]
    return tag_times


def createTagMask(height, width, channels):
    mask = np.zeros((height, width, channels))
    cv2.rectangle(mask, (2,2), (width-2, height-2), (0, 255, 0), 4)
    return mask


def getTagFrames(ticks, positions):
    tag_on = np.zeros((len(ticks)))
    if positions is not None:
        for p in positions:
            temp = abs(ticks -p)
            tag_on[(temp) == min(temp)] = 1
    return tag_on


def playback(block, array=None):
    if not array:
        array = findVideoArrayID(block)
    if not array or not array in block.data_arrays:
        print('DataArray does not exit!')
        exit()

    da = block.data_arrays[array]
    dim = da.dimensions[-1]
    if not isinstance(dim, nix.core.RangeDimension):
        print("Expected last dimension to be a RangeDimension!")
        exit()
    ticks = dim.ticks
    intervals = np.diff(ticks)
    intervals = np.hstack((intervals, np.mean(intervals)))

    height, width, channels, nframes = da.data.shape

    tag_mask = createTagMask(height, width, channels)
    no_tag_mask = np.zeros((height, width, channels))
    tag_positions = getTagPositions(block);
    tag_on = getTagFrames(ticks, tag_positions)

    tag_fader = 0
    for k in range(nframes):
        start = time.time()
        frame = da.data[:, :, :, k]
        frame = frame/256.
        if tag_on[k] == 1:
            tag_fader = 3
            img = cv2.add(frame,tag_mask)
        else:
            if tag_fader > 0:
                img = cv2.add(frame, tag_mask)
            img = cv2.add(frame, no_tag_mask)
        #gray = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
        cv2.imshow('frame', img)
        wait_interval = np.max((1, int(intervals[k] - (time.time() - start)*1000)))
        if cv2.waitKey(wait_interval) & 0xFF == ord('q'):
            break


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='guppy playback')
    parser.add_argument('file', type=str, help='the nix file containing the video')
    parser.add_argument('-a', '--array', type=str, default=None, 
                        help='the id of the data array, first video block will be taken if omitted')
    parser.add_argument('-b', '--block', type=str, default=None,
                        help='the block containing the video, fist block will be selected if omitted')
    args = parser.parse_args()

    if not os.path.exists(args.file):
        print('File does not exits!')
        exit()

    nf = nix.File.open(args.file, nix.FileMode.ReadOnly)
    block = nf.blocks[args.block or 0]
    playback(block, args.array)
    cv2.destroyAllWindows()
