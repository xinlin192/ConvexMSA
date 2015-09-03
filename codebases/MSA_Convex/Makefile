CC=g++

cpps=$(shell ls *.cpp)
hfiles=$(shell ls *.h)
exec='MSA_Convex'

all: 
	$(CC) -fopenmp -O3 $(cpps) $(hfiles) -o $(exec)

push:
	git add $(cpps) Makefile
	git commit -m "upgrade $(exec)"
	git push

clean:
	rm -rf $(exec)
