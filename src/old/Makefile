all: leftMove_old same_old

CXXFLAGS=-O3 -g -DNDEBUG
#CXXFLAGS=-O0 --no-inline  -g

leftMove_old: leftMove.cpp nrpa.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

same_old: same.cpp nrpa.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -rf *.o same leftMove *~

