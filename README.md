# C++ step instrumenter

# Introduction

C++ step instrumenter (cppstepin) tool is designed for instrumenting C++ code with counting of executing operations, or steps. This tool scans source code, computes a number of steps into every statement and inserts the function call, to which it passes a number of executed steps as input parameter. The step can be arithmetic, logical, bitwise expression, function call, memory manage operation, etc. 

# How does it works?

Input data: C++ source code file. 
Output data: Instrumented C++ source code file.

Let we have a code part:

```
int x = 0, y=0; //2 assignment operations
int z = x + y;  //1 addition and 1 assignment operation
char* p = new char[50]; //1 assignment step, 1 ‘new’ operation 
```

The instrumenter will insert the following calls:

```
int x = 0, y=0; 
CLK(2); //2 assignment steps
int z = x + y;  
CLK(2); //1 addition step and 1 assignment step
char* p = new char[50]; 
CLK(2); //1 assignment step, 1 ‘new’ operation step
```

# Setup parameters

Setup parameters are set via command line parameters. Command line parameters are assigned as set of pairs

<Key> <Value>,                 

where <key> is a name of parameter with key flag (key-incensitive). Key flag is a symbol ‘–‘  or ‘/’ ;
<Value> is a parameter value. Some values are optional, some are necessary.

Example:
Cppstepin.exe /input CSourcecode.cpp /output CSourceCodeInstrumented.cpp

| Name     | Mandatory | Default |Description |
|----------|-----------| ------- |-----------------------------------------------------------------------------------------|
| Input    |     Y     |         | Input file name that is going to be instrumented                                        |
| Output   |           |         | Output file name to which the instrumented code will be written. If this parameter is omitted, the input file will be overwritten with instrumented file.                          |
| I        |           |         | Defines Include directory for compiler                                                  |
| D        |           |         | Pre-processor definition for compiler                                                   |
| Function |           | CLK     | Instrumented function name                                                              |
| Clock    |           |         | Name of the clock file. About clock file read below                                     |
| Include  |           |         | File name for directive “include” that will be added to the instrumenting file          |
| IncludeStd|          |         | This parameter is related with ‘include’ parameter and points, that include file name must be framed with <>, not with quotes|
| Extern   |           |         | Additional “extern” definition that will be added to the instrumenting file             |
| Step     |           | 1       | A number of steps after that instrumenting function call will be injected into source code|
|Statement |           | 1       | A number of statements after that instrumenting function call will be injected into source code|
|--------------------------------------------------------------------------------------------------------------------------|

#Installation

1.	Install clang  http://clang.llvm.org/. 
The application was developped with LLVM 6.0.1. As LLVM has not stable interface, on the later version the application might not be compiled. Write me at that case – I will correct the code.
2.	In CMakeLists.txt, correct paths for LLVM.
3.	Set cppstepin as active directory.
4.	Run project generation.  
For Windows, run CMake –G “Visual studio 15” ./src. It generates solution for Visual Studio. Open cppstepin.sln and compile the project.
For Linux, cmake -G "Unix Makefiles" ./src. After that, run make.




