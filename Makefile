PLATFORM = linux64
INC      = -I/opt/gurobi751/linux64/include/
CPP      = clang++
CPPLIB   = -L/opt/gurobi751/linux64/lib/ -lgurobi_c++ -lgurobi75
# New ABI in GCC 5.0 needs to be disabled to compile with Gurobi
CARGS    = -m64 -g -D_GLIBCXX_USE_CXX11_ABI=0

execRaw:
	clang++ -Idir ./src/exec.cpp -o ./bin/exec
parseGraph:
	clang++ -Idir ./src/parseGraph.cpp -o ./bin/$@ $<
vcLpSolve:
	clang++ $(CARGS) -Idir -o ./bin/$@ $< ./src/vcLpSolve.cpp $(INC) $(CPPLIB) -lm
