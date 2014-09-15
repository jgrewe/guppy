from __future__ import print_function, division
import nix
import cv2 as cv
import argparse
import os
import numpy as np
from IPython import embed


def export_file(file_name, output_name):
    nix_file = nix.File.open(file_name, nix.FileMode.ReadOnly)
    if not nix_file:
        raise ValueError('Could not open file %s!' %file_name)
    block = nix_file.blocks[0]
    embed()
    #tags = 
    pass


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
