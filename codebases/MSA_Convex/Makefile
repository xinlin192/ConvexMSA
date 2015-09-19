CC=g++

cpps=$(shell ls *.cpp)
hfiles=$(shell ls *.h)
exec='MSA_Convex'

all: 
	$(CC) -fopenmp -O3 MSA_Convex.cpp  -o $(exec)

push:
	git add $(cpps) $(hfiles) Makefile README
	git commit -m "upgrade $(exec)"
	git push

test: 
	$(CC) -fopenmp -O3 test_CSW.cpp -o test_CSW


clean:
	rm -rf $(exec) test_CSW
