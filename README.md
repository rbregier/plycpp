
plycpp: a simple C++ library to import/export Polygon File Format data.
=============

This library intends to provide an easy to use solution to manipulate PLY files in C++.

If you are looking for a more complete alternative and are not interested by C++ syntactic sugar, you may want to have a look at:
* ply_io, the C library developped by Greg Turk (http://people.sc.fsu.edu/~jburkardt/c_src/plycpp/plycpp.html)
* RPly, an other C library developed by Diego Nehab (http://w3.impa.br/~diego/software/rply/)

Features
------------
* Easy to install: Add "hdr/plycpp.h" and "src/plycpp.cpp" to your project and you are ready to go (or use CMake and a Git submodule if you prefer).
* Load PLY files in ASCII and Binary mode.
* Save PLY data in ASCII and Binary mode.
* Handle arbitrary elements and properties.
* Safety mechanisms to check data type in Debug mode.
* ParsingException triggered if anything goes wrong.


Syntax example
----------

See [src/example.cpp](src/example.cpp) for a full example.


Current limitations
-------
* Property lists have to contain exactly 3 values per element, and be indexed by a "uchar" type. For typical use, this means that __only triangular meshes are supported__.
* Decoding of binary files encoded with an endianness different of the one of the current architecture is not yet implemented.

Compilers supported
---------
This code should be supported by any C++11 compliant compiler.
Tested with Visual Studio 2013 and Visual Studio 2017.