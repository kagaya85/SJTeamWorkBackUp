CC = g++
MAKE = make
RM = rm -f
RMRF = rm -rf
FLAGS = -std=c++11

ROUTE = ../common/

DataLinkCacheDir = toDatalinkCache
NetworkCacheDir = toNetworkCache

NETWORK-NAME = network
NETWORK-SRC = protocol6_$(NETWORK-NAME).cpp
NETWORK-LIB = $(ROUTE)lib$(NETWORK-NAME).so
NETWORK-TARGET = protocol6_$(NETWORK-NAME)

DATALINK-NAME = datalink
DATALINK-SRC = protocol6_$(DATALINK-NAME).cpp
DATALINK-LIB = $(ROUTE)lib$(DATALINK-NAME).so
DATALINK-TARGET = protocol6_$(DATALINK-NAME)

PHYSICAL-NAME = physical
PHYSICAL-SND-SRC = protocol6_$(PHYSICAL-NAME)_c.cpp
PHYSICAL-REC-SRC = protocol6_$(PHYSICAL-NAME)_s.cpp
PHYSICAL-LIB = $(ROUTE)lib$(PHYSICAL-NAME).so
PHYSICAL-SND-TARGET = protocol6_$(PHYSICAL-NAME)_c
PHYSICAL-REC-TARGET = protocol6_$(PHYSICAL-NAME)_s

.PHONY: clean
	#avoid to make clean.*


all: $(NETWORK-TARGET) \
	 $(DATALINK-TARGET) \
	 $(PHYSICAL-SND-TARGET) $(NETWORK-PHYSICAL-TARGET)

# $@-target
# $^-all prerequisites
# $<-first prerequisite


$(NETWORK-TARGET): $(NETWORK-SRC) $(NETWORK-LIB)
	$(CC) $(FLAGS) -o $@ $< -L . -l $(NETWORK-NAME)

$(DATALINK-TARGET): $(DATALINK-SRC) $(DATALINK-LIB)
	$(CC) $(FLAGS) -o $@ $< -L . -l $(DATALINK-NAME)

$(PHYSICAL-SND-TARGET): $(PHYSICAL-SND-SRC) $(PHYSICAL-LIB)
	$(CC) $(FLAGS) -o $@ $< -L . -l $(PHYSICAL-NAME)
$(PHYSICAL-REC-TARGET): $(PHYSICAL-REC-SRC) $(PHYSICAL-LIB)
	$(CC) $(FLAGS) -o $@ $< -L . -l $(PHYSICAL-NAME)

clean:
	$(RM) $(NETWORK-TARGET) \
	 	  $(DATALINK-TARGET) \
	 	  $(PHYSICAL-SND-TARGET) $(NETWORK-PHYSICAL-TARGET)
	$(RMRF) $(toDatalinkCache) $(toNetworkCache)
