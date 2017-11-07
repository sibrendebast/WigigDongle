CC = gcc
CXX = g++
TARGET = SampleTransfer
CFLAGS = -Wall -O3 -DC_LINUX

RM = rm -rf

LIBDIR += -L$(PWD)/lib
#ARM_LIBDIR += -L$(PWD)/arm_lib

HEADER += -I$(PWD)/include -I$(shell pwd)/include -I$(shell pwd)/../include
OBJ_DIR := obj
LIBS += -lMrLoopBF -lusb-1.0 -lpthread

LBITS := $(shell getconf LONG_BIT)

ML_LIBDIR = -L$(PWD)/lib/x86_64/ -L$(shell pwd)/../lib/x86_64
ML_LPATH = ../lib/x86_64/

ML_LFLAGS = $(ML_LIBDIR) $(LIBS) -Wl,-rpath='$$ORIGIN:$$ORIGIN/lib:$$ORIGIN/$(ML_LPATH)'

CXX_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CXX_FILES:.cpp=.o)))

all: $(TARGET)

$(TARGET): $(OBJ_DIR)/$(TARGET).o
ifeq ($(ML_ARCH),arm)
	@echo "build arm"
	@#$(CXX) -o $@ $^ $(ARM_LFLAGS)
else
	@echo "build x86_64"
	@#$(CXX) -o $@ $^ $(LFLAGS)
endif
	@$(CXX) -o $@ $^ $(ML_LFLAGS)
	@$(RM) $(OBJ_DIR)

obj/%.o: $(OBJDIR) src/%.cpp
	@mkdir -p $(OBJ_DIR)
	@$(CXX) $(HEADER) $(CFLAGS) -std=c++11 $(FLAG) -c -o $@ $<

clean:
	$(RM) $(TARGET) $(OBJ_DIR)

install:
	cp ./$(TARGET) ../bin

.PHONY: all clean install
