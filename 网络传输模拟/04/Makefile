CC = g++
MAKE = make
RM = rm -f
RMRF = rm -rf
FLAGS = -std=c++11

ROUTE = ../common/

DataLinkCacheDir = toDatalinkCache
NetworkCacheDir = toNetworkCache

COMMON-NAME = common
COMMON-SRC = $(ROUTE)$(COMMON-NAME).cpp
COMMON-H = $(ROUTE)$(COMMON-NAME).h

NETWORK-NAME = network
NETWORK-SRC = $(ROUTE)$(NETWORK-NAME).cpp
NETWORK-H = $(ROUTE)$(NETWORK-NAME).h

DATALINK-NAME = datalink
DATALINK-SRC = $(ROUTE)$(DATALINK-NAME).cpp
DATALINK-H = $(ROUTE)$(DATALINK-NAME).h

PHYSICAL-NAME = physical
PHYSICAL-SRC = $(ROUTE)$(PHYSICAL-NAME).cpp
PHYSICAL-H = $(ROUTE)$(PHYSICAL-NAME).h

NETWORK-NAME = network
NETWORK-SRC = protocol4_$(NETWORK-NAME).cpp
NETWORK-TARGET = protocol4_$(NETWORK-NAME)
NETWORK-O = $(ROUTE)$(NETWORK-NAME).o

DATALINK-NAME = datalink
DATALINK-SRC = protocol4_$(DATALINK-NAME).cpp
DATALINK-TARGET = protocol4_$(DATALINK-NAME)
DATALINK-O = $(ROUTE)$(DATALINK-NAME).o

PHYSICAL-NAME = physical
PHYSICAL-SND-SRC = protocol4_$(PHYSICAL-NAME)_c.cpp
PHYSICAL-REC-SRC = protocol4_$(PHYSICAL-NAME)_s.cpp
PHYSICAL-SND-TARGET = protocol4_$(PHYSICAL-NAME)_c
PHYSICAL-REC-TARGET = protocol4_$(PHYSICAL-NAME)_s
PHYSICAL-O = $(ROUTE)$(PHYSICAL-NAME).o

.PHONY: clean
	#avoid to make clean.*


all: $(NETWORK-TARGET) \
	 $(DATALINK-TARGET) \
	 $(PHYSICAL-SND-TARGET) $(PHYSICAL-REC-TARGET)

# $@-target
# $^-all prerequisites
# $<-first prerequisite


$(NETWORK-TARGET): $(NETWORK-SRC)
	$(CC) $(FLAGS) -o $@ $(NETWORK-SRC) $(NETWORK-SRC) $(NETWORK-H) $(COMMON-SRC) $(COMMON-H)

$(DATALINK-TARGET): $(DATALINK-SRC)
	$(CC) $(FLAGS) -o $@ $(DATALINK-SRC) $(DATALINK-SRC) $(DATALINK-H) $(COMMON-SRC) $(COMMON-H)

$(PHYSICAL-SND-TARGET): $(PHYSICAL-SND-SRC)
	$(CC) $(FLAGS) -o $@ $(PHYSICAL-SND-SRC) $(PHYSICAL-SRC) $(PHYSICAL-H) $(COMMON-SRC) $(COMMON-H)
$(PHYSICAL-REC-TARGET): $(PHYSICAL-REC-SRC)
	$(CC) $(FLAGS) -o $@ $(PHYSICAL-REC-SRC) $(PHYSICAL-SRC) $(PHYSICAL-H) $(COMMON-SRC) $(COMMON-H)

clean:
	$(RM) $(NETWORK-TARGET) \
	 	  $(DATALINK-TARGET) \
	 	  $(PHYSICAL-SND-TARGET) $(PHYSICAL-REC-TARGET) \
		  *.o
	$(RMRF) $(NetworkCacheDir) $(NetworkCacheDir)

receiver:
	./receiver4_datalink &
	./receiver4_network xldg_rec.txt &
	./receiver4_physical 4000 &

sender:
	./sender4_datalink &
	./sender4_network xldg.txt &
	./sender4_physical 192.168.137.64 4000 &
