### koncept4 - alarm clock

This is a program intended to serve as a more advanced alarm clock for a Windows PC - to play sounds and shut
down the machine at specific times, based on the configuration file and console commands. It consists of 2 programs
- konc4d running in the background and konc4 - a console allowing the user to give commands to konc4d.

Compiling and running tests is done via Make - `make all` compiles the program, `make test` compiles and runs
all unit tests, `make test_integration` compiles and runs all integration tests. Compilation is done via MinGW
version of GCC.

The project is written mostly in C, using only the C and C++ standard libraries and the Win32 API.
