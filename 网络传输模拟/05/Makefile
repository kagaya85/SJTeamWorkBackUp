CC = g++
MAKE = make
RM = rm -f
RMRF = rm -rf
FLAGS = -std=c++11

ROUTE = ../common/

DataLinkCacheDir = toDatalinkCache
NetworkCacheDir = toNetworkCache

NETWORK-NAME = network
NETWORK-SRC = protocol5_$(NETWORK-NAME).cpp
NETWORK-TARGET = protocol5_$(NETWORK-NAME)
NETWORK-O = $(ROUTE)$(NETWORK-NAME).o

DATALINK-NAME = datalink
DATALINK-SRC = protocol5_$(DATALINK-NAME).cpp
DATALINK-TARGET = protocol5_$(DATALINK-NAME)
DATALINK-O = $(ROUTE)$(DATALINK-NAME).o

PHYSICAL-NAME = physical
PHYSICAL-SND-SRC = protocol5_$(PHYSICAL-NAME)_c.cpp
PHYSICAL-REC-SRC = protocol5_$(PHYSICAL-NAME)_s.cpp
PHYSICAL-SND-TARGET = protocol5_$(PHYSICAL-NAME)_c
PHYSICAL-REC-TARGET = protocol5_$(PHYSICAL-NAME)_s
PHYSICAL-O = $(ROUTE)$(PHYSICAL-NAME).o

.PHONY: clean
	#avoid to make clean.*


all: $(NETWORK-TARGET) \
	 $(DATALINK-TARGET) \
	 $(PHYSICAL-SND-TARGET) $(NETWORK-PHYSICAL-TARGET)

# $@-target
# $^-all prerequisites
# $<-first prerequisite


$(NETWORK-TARGET): $(NETWORK-SRC)
	$(CC) $(FLAGS) -o $@ $< $(NETWORK-O)

$(DATALINK-TARGET): $(DATALINK-SRC)
	$(CC) $(FLAGS) -o $@ $< $(DATALINK-O)

$(PHYSICAL-SND-TARGET): $(PHYSICAL-SND-SRC)
	$(CC) $(FLAGS) -o $@ $< $(PHYSICAL-O)
$(PHYSICAL-REC-TARGET): $(PHYSICAL-REC-SRC)
	$(CC) $(FLAGS) -o $@ $< $(PHYSICAL-O)

clean:
	$(RM) $(NETWORK-TARGET) \
	 	  $(DATALINK-TARGET) \
	 	  $(PHYSICAL-SND-TARGET) $(NETWORK-PHYSICAL-TARGET)
	$(RMRF) $(toDatalinkCache) $(toNetworkCache)
