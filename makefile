hw4: hw4.o SetupData.o Log.o
	g++ hw4.o SetupData.o Log.o -lpthread -o hw4
hw4.o: hw4.cc SetupData.h
	g++ -c hw4.cc
SetupData.o: SetupData.cc SetupData.h Log.h
	g++ -c SetupData.cc
Log.o: Log.cc Log.h
	g++ -c Log.cc
clean:
	rm *.o
	rm -f hw4
cmake:
	make clean
	make
