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
NETWORK-LIB = lib$(NETWORK-NAME).so

DATALINK-NAME = datalink
DATALINK-SRC = $(DATALINK-NAME).cpp
DATALINK-H = $(DATALINK-NAME).h
DATALINK-O = $(DATALINK-NAME).o
DATALINK-LIB = lib$(DATALINK-NAME).so

PHYSICAL-NAME = physical
PHYSICAL-SRC = $(PHYSICAL-NAME).cpp
PHYSICAL-H = $(PHYSICAL-NAME).h
PHYSICAL-O = $(PHYSICAL-NAME).o
PHYSICAL-LIB = lib$(PHYSICAL-NAME).so

.PHONY: clean
	#avoid to make clean.*


all: $(NETWORK-LIB) $(DATALINK-LIB) $(PHYSICAL-LIB)

# $@-target
# $^-all prerequisites
# $<-first prerequisite

$(NETWORK-LIB): $(NETWORK-O) $(COMMON-O)
	$(CC) -shared -Wl,-soname,$(NETWORK-LIB) -o $(NETWORK-LIB) $^
$(NETWORK-O): $(NETWORK-SRC) $(NETWORK-H)
	$(CC) $(FLAGS) -c -fPIC $< -o $@

$(DATALINK-LIB): $(DATALINK-O) $(COMMON-O)
	$(CC) -shared -Wl,-soname,$(DATALINK-LIB) -o $(DATALINK-LIB) $^
$(DATALINK-O): $(DATALINK-SRC) $(DATALINK-H)
	$(CC) $(FLAGS) -c -fPIC $< -o $@

$(PHYSICAL-LIB): $(PHYSICAL-O) $(COMMON-O)
	$(CC) -shared -Wl,-soname,$(PHYSICAL-LIB) -o $(PHYSICAL-LIB) $^
$(PHYSICAL-O): $(PHYSICAL-SRC) $(PHYSICAL-H)
	$(CC) $(FLAGS) -c -fPIC $< -o $@


$(COMMON-O): $(COMMON-SRC) $(COMMON-H)
	$(CC) $(FLAGS) -c -fPIC $< -o $@

clean:
	$(RM) *.o *.so
