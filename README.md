polyworld-modified
==================

This repo is a github version taken from Binh Nguyen's excellent windows modifications of the "newer" polyworld project. 

The original link I downloaded this source from was here: 

https://docs.google.com/leaf?id=0B5GXtJevZpAMYzk1YjAyMjQtOWEyZS00YzM3LWE5MzktYjllMWExNTM1NjUw&hl=en

## Original Instructions on how to build from Binh Nguyen's blog:

http://tb-nguyen.blogspot.com/2010/05/how-to-build-polyworld-for-windows.html 

MONDAY, 17 MAY 2010

### How to Build Polyworld for Windows
1.  Get Qt and MinGW

    Download qt-sdk-win-opensource-2010.02.1.exe   
    Install to d:\work\tools\qt

2.  Get GSL for Windows

    Download gsl-1.8-bin.zip and gsl-1.8-lib.zip files
    Extract to d:\work\tools\gnu32

3.  Get CVS

    Download cvs-1-11-22.zip
    Extract to d:\work\tools\cvs

4.  Get Boost

    Download boost_1_43_0.zip
    Extract to d:\work\tools\boost

5.  Start the Qt Command Prompt

    start > all programs > qt sdk by nokia v2010.02.1 (open source) > qt command prompt

6.  Set up your path

    path=%path%;d:\work\tools\gnuwin32\bin;d:\work\tools\cvs

7.  Download polyworld

    cvs -d:pserver:anonymous@polyworld.cvs.sf.net:/cvsroot/polyworld checkout -P polyworld
    Copy to d:\work\polyworld

8.  Get a Project File and a World File

    Dowload polyworld-modified.zip
    Extract to d:\work\polyworld-modified
    Copy d:\work\polyworld-modified\polyworld.pro to d:\work\polyworld
    Copy d:\work\polyworld-modified\debug\worldfile to d:\work\polyworld\debug

9.  Genrate a Makefile

    Go to d:\work\polyworld
    Double check that all the directories in polyworld.pro are correct for you.
    qmake

10. Edit the souce code

    See instructions below

11. Build

    Go to d:\work\polyworld
    mingw32-make

### Edit the source code

1.  Some header files are in a different place for MinGW

    Look for the files in d:\work\tools\qt\2010.02.1\mingw

2.  Some header files are not available for MinGW

    Put a #ifdef linux and #endif around them.

    Some functions are not available for MinGW

    MinGW does not have drand48().
    Instead use double( rand() ) / double( RAND_MAX )

    MinGW does not have alloca().
    Instead use boost shared arrays

    For the rest, put a #ifdef linux and #endif around them.
    There are some big chunks where you'll have to do this but you won't affect the simulation too much.

    There are some functions for file writing that won't work because Polyworld uses a few commands only available on Linux and Mac. Use the worldfile in polyworld-modified.zip to disable most of them.

4.  Some functions will cause a crash

    There are a few lines in the genome::crossover() that will cause a crash.

5.  There are a few more small changes you'll need to make but I hope this guide has helped you with most of the big ones.

### Notes
* If you have trouble modifying the code, I've included the modifications in polyworld-modified.zip

* If you have trouble building the code, I've included polyworld.exe and all of the dependencies in polyworld-modified.zip

* Use the dynamic link libraries in d:\work\tools\qt\2010.02.1\qt\bin and not d:\work\tools\qt\2010.02.1\bin

### References
* http://www.beanblossom.in.us/larryy/BuildingPolyworld.html
* http://wiki.yak.net/634
* http://wiki.yak.net/630