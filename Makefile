# list of test drivers (with main()) for development
TESTSOURCES = $(wildcard test*.cpp)
# names of test executables
TESTS       = $(TESTSOURCES:%.cpp=%)

# list of sources used in project
SOURCES 	= $(wildcard *.cpp)
SOURCES     := $(filter-out $(TESTSOURCES), $(SOURCES))
# list of objects used in project
OBJECTS		= $(SOURCES:%.cpp=%.o)

SO_PATH = $(LD_LIBRARY_PATH)

LIB = dafka.so
RPC_LIB = drpc.so

#Default Flags
CXXFLAGS = -std=c++14 -Wconversion -Wall -Werror -Wextra -pedantic -pthread

# make debug - will compile "all" with $(CXXFLAGS) and the -g flag
#              also defines DEBUG so that "#ifdef DEBUG /*...*/ #endif" works
debug: CXXFLAGS += -g3 -DDEBUG
debug: clean all

# highest target; sews together all objects into executable
all: $(LIB) test_client

final: clean $(LIB)
	ln -f $(LIB) $(SO_PATH)

$(LIB): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(LIB) $(SO_PATH)/$(RPC_LIB) -shared

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) $(TESTS) $(PARTIAL_SUBMITFILE) $(FULL_SUBMITFILE) *.out test_client

headers:
	cp ../drpc/drpc.h .
	cp ../channel/Channel.h .

# test1: test1.cpp $(LIB)
# 	$(CXX) $(CXXFLAGS) -o $@ $^
test_basic: test_basic.cpp $(LIB) $(SO_PATH)/$(RPC_LIB) 
	$(CXX) $(CXXFLAGS) -o $@ $^

test_client: test_client.cpp $(LIB) $(SO_PATH)/$(RPC_LIB) 
	$(CXX) $(CXXFLAGS) -o $@ $^

# rule for creating objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -g -c $*.cpp