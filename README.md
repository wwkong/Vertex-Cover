# CSE 6140 Vertex Cover Algorithms

## Requirements

* A 64-bit Linux environment
* A Clang C++ compiler or GCC (version 5.0 or higher) that is compatible with the C++11 standard
* A Gurobi Optimizer for 64-bit Linux (version 7.5 or higher)

## Compiling Instructions

1. Install the relevant software in the **Requirements** section. The academic version of Gurobi (http://www.gurobi.com/) is sufficient and a license is freely available for students and faculty at public and private universities.

2. Change the variable GRB_HOME in the provided **Makefile** to point to the linux64 folder of your Gurobi installation. By default, the **Makefile** has this set to "/opt/gurobi751/linux64".

3. If you are using a version of Gurobi which is higher than 7.5, change the last library in the CPPLIB variable appropriately. By default this is -lgurobi75 but if you are running version 8.1, for example, this will need to be changed to -lgurobi81.

4. Compile the main executable through the provided **Makefile** in one of two ways:  

 (a) If you are using **Clang**, run the command "make -j1 execClang"  
 (b) If you are using **GCC**, run the command "make -j1 execCpp"

5. Check to see if a binary named **exec** has been generated in the **bin** folder. Note that this code package comes with a copy of this binary for convenience.

## Runtime Instructions

* Move all relevant input graphs into the **input** folder
* An example of a command line call can be found in the **example** folder 
* The general form is:  
```bash
exec -inst <filename> -alg [BnB|Approx|LS1|LS2] -time <cutoff in seconds> -seed <random seed> 
```
* Running the executable in the above manner will write .sol and .trace files in the current directory with the structure:
```
<instance>_<method>_<cutoff>_<randSeed>.[sol|trace]
```
Note that randSeed is only applicable when the method of choice is randomized (e.g. local search). When the method is deterministic (e.g. branch-and-bound), randSeed is omitted from the solution file's name.