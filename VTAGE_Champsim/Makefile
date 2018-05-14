CC = g++
OPT = -O3
LIBS = -lcvp -lboost_iostreams -lz
FLAGS = -std=c++11 -L. -L/home/anjana/ValuePredictionChampSim/lib/lib/ $(LIBS) $(OPT) -static -g 
OBJ = vtage.o
DEPS = cvp.h vtage.h

all: cvp

cvp: $(OBJ)
	$(CC) $(FLAGS) -o $@ $^

%.o: %.cc $(DEPS)
	$(CC) $(FLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f *.o cvp
