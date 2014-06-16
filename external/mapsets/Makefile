# Variables

CXX = g++
CXXFLAGS = -Wall -O2 -std=c++11

OBJ = main.o colors.o common.o comparer.o debug_utils.o fd_adjustment.o geom_utils.o graph_algorithms.o random_utils.o \
		rectangle_node.o svg_utils.o test_visibility.o tests.o time_utils.o visibility.o visibility_utils.o


# Targets

main: $(OBJ)
	$(CXX) $(CXXFLAGS) -o mapsets $(OBJ)


main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp
	


clean :
	rm -f $(OBJ)
	rm -f mapsets
	rm -f mapsets.exe
	