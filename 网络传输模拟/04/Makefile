CC = g++
MAKE = make
RM = rm -f
RMRF = rm -rf
FLAGS = -std=c++11

ROUTE = ../common

DataLinkCacheDir = toDatalinkCache
NetworkCacheDir = toNetworkCache

NETWORK-NAME = network
NETWORK-SRC = protocol4_$(NETWORK-NAME).cpp
NETWORK-TARGET = protocol4_$(NETWORK-NAME)

DATALINK-NAME = datalink
DATALINK-SRC = protocol4_$(DATALINK-NAME).cpp
DATALINK-TARGET = protocol4_$(DATALINK-NAME)

PHYSICAL-NAME = physical
PHYSICAL-SND-SRC = protocol4_$(PHYSICAL-NAME)_c.cpp
PHYSICAL-REC-SRC = protocol4_$(PHYSICAL-NAME)_s.cpp
PHYSICAL-SND-TARGET = protocol4_$(PHYSICAL-NAME)_c
PHYSICAL-REC-TARGET = protocol4_$(PHYSICAL-NAME)_s

.PHONY: clean
	#avoid to make clean.*


all: $(NETWORK-TARGET) \
	 $(DATALINK-TARGET) \
	 $(PHYSICAL-SND-TARGET) $(NETWORK-PHYSICAL-TARGET)

# $@-target
# $^-all prerequisites
# $<-first prerequisite


$(NETWORK-TARGET): $(NETWORK-SRC)
	$(CC) $(FLAGS) -o $@ $< -L $(ROUTE) -l $(NETWORK-NAME)

$(DATALINK-TARGET): $(DATALINK-SRC)
	$(CC) $(FLAGS) -o $@ $< -L $(ROUTE) -l $(DATALINK-NAME)

$(PHYSICAL-SND-TARGET): $(PHYSICAL-SND-SRC)
	$(CC) $(FLAGS) -o $@ $< -L $(ROUTE) -l $(PHYSICAL-NAME)
$(PHYSICAL-REC-TARGET): $(PHYSICAL-REC-SRC)
	$(CC) $(FLAGS) -o $@ $< -L $(ROUTE) -l $(PHYSICAL-NAME)

clean:
	$(RM) $(NETWORK-TARGET) \
	 	  $(DATALINK-TARGET) \
	 	  $(PHYSICAL-SND-TARGET) $(NETWORK-PHYSICAL-TARGET)
	$(RMRF) $(toDatalinkCache) $(toNetworkCache)
