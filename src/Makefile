all: leftMove same

CXXFLAGS=-O3 -g -DNDEBUG -lpthread -std=c++11
#CXXFLAGS=-O0 --no-inline  -g -lpthread -std=c++11


OBJS= nrpa.o 

.PHONY: test run410 show410 show420 show410

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< 

leftMove: leftMove.o $(OBJS) 
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJS)  

same: same.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJS)

clean:
	rm -rf *.o same leftMove *~

test:
	make -C test simple-test

run410: 
	./same -r 1 -l 4 -n 10 -sS -Tlatest

show410:
	cd plots && ./plot_all.gp && xpdf  pdf/nrpa_stats_level.4_nbIter.10.timer.pdf && cd ../

run420:
	./same -r 1 -l 4 -n 20 -sS -Tlatest

show420:
	cd plots && ./plot_all.gp && xpdf  pdf/nrpa_stats_level.4_nbIter.20.timer.pdf && cd ../


# non trivial deps. 
nrpa.o: nrpa.cpp nrpa.hpp stats.hpp rollout.hpp rollout.inl policy.hpp nrpa.inl threadpool.hpp 


