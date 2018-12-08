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

.PHONY: clean
	#avoid to make clean.*


all: $(NETWORK-O) $(DATALINK-O) $(PHYSICAL-O)

# $@-target
# $^-all prerequisites
# $<-first prerequisite

$(NETWORK-O): $(NETWORK-SRC) $(NETWORK-H) $(COMMON-O)
	$(CC) $(FLAGS) -c $< -o $@

$(DATALINK-O): $(DATALINK-SRC) $(DATALINK-H) $(COMMON-O)
	$(CC) $(FLAGS) -c $< -o $@

$(PHYSICAL-O): $(PHYSICAL-SRC) $(PHYSICAL-H) $(COMMON-O)
	$(CC) $(FLAGS) -c $< -o $@


$(COMMON-O): $(COMMON-SRC) $(COMMON-H)
	$(CC) $(FLAGS) -c -fPIC $< -o $@

clean:
	$(RM) *.o *.so
