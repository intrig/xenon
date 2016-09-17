#  The Intrig Message Decoder
* 1 [Introduction ](#1)
* 2 [Installing ](#2)
    * 2.1 [For Mac and Linux Systems ](#2.1)
    * 2.2 [Building on Windows ](#2.2)
    * 2.3 [Optional Dependencies ](#2.3)
    * 2.4 [Updating to Latest Release ](#2.4)
* 3 [Using Xenon ](#3)
* 4 [Licensing ](#4)

##<a name="1"/>1 Introduction 


Intrig Xenon provides flexible, powerful and easy to use C++ API for decoding protocol messages.

Originally developed for 3G and 4G programmers and test engineers to decode their complex messages, the xenon decoder
has evolved into a general purpose open source tool with an ever expanding protocol support base.

And, using XDDL, you can support your own proprietary message formats.

See xenon in action for yourself using the Intrig online decoder: <http://intrig.com/x82da86>.

##<a name="2"/>2 Installing 


Xenon is available as source only.  It uses the `cmake` build system and requires a modern C++11 compiler.

###<a name="2.1"/>2.1 For Mac and Linux Systems 


    git clone --recursive https://github.com/intrig/xenon.git
    cd xenon
    make 
    make test
###<a name="2.2"/>2.2 Building on Windows 


Windows requires the following steps:

1. Install [Github Desktop](https://desktop.github.com)

   Clone the xenon repo

2. Install [cmake](https://cmake.org/runningcmake/)

   Follow the instructions in the above link to have cmake generate the Visual Studio build files.

3. Microsoft Visual Studio 2015 (or later)
  
  Load and build the Visual Studio Solution file located at `xenon/build/Program.sln`.

From now on, you can use the github shell to issue git commands.

###<a name="2.3"/>2.3 Optional Dependencies 


Boost dependencies are not required for xenon, but some tests will be skipped if boost cannot be found.

* ubuntu: sudo apt-get install libboost-all-dev
* mac: brew install boost
* Windows: dunno, I just skip it

Tests can be run with

    make test

###<a name="2.4"/>2.4 Updating to Latest Release 


    git pull --recurse-submodules
    git submodule update

##<a name="3"/>3 Using Xenon 


For your application, add the `xenon/include` directory to your include path, and and link with the xenon library found
in the `xenon/o` directory.

See the `xenon/examples' directory for some common uses of the decoder.

Also, the tools directory contains useful examples, such as `idm` and `xv`.

##<a name="4"/>4 Licensing 


Choose from multiple licensing for Xenon:

* **GPL** for open source and internal tools
* **Intrig Commercial License** for distributing with your own software.  This version comes with unlimited technical support.
  Contact support@intrig.com for more on commercial licensing.

Additionally, [The Intrig C++ Toolkit](https://github.com/intrig/ict), a set of powerful C++ types and function that
xenon is built upon, is available under the permissive MIT license.  Check it out!

