# Variables

CXX = g++
CXXFLAGS = -Wall -Wno-unknown-pragmas -pipe -O3 -std=c++11

HEADERS = \
src/clustering.h \
src/cmdargs.h \
src/common.h \
src/dot_parser.h \
src/graph.h \
src/metrics.h \
src/point.h \
src/random_utils.h \
src/rectangle.h \
src/segment.h \
src/infomap/Greedy.h \
src/infomap/IMNode.h \
src/infomap/infohiermap.h \

SOURCES = \
src/betweenness.cpp \
src/clustering.cpp \
src/common.cpp \
src/geometrichierarchical.cpp \
src/geomkmeans.cpp \
src/graph.cpp \
src/graphhierarchical.cpp \
src/graphkmeans.cpp \
src/infomap.cpp \
src/main.cpp \
src/metrics.cpp \
src/random_utils.cpp \
src/infomap/Greedy.cpp \
src/infomap/infohiermap.cpp \

# Targets

TARGET = kmeans

OBJECTS = $(SOURCES:src/%.cpp=build/%.o)

## Default rule executed
all: $(TARGET)
	@true

## Clean Rule
clean:
	$(RM) $(TARGET) $(OBJECTS)

noomp: $(TARGET)
	@true

## Rule for making the actual target
$(TARGET): $(OBJECTS)
	@echo "Linking object files to target $@..."
	$(CXX) $(LDFLAGS) -o $@ $^
	@echo "-- Link finished --"

## Generic compilation rule for object files from cpp files
build/%.o : src/%.cpp $(HEADERS) Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
