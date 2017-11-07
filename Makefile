PLATFORM = linux64
INC      = -I/opt/gurobi751/linux64/include/
CPP      = clang++
CPPLIB   = -L/opt/gurobi751/linux64/lib/ -lgurobi_c++ -lgurobi75
# New ABI in GCC 5.0 needs to be disabled to compile with Gurobi
CARGS    = -m64 -g -D_GLIBCXX_USE_CXX11_ABI=0

exec:
	clang++ $(CARGS) -Idir ./src/exec.cpp -o ./bin/$@ $< $(INC) $(CPPLIB) -lm
execRaw:
	clang++ -Idir ./src/exec.cpp -o ./bin/exec
parseGraph:
	clang++ -Idir ./src/parseGraph.cpp -o ./bin/$@ $<
vcLpSolve:
	clang++ $(CARGS) -Idir ./src/vcLpSolve.cpp -o ./bin/$@ $< $(INC) $(CPPLIB) -lm
branchAndBound:
	clang++ $(CARGS) -Idir ./src/branchAndBound.cpp -o ./bin/$@ $< $(INC) $(CPPLIB) -lm
