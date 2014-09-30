from __future__ import print_function, division
import nix
import cv2 as cv
import argparse
import os
import numpy as np
from IPython import embed


def write_selections(tag, output_name):
    pos = tag.positions.data
    extents = tag.extents.data
    pos_data = np.empty(pos.shape)
    pos.read_direct(pos_data)
    extent_data = np.empty(extents.shape)
    extents.read_direct(extent_data) 
    frame_array = tag.references[0]
    frame_data = frame_array.data 
    frames = np.empty(frame_data.shape)
    frame_data.read_direct(frames)
    frame_times = frame_array.dimensions[-1].ticks
    frame_rate = 1000/np.mean(np.diff(frame_times))

    selection = None
    for i in range(pos_data.shape[1]):
        p = pos_data[:,i]
        e = pos_data[:,i]
        if len(frames) == 4:
            selection = frames[:, :, :, (np.all((frame_times >= p[-1], frame_times < p[-1]+e[-1]), axis=0))]
        elif len(frames) == 3:
            selection = frames[:, :, (np.all((frame_times >= p[-1], frame_times < p[-1]+e[-1]), axis=0))]
        filename = output_name.split('.')[-2] + '_' + str(i) + '.' +  output_name.split('.')[-1]
        write_frames_to_avi(selection, frame_rate, filename)


def write_frames_to_avi(frames, frame_rate, output_name):
    print("write selection: " + output_name)
    pass


def export_file(file_name, output_name):
    nix_file = nix.File.open(file_name, nix.FileMode.ReadOnly)
    if not nix_file:
        raise ValueError('Could not open file %s!' %file_name)
    block = nix_file.blocks[0]
    tag = block.multi_tags[0]
    write_selections(tag, output_name)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='guppy avi to nix converter with tagging option')
    parser.add_argument('file', type=str, help="filename of the file-to-convert")
    parser.add_argument('-o', '--output', type=str, default=None, help="specifies the name pattern of the output file(s)")

    args = parser.parse_args()
    
    if not os.path.exists(args.file):
        print('File does not exits!')
        exit()
    
    output_name = arg.output if args.output else args.file.split('.')[-2] + '_export.h5'
    if os.path.exists(output_name):
        ans = raw_input('Output file(s) %s already exist(s)! Overwrite? ([y]/n/c): ' % output_name)
        if ans == 'c':
            print('\tExport cancelled!')
            exit()
        elif ans == 'n':
            output_name = raw_input('Please give a new file name: ')
            if '/' in output_name or '\\' in output_name:
                print ('TODO: make sure the path exists!')
    
    export_file(args.file, output_name)
