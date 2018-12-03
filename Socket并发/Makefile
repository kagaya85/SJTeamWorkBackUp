SOURCE = $(wildcard *.cpp)
TARGETS = $(patsubst %.cpp, %, $(SOURCE)) 
FLAGS = -std=c++11
CC = g++

all:$(TARGETS) cleantxt

$(TARGETS):%:%.cpp
	$(CC) $(FLAGS) $< -o $@ 

.PHONY:clean all cleantxt
cleantxt:
	-rm -f *.txt
	-rm -rf ./txt

clean:    
	-rm $(TARGETS)
	-rm -f *.txt
	-rm -rf ./txt