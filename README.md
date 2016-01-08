# xenon
The Intrig message decoder.

## Quick Start

    git clone https://github.com/intrig/xenon.git
    cd xenon
    make 
    make test

## Dependencies

    cmake
    C++11 compiler

## Additinal dependencies

These dependencies are not required for xenon, but are for running some tests.

    sudo apt-get install libboost-all-dev

After building, a static library, xenon, can be found in the o directory.

See the examples directory for some common uses of the decoder.

Also, the tools directory contains examples, such as idm and xv.

For your application, you just need the contents of the include directory and link with the xenon library.


