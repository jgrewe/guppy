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

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='guppy playback')
    parser.add_argument('file', type=str)
    parser.add_argument('array', type=str)
    parser.add_argument('--block', type=str, default=None)
    args = parser.parse_args()

    if not os.path.exists(args.file):
        print('File does not exits!')
        exit()

    nf = nix.File.open(args.file, nix.FileMode.ReadOnly)
    block = nf.blocks[args.block or 0]
    if not args.array in block.data_arrays:
        print('DataArray does not exit!')
        exit()

    da = block.data_arrays[args.array]
    dim = da.dimensions[-1]
    if not isinstance(dim, nix.core.RangeDimension):
        print("Expected last dimension to be a RangeDimension!")
        exit()
    ticks = dim.ticks
    intervals = np.diff(ticks)
    intervals = np.hstack((intervals, np.mean(intervals)))

    height, width, channel, nframes = da.data.shape
    begin = time.time()
    for k in range(nframes):
        start = time.time()
        frame = da.data[:, :, :, k]
        frame = frame/256.
        #gray = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
        cv2.imshow('frame', frame)
        wait_interval = np.max((1, int(intervals[k] - (time.time() - start)*1000)))
        if cv2.waitKey(wait_interval) & 0xFF == ord('q'):
            break
    done = time.time()
    print(done-begin)

    cv2.destroyAllWindows()
