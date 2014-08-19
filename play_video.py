#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import print_function, division

import nix
import cv2
import argparse


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='guppy playback')
    parser.add_argument('file', type=str)
    parser.add_argument('array', type=str)
    parser.add_argument('--block', type=str, default=None)
    args = parser.parse_args()

    nf = nix.File.open(args.file, nix.FileMode.ReadOnly)
    block = nf.blocks[args.block or 0]
    da = block.data_arrays[args.array]

    height, width, channel, nframes = da.data.shape

    for k in range(nframes):
        frame = da.data[:, :, :, k]
        #gray = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
        cv2.imshow('frame', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cv2.destroyAllWindows()