all: bus

CXXFLAGS=-O3 -g -DNDEBUG -lpthread -I ../ -std=c++11
#CXXFLAGS=-O0 --no-inline  -g -lpthread -std=c++11


OBJS= ../nrpa.o 

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< 


bus: bus.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJS)

clean:
	rm -rf *.o same leftMove *~

# non trivial deps. 
nrpa.o: nrpa.cpp nrpa.hpp rollout.hpp rollout.inl policy.hpp nrpa.inl threadpool.hpp
bus.o: bus.cpp
