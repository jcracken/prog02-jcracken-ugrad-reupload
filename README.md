### Assignment 02

As of right now, the assignment is mostly completed. Some problems with bilinear filtering remain; hard to ascertain the cause. Some things to note:

* When originally starting this assignment, I looked and saw that the load/write code for HDR images would be from third party code, and assumed it would be easier to handle most of the HDR specific functions and work in the main file instead of altering my PPM object code. This was probably a mistake, and there are a few functions in main that should probably be in ppm. Nonetheless, I tried to make it as modular as possible otherwise.

* Usage is: prog02 input output filetype. I didn't make it very clear the last assignment whether to exclude the file extension or not. In that assignment, the user needed to exclude the file extension, but due to the fact that multiple file formats are being used here, the user must *include* the file extension.

* You can alter gamma settings by hitting left and right on the keyboard to decrease and increase gamma by 0.1, respectively. Default gamma is at 1.0.

* You can toggle the bilinear filter by hitting 'b'.

* For the convolution boundary condition, it is handled by boundary padding through image reflection.
