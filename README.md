# C- Compiler
This is project is a compiler for the C- programming language that was made in 14 weeks as a part of the University of Idaho's Compiler Design class in the Fall of 2022.
The Compiler generates code for the Tiny Machine (TM) virtual machine.
The source code for the TM is also included in the [src](/src) folder along with the source code for the compiler.

To compile the compiler on a linux system, make sure you have cmake installed, navigate to the [src](/src) folder, and use the "make" command.

To compile the TM on a linux system, make sure you have cmake installed, navigate to the [src](/src) folder, and use the command "make tm".

To use the compiler on a linux system, just create a C- program in a file (preferably ending in ".c-"), then type "./c- \*file name\*", and a ".tm" file with the same name as your program file will appear in your current working directory that can be executed by the Tiny Machine.

To run a ".tm" executable on the Tiny Machine, refer to the documentation on the Tiny Machine in the [doc](/doc) folder.