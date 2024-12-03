# README

You need `Regina` version 7.4 or higher, or a build from the regina git repository 
to run this code.

Installation instructions:

1. Copy `main.cc`, `boilerplate.h`, and the census into a folder
2. Copy the census files in
2. Open a terminal and navigate to the folder
3. Type in `regina-helper makefile` to build a makefile
4. Type in `make main`
5. Run the classification algorithm by typing `./main`

This will run the classification algorithm on 6-pentachoron triangulations
homeomorphic to 4-spheres.

You can change the census file to run the algorithm on at line 228 of `main.cc`.

Note that this algorithm works with edge degree isomorphism signatures which can be
obtained from a triangulation by performing

`T.isoSig_EdgeDegree()`

in regina-python.
