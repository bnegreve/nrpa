all: leftMove same

CXXFLAGS=-O3 -g
#CXXFLAGS=-O0 --no-inline  -g


OBJS=rollout.o nrpa.o movemap.cpp

%.o: %.cpp %.hpp
	$(CC) $(CXXFLAGS) -o $@ -c $< 

leftMove: leftMove.cpp nrpa_inc.cpp $(OBJS) 
	g++ $(CXXFLAGS) -o $@ $< $(OBJS)  

same: same.cpp nrpa_inc.cpp $(OBJS)
	g++ $(CXXFLAGS) -o $@ $< $(OBJS)

leftMove2: leftMove2.cpp nrpa.hpp $(OBJS) 
	g++ $(CXXFLAGS) -o $@ $< $(OBJS)  
