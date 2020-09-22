checkers.exe: main.o checkers.o checkersDisplay.o
	g++ -o checkers.exe main.o checkers.o checkersDisplay.o

main.o: main.cpp 
	g++ -c main.cpp 

checkers.o: checkers.cpp checkers.h
	g++ -c checkers.cpp checkers.h

checkersDisplay.o: checkersDisplay.cpp checkers.h
	g++ -c checkersDisplay.cpp checkers.h

