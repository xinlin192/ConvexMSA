cpps=$(shell ls *.cpp)
exec='PSA_NAIVE'

all: 
	g++ -fopenmp -O3 $(cpps) -o PSA_NAIVE

push:
	git add $(cpps) Makefile 
	git commit -m "upgrade"
	git push

clean:
	rm -rf PSA_NAIVE $(exec)
