About guppy
----------- 

This is a small video recorder tool that stores videos grabbed from
open CV devices either as avi of to the nix data model [see NIX
project pages for details](https://github.com/G-Node/nix).  Note: the
tool does not record audio data. So far it is more or less a testing
platform for the nix-io. Do not expect too much!

Build Dependencies
------------------

In order to build program a recent C++11 compatible compiler is needed
(g++ 4.8 tested). Further guppy depends on the following third party
libraries: 

- openCV2
- HDF5 (version 1.8 or higher)
- nix 
- Boost

Note: so far, use the Makefile to build it

Usage
-----

After successful building call it from the command line:

> ./guppy-gui

To call it using nix-io

> ./guppy-gui --nix-io true

For command line options call:

> ./guppy-gui --help

When you press the space-bar the recording starts, pressing space
again stops the recording. When using avi output (default) two files
are written one is the avi, the other a text file containing the times
the frames were grabbed. When using nix-io, the data is stored
uncompressed in a nix hdf5 file. When using nix, you can save the
times of certain events by pressing "t" at any time.

End the program by pressing "ESC".

Note: upon restart, existing data files from the same day are
overwritten!
