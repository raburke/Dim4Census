# README
This repository contains source code related to classifying the census of closed orientable 4-manifolds triangulated with up to six pentachora.

In order to run the code within this repo, you will need `Regina` version 7.4 or higher, or a build from [the Regina git repository](https://github.com/regina-normal/regina).

## Installation Instructions

1. Copy `main.cc`, `boilerplate.h`, and the census into a folder.
3. Open a terminal and navigate to the folder.
4. Type `regina-helper makefile` to build a makefile for your system.
5. Type `make main` to compile the main program.
6. Run the classification algorithm by typing `./main { census file }` where `{ census file }` is the path to one of the census files, e.g. `./main Census/2p-closedOrientable.esig`

Note that this algorithm works with *edge degree isomorphism signatures* and not regular isomorphism signatures. 
All of the census files in this repo have already been converted to edge degree iso sigs, so no conversion is required on these files.
If you need to convert other (standard) isomorphism signature to edge degree signatures, use the `T.isoSig_EdgeDegree()` function in `regina-python`.

Happy classifying!
