cpps=$(shell ls *.cpp)
exec='PSA_CUBE'

all: 
	g++ -fopenmp -O3 $(cpps) -o $(exec)

push:
	git add $(cpps) Makefile
	git commit -m "upgrade $(exec)"
	git push

clean:
	rm -rf $(exec)
