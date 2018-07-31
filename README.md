# C++ step instrumenter

# Introduction

C++ step instrumenter (cppstepin) tool is designed for instrumenting C++ code with counting of executing operations, or steps. This tool scans source code, computes a number of steps into every statement and inserts the function call, to which it passes a number of executed steps as input parameter. The step can be arithmetic, logical, bitwise expression, function call, memory manage operation, etc. 

# How does it works?

Input data: C++ source code file. 
Output data: Instrumented C++ source code file.

Let we have a code part:

int x = 0, y=0; //2 assignment operations
int z = x + y;  //1 addition and 1 assignment operation
char* p = new char[50]; //1 assignment step, 1 ‘new’ operation 

The instrumenter will insert the following calls:

int x = 0, y=0; 
CLK(2); //2 assignment steps
int z = x + y;  
CLK(2); //1 addition step and 1 assignment step
char* p = new char[50]; 
CLK(2); //1 assignment step, 1 ‘new’ operation step

# Setup parameters

Setup parameters are set via command line parameters. Command line parameters are assigned as 

<Key> [<Value>],                 

where <key> is a name of parameter with key flag (key-incensitive). Key flag is a symbol ‘–‘  or ‘/’ ;
<Value> is a parameter value. Some values are optional, some are necessary.

Example:
Cppstepin.exe /input CSourcecode.cpp /output CSourceCodeInstrumented.cpp

| Name     | Mandatory | Default |Description |
|----------|-----------| ------- |-----------------------------------------------------------------------------------------|
| Input    |     Y     |         | Input file name that is going to be instrumented                                        |
| Output   |           |         | Output file name to which the instrumented code will be written                         |
| I        |           |         | Defines Include directory for compiler                                                  |
| D        |           |         | Pre-processor definition for compiler                                                   |
| Function |           | CLK     | Instrumented function name                                                              |
| Clock    |           |         | Name of the clock file. About clock file read below                                     |
|--------------------------------------------------------------------------------------------------------------------------|





