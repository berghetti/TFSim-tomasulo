EXEC_NAME = tfsim

ROOT=$(shell pwd)
# edit here to your systemC installation folder
SYSTEMC_LIB=$(ROOT)/systemc-2.3.3/build/src
SYSTEMC_INCLUDE=$(ROOT)/systemc-2.3.3/src
# edit here to your nana installation folder
NANA_INCLUDE=$(ROOT)/nana/include
NANA_LIB=$(ROOT)/nana/built

CXX = g++
CC ?= gcc
INCLUDEDIR = include/
SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.cpp)

OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o) \
       $(OBJ_DIR)/bpb.o

CPPFLAGS += -I $(SYSTEMC_INCLUDE) -I $(NANA_INCLUDE)
LDLFLAGS += -L $(NANA_LIB) -L $(SYSTEMC_LIB) -Wl,-rpath=$(SYSTEMC_LIB)

CFLAGS += -Wall -ggdb -O0
CXXFLAGS += -std=c++17
LIBS += -lnana -lX11 -lpthread -lrt -lXft -lpng -lasound -lfontconfig -lm -lsystemc

.PHONY: all nofs clean

all: EXE
nofs: EXE_NOFS

EXE: $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDLFLAGS) $^ $(LIBS) -lstdc++fs -o $(EXEC_NAME)

EXE_NOFS: $(OBJ)
	$(CXX) $(LDLFLAGS) $^ $(LIBS) -o $(EXEC_NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# compile C files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ)
	rm $(EXEC_NAME)
