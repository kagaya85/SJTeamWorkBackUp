CC = g++
MAKE = make
RM = rm -f

FLAGS = -std=c++11

COMMON-NAME = common
COMMON-SRC = $(COMMON-NAME).cpp
COMMON-H = $(COMMON-NAME).h
COMMON-O = $(COMMON-NAME).o

NETWORK-NAME = network
NETWORK-SRC = $(NETWORK-NAME).cpp
NETWORK-H = $(NETWORK-NAME).h
NETWORK-O = $(NETWORK-NAME).o

DATALINK-NAME = datalink
DATALINK-SRC = $(DATALINK-NAME).cpp
DATALINK-H = $(DATALINK-NAME).h
DATALINK-O = $(DATALINK-NAME).o

PHYSICAL-NAME = physical
PHYSICAL-SRC = $(PHYSICAL-NAME).cpp
PHYSICAL-H = $(PHYSICAL-NAME).h
PHYSICAL-O = $(PHYSICAL-NAME).o

.PHONY: all clean
	#avoid to make clean.*


all: $(NETWORK-O) $(DATALINK-O) $(PHYSICAL-O)

# $@-target
# $^-all prerequisites
# $<-first prerequisite

$(NETWORK-O): $(NETWORK-SRC)
	$(CC) $(FLAGS) -c $< -o $@

$(DATALINK-O): $(DATALINK-SRC)
	$(CC) $(FLAGS) -c $< -o $@

$(PHYSICAL-O): $(PHYSICAL-SRC)
	$(CC) $(FLAGS) -c $< -o $@

$(COMMON-O): $(COMMON-SRC) $(COMMON-H)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	$(RM) *.o
