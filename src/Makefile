all: leftMove same

#CXXFLAGS=-O3 -g
CXXFLAGS=-O0 -g


OBJS=rollout.o

%.o: %.cpp %.hpp
	$(CC) $(CXXFLAGS) -o $@ -c $< 

leftMove: leftMove.cpp nrpa.cpp $(OBJS) 
	g++ $(CXXFLAGS) -o $@ $< $(OBJS)  

same: same.cpp nrpa.cpp $(OBJS)
	g++ $(CXXFLAGS) -o $@ $< $(OBJS)
