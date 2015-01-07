CC:=g++
EXT1:=cpp

CFLAGS:=-Wall -o
TFLAGS:=-Wno-write-strings -o
SFLAGS:=-fPIC -shared 

P1:=irouter
P2:=brouter
P3:=controller

L1:=commons

EL1:=cppunit

T1:=test

#-Wl,-soname,lib$(L1).so

SRC:=src
TARGET:=bin
LIB:=lib
TESTS:=tests
FILES:=files

INC:=-I $(SRC)/$(L1)

all: $(P1) $(P2) $(P3)

$(P1): so
	$(CC) $(INC)/ -L$(LIB)/ $(SRC)/$(P1)/*.$(EXT1) -l$(L1) $(CFLAGS) $(TARGET)/$(P1)

$(P2): so
	$(CC) $(INC)/ -L$(LIB)/ $(SRC)/$(P2)/*.$(EXT1) -l$(L1) $(CFLAGS) $(TARGET)/$(P2) 

$(P3): so 
	$(CC) $(INC)/ -L$(LIB)/ $(SRC)/$(P3)/*.$(EXT1) -l$(L1) $(CFLAGS) $(TARGET)/$(P3)
    
so:
	$(CC) $(SFLAGS) $(CFLAGS) $(LIB)/lib$(L1).so $(SRC)/$(L1)/*.$(EXT1)	

test: so
	for filename in $(shell ls $(SRC)/$(T1)) ; do \
		g++ $(INC) -L$(LIB) $(SRC)/$(T1)/$$filename -l$(L1) -l$(EL1) $(TFLAGS) $(TESTS)/$$filename ; \
		done

clean:
	rm -rf $(TARGET)/* $(LIB)/* $(TESTS)/* $(FILES)/*

.PHONY: clean
