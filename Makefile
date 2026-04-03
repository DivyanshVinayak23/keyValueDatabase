CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra

SRCS = BTree.cpp nodeStructure.cpp pager.cpp test.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = kvdb

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) test.db
