#  The Intrig Message Decoder
* 1 [Introduction ](#1)
    * 1.1 [Quick Start ](#1.1)
        * 1.1.1 [Building on Windows ](#1.1.1)
    * 1.2 [Dependencies ](#1.2)
        * 1.2.1 [Optional Dependencies ](#1.2.1)
    * 1.3 [Updating an existing repo to latest ](#1.3)
* 2 [Using Xenon ](#2)
* 3 [Licensing ](#3)

##<a name="1"/>1 Introduction 

Intrig Xenon provides flexible, powerful and easy to use C++ API for decoding protocol messages.

Originally developed for 3G and 4G programmers and test engineers to decode their complex messages, the xenon decoder
has evolved into a general purpose open source tool with an ever expanding protocol support base.

And, using XDDL, you can support your own proprietary message formats.

See xenon in action for yourself using the Intrig online decoder: [http://intrig.com/x82da86](intrig.com).

###<a name="1.1"/>1.1 Quick Start 


    git clone --recursive https://github.com/intrig/xenon.git
    cd xenon
    make 
    make test

####<a name="1.1.1"/>1.1.1 Building on Windows 


Windows requires the following steps:

1. Install [Github Desktop](https://desktop.github.com)

   Clone the xenon repo.

2. Install [cmake](https://cmake.org/runningcmake/)

   Follow the instructions in the above link to have cmake generate the Visual Studio build files.

3. Microsoft Visual Studio 2015 (or later)
  
  Load and build the Visual Studio Solution file located at `xenon/build/Program.sln`.

###<a name="1.2"/>1.2 Dependencies 


* cmake
* C++11 compiler
* Github desktop (for Windows only)

####<a name="1.2.1"/>1.2.1 Optional Dependencies 


Boost dependencies are not required for xenon, but some tests tests will be skipped if boost cannot be found.

* ubuntu: sudo apt-get install libboost-all-dev
* mac: brew install boost
* Windows: dunno, I just skip it

Tests can be run with

    make test

###<a name="1.3"/>1.3 Updating an existing repo to latest 


    git pull
    git submodule update

##<a name="2"/>2 Using Xenon 


For your application, add the `xenon/include` directory to your include path, and and link with the xenon library found
in the `xenon/o` directory.

See the `xenon/examples' directory for some common uses of the decoder.

Also, the tools directory contains useful examples, such as `idm` and xv.

##<a name="3"/>3 Licensing 


Choose from multiple licensing for Xenon:

* **GPL** for open source and internal tools
* **Intrig Commercial License** for distributing with your own software.  This version comes with unlimited support.
  Contact support@intrig.com for more on commercial licensing.

Additionally, [The Intrig C++ Toolkit](https://github.com/intrig/ict), a set of powerful C++ types and function that
xenon is built upon, is available under the permissive MIT license.  Check it out!

