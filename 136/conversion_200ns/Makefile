

# WARNING: g++, ROOT (cern), pkg-config and fasterac are required to compile this

# Compile using either one of these commands : 
# 	either default, debug (with extra warning flags) or optimized 
# $ make -B 
# $ make debug -B 
# $ make opt -B 

# ----------------------------------------------------
# Modify ONLY the two next variables :

# ----------------------------------------------------
# First set the path to the Nuball2/lib/ folder
FRAMEWORK_LIB_PATH := ../../lib

# ----------------------------------------------------
# Then write down the name of the main file 
SRCEXE := main.cpp


# ==================================================
# --------------------------------------------------
# 			DONT TOUCH THE FOLLOWING !!!!
# --------------------------------------------------
# ==================================================

FASTERAC_CFLAGS := $(shell pkg-config --cflags libfasterac)
FASTERAC_LIBS   := $(shell pkg-config --libs libfasterac)

ROOT_CFLAGS     := $(shell root-config --cflags)
ROOT_LIBS       := $(shell root-config --libs)

HEADER_DIR := $(shell find $(FRAMEWORK_LIB_PATH) -type d)
HEADER_FILES := $(shell find $(HEADER_DIR) -type f -name '*.hpp' )
HEADER_INCLUDES := $(addprefix -I,$(shell find $(HEADER_DIR) -type d ))

.PHONY: debug opt

%.o: %.cpp ${HEADER_FILES}
	${CC} -c $< ${CFLAGS} ${LIBS} ${HEADER_INCLUDES}

CC        = g++
CFLAGS    = ${FASTERAC_CFLAGS} ${ROOT_CFLAGS} -pthread -std=c++17 -Wall 
LIBS      = ${FASTERAC_LIBS}   ${ROOT_LIBS}

# Set SRCEXE to the name of the source file you want to compile

# The name of the executable will be the same as the source file without the .cpp extension
EXE := $(SRCEXE:.cpp=)

default: $(EXE)

$(EXE): $(SRCEXE:.cpp=.o) ${HEADER_FILES}
	${CC} $< -o $@ ${CFLAGS} ${LIBS}

opt: CFLAGS += -O2
opt: default

debug: CFLAGS += -g -O0 -DDEBUG -Wextra -ftemplate-depth-25 #-Wold-style-cast
debug: default

clean:
	rm -f *.o
	rm -f *__*.*
	rm -f $(EXE)
