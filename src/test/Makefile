CXXFLAGS=-O3 -g -DNDEBUG -lpthread -I ../ -std=c++11
#CXXFLAGS=-O0 --no-inline  -g -lpthread -std=c++11

NRPA_DEPS=../nrpa.hpp ../rollout.hpp ../rollout.inl ../policy.hpp ../threadpool.hpp ../cli.hpp ../nrpa.inl
NRPA_OBJS= ../nrpa.o 





SRCS= algebra.cpp \
	bus.cpp \
	formula.cpp \
	maximum.cpp \
	parity.cpp \
	prime.cpp \
	prisonners.cpp \
	same.cpp \
	serieFinanciere.cpp \
	tsptw.cpp \
	tsptw_stop.cpp \
	ws.cpp

TESTS= $(basename $(SRCS))

all: $(TESTS)

%: %.o $(NRPA_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(NRPA_DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

deps: $(SRCS)
	$(CXX) $(CXXFLAGS) -MM $^

simple-test: $(TESTS) test_driver.sh
	for i in $$(ls test-output/*-simple); do ./test_driver.sh -q $$i ; done 

regen-test:
	@for i in $$(ls test-output/*-simple); do   \
		COM=$$(head -q -n 1 $$i)           ;\
		echo "Regenerating test with $$COM";\
		./test_driver.sh -c -t $$i $$COM      ;\
	done


clean:
	rm -rf *.o *~ $(TESTS)

#deps: generated with make deps
algebra.o: algebra.cpp ../nrpa.hpp ../rollout.hpp ../rollout.inl \
 ../policy.hpp ../threadpool.hpp ../cli.hpp ../nrpa.inl
bus.o: bus.cpp ../nrpa.hpp ../rollout.hpp ../rollout.inl ../policy.hpp \
 ../threadpool.hpp ../cli.hpp ../nrpa.inl
formula.o: formula.cpp ../nrpa.hpp ../rollout.hpp ../rollout.inl \
 ../policy.hpp ../threadpool.hpp ../cli.hpp ../nrpa.inl
maximum.o: maximum.cpp ../nrpa.hpp ../rollout.hpp ../rollout.inl \
 ../policy.hpp ../threadpool.hpp ../cli.hpp ../nrpa.inl
parity.o: parity.cpp ../nrpa.hpp ../rollout.hpp ../rollout.inl \
 ../policy.hpp ../threadpool.hpp ../cli.hpp ../nrpa.inl
prime.o: prime.cpp ../nrpa.hpp ../rollout.hpp ../rollout.inl \
 ../policy.hpp ../threadpool.hpp ../cli.hpp ../nrpa.inl
prisonners.o: prisonners.cpp ../nrpa.hpp ../rollout.hpp ../rollout.inl \
 ../policy.hpp ../threadpool.hpp ../cli.hpp ../nrpa.inl
same.o: same.cpp ../nrpa.hpp ../rollout.hpp ../rollout.inl ../policy.hpp \
 ../threadpool.hpp ../cli.hpp ../nrpa.inl
serieFinanciere.o: serieFinanciere.cpp ../nrpa.hpp ../rollout.hpp \
 ../rollout.inl ../policy.hpp ../threadpool.hpp ../cli.hpp ../nrpa.inl
tsptw.o: tsptw.cpp ../nrpa.hpp ../rollout.hpp ../rollout.inl \
 ../policy.hpp ../threadpool.hpp ../cli.hpp ../nrpa.inl
tsptw_stop.o: tsptw_stop.cpp ../nrpa.hpp ../rollout.hpp ../rollout.inl \
 ../policy.hpp ../threadpool.hpp ../cli.hpp ../nrpa.inl
ws.o: ws.cpp ../nrpa.hpp ../rollout.hpp ../rollout.inl ../policy.hpp \
 ../threadpool.hpp ../cli.hpp ../nrpa.inl
