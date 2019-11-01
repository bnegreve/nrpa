all: leftMove same

CXXFLAGS=-O3 -g -DNDEBUG -lpthread -std=c++11
LDFLAGS=-lpthread
#CXXFLAGS=-O0 --no-inline  -g -lpthread -std=c++11

NRPA_SRCS= nrpa.cpp
NRPA_OBJS= $(patsubst %.cpp, %.o, $(NRPA_SRCS))
OTHER_SRCS= same.cpp leftMove.cpp
OTHER_OBJS= $(patsubst %.cpp, %.o, $(OTHER_SRCS))



.PHONY: test run410 show410 show420 show410

%.o: %.cpp %.hpp 
	$(CXX) $(CXXFLAGS) -o $@ -c $< 

leftMove: leftMove.o $(NRPA_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(NRPA_OBJS) $(LDFLAGS)

same: same.o $(NRPA_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(NRPA_OBJS) $(LDFLAGS)

clean:
	rm -rf *.o same leftMove *~

deps:
	$(CXX) $(CXXFLAGS) -MM $(NRPA_SRCS) $(OTHER_SRCS)

test:
	make -C test simple-test

run410: same
	./same -r 1 -l 4 -n 10 -sS -Tlatest

show410:
	cd plots && ./plot_all.gp && xpdf  pdf/nrpa_stats_level.4_nbIter.10.timer.pdf && cd ../

run420: same
	./same -r 1 -l 4 -n 20 -sS -Tlatest

show420:
	cd plots && ./plot_all.gp && xpdf  pdf/nrpa_stats_level.4_nbIter.20.timer.pdf && cd ../

nrpa.o: nrpa.cpp nrpa.hpp rollout.hpp rollout.inl policy.hpp \
 threadpool.hpp cli.hpp stats.hpp nrpa.inl
same.o: same.cpp nrpa.hpp rollout.hpp rollout.inl policy.hpp \
 threadpool.hpp cli.hpp stats.hpp nrpa.inl
leftMove.o: leftMove.cpp nrpa.hpp rollout.hpp rollout.inl policy.hpp \
 threadpool.hpp cli.hpp stats.hpp nrpa.inl
