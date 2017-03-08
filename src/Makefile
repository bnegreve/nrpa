all: leftMove same

CXXFLAGS=-O3 -g -DNDEBUG
#CXXFLAGS=-O0 --no-inline  -g


OBJS= nrpa.o 

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< 

leftMove: leftMove.o $(OBJS) 
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJS)  

same: same.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJS)

clean:
	rm -rf *.o same leftMove *~

# non trivial deps. 
nrpa.o: nrpa.cpp nrpa.hpp rollout.hpp rollout.inl policy.hpp nrpa.inl


