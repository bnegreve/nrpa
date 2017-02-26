all: leftMove same

CXXFLAGS=-O3 -g -DNDEBUG
#CXXFLAGS=-O0 --no-inline  -g


OBJS=rollout.o nrpa.o movemap.o

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< 

leftMove: leftMove.cpp nrpa.hpp $(OBJS) 
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJS)  

same: same.cpp nrpa.hpp $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJS)

clean:
	rm -rf *.o same leftMove *~

