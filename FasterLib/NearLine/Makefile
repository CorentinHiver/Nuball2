# WARNING: g++, root-system, pkg-config and fasterac are required to compile this

FASTERAC_CFLAGS := $(shell pkg-config --cflags libfasterac)
FASTERAC_LIBS   := $(shell pkg-config --libs libfasterac)

ROOT_CFLAGS     := $(shell root-config --cflags)
ROOT_LIBS       := $(shell root-config --libs)

HEADER_DIR := $(shell find ../../lib/ ../lib -type d)
HEADER_FILES := $(shell find $(HEADER_DIR) -type f -name '*.hpp' )
# HEADER_INCLUDES := $(addprefix -I,$(shell find $(HEADER_DIR) -type d ))
HEADER_INCLUDES := $(foreach file,$(HEADER_FILES),-I$(dir $(file)))

.PHONY: debug opt

%.o: %.cpp ${HEADER_FILES}
	${CC} -c $< ${CFLAGS} ${LIBS} ${HEADER_INCLUDES}

CC        = g++
CFLAGS    = ${FASTERAC_CFLAGS} ${ROOT_CFLAGS} -pthread -std=c++17 -Wall
LIBS      = ${FASTERAC_LIBS}   ${ROOT_LIBS}

# Set SRCEXE to the name of the source file you want to compile
SRCEXE = NearLine.cpp

# The name of the executable will be the same as the source file without the .cpp extension
EXE := $(SRCEXE:.cpp=)

default: $(EXE)

$(EXE): $(SRCEXE:.cpp=.o) ${HEADER_FILES}
	${CC} $< -o $@ ${CFLAGS} ${LIBS}

opt: CFLAGS += -O2
opt: default

debug: CFLAGS += -g -O0 -DDEBUG -Wextra
debug: default

rootlogon:
	@echo "---"

clean:
	rm -f *.o
	rm -f *__*.*
	rm -f $(EXE)
