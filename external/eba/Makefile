# Variables

CXX = g++
CXXFLAGS = -Wall -O2

OBJ = main.o common.o graph.o geomkmeans.o graphkmeans.o metrics.o graphhierarchical.o geometrichierarchical.o clustering.o

# Targets

main: $(OBJ)
	$(CXX) $(CXXFLAGS) -o kmeans $(OBJ)


main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp
	


clean :
	rm -f $(OBJ)
	rm -f kmeans
	rm -f kmeans.exe	
	