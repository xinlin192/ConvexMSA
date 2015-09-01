cpps=$(shell ls *.cpp)
exec='PSA_DP'

all: 
	g++ -fopenmp -O3 $(cpps) -o $(exec)

push:
	git add $(cpps) Makefile *.seq
	git commit -m "upgrade $(exec)"
	git push

clean:
	rm -rf PSA_NAIVE $(exec)
