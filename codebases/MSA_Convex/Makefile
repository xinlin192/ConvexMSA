CC=g++
CFLAGS='-O3 -fopenmp'

cpps=$(shell ls *.cpp)
exec='MSA_Convex'

all: 
	$(CC) $(CFLAGS) $(cpps) -o $(exec)

push:
	git add $(cpps) Makefile
	git commit -m "upgrade $(exec)"
	git push

clean:
	rm -rf $(exec)
