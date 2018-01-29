### RGBE File Parsing, Updated

Author: Joshua Levine

This repo includes slightly modified code for RGBE file parsing.

The original code is based on [Bruce Walter's Code](http://www.graphics.cornell.edu/~bjw/rgbe.html).

Note, this is modified by:
* Updates from C to C++ (changing the standard header libraries)
* Slightly modifying `RGBE_ReadHeader` so as to parse all of the header information until a blank line is reached.
