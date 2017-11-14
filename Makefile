GRB_HOME = /opt/gurobi751/linux64
PLATFORM = linux64
INC      = -I$(GRB_HOME)/include/
CPPLIB   = -L$(GRB_HOME)/lib/ -lgurobi_c++ -lgurobi75
CPP      = g++ #clang++
# New ABI in GCC 5.0 needs to be disabled to compile with Gurobi
CARGS    = -std=c++11 -O2 -m64 -g -D_GLIBCXX_USE_CXX11_ABI=0
LIB = -lrt
CPPFLAGS   = -O2 -std=c++11 -m64 -g -D_GLIBCXX_USE_CXX11_ABI=0
SOURCES_DIR = src
SOURCES  = graph.cpp parseGraph.cpp approx.cpp exec.cpp 
OBJS_DIR = bin
OBJS += $(patsubst %.cpp, $(OBJS_DIR)/%.o, $(SOURCES))
EXEC = exec

#exec:
#	clang++ $(CARGS) -Idir ./src/exec.cpp -o ./bin/$@ $< $(INC) $(CPPLIB) -lm
#execRaw:
#	clang++ -Idir ./src/exec.cpp -o ./bin/exec
#parseGraph:
#	clang++ -Idir ./src/parseGraph.cpp -o ./bin/$@ $<
#vcLpSolve:
#	clang++ $(CARGS) -Idir ./src/vcLpSolve.cpp -o ./bin/$@ $< $(INC) $(CPPLIB) -lm
#branchAndBound:
#	clang++ $(CARGS) -Idir ./src/branchAndBound.cpp -o ./bin/$@ $< $(INC) $(CPPLIB) -lm

all: dir $(EXEC)
dir:
	-mkdir -p $(OBJS_DIR)

$(EXEC): $(OBJS)
	$(CPP) $(CPPFLAGS) $(LIB) $(OBJS) -o $(EXEC)

$(OBJS_DIR)/%.o: $(SOURCES_DIR)/%.cpp
	$(CPP) $(CPPFLAGS) $(LIB) -o $@ -c $<

clean:
	-rm -rf $(OBJS)


