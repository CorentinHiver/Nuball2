

# WARNING: g++, ROOT (cern), pkg-config and fasterac are required to compile this
# It is better to compile everything in c++ > 17

# Compile using either one of these commands : 
# 	either default, debug (with extra warning flags) or optimized 
# $ make -B 
# $ make debug -B 
# $ make opt -B 

# ==================================================
# ----------------------------------------------------
# Modify ONLY the two next variables :

# ----------------------------------------------------
# Path to the Nuball2/lib/ folder :
FRAMEWORK_LIB_PATH := ../../lib

# ----------------------------------------------------
# Then write down the name of the main file  :
SRCEXE := main.cpp
# ==================================================


# ==================================================
# --------------------------------------------------
# 			DONT TOUCH THE FOLLOWING !!!!
# --------------------------------------------------
# ==================================================

FASTERAC_CFLAGS := $(shell pkg-config --cflags libfasterac)
FASTERAC_LIBS   := $(shell pkg-config --libs   libfasterac)

ROOT_CFLAGS     := $(shell root-config --cflags)
ROOT_LIBS       := $(shell root-config --libs)

HEADER_DIR := $(shell find $(FRAMEWORK_LIB_PATH) -type d)
HEADER_FILES := $(shell find $(HEADER_DIR) -type f -name '*.hpp' )
HEADER_INCLUDES := $(addprefix -I,$(shell find $(HEADER_DIR) -type d ))

.PHONY: debug opt

%.o: %.cpp ${HEADER_FILES}
	${CC} -c $< ${CFLAGS} ${LIBS} ${HEADER_INCLUDES}

CC        = g++
CFLAGS    = ${FASTERAC_CFLAGS} ${ROOT_CFLAGS} -pthread -std=c++17 -std=c++20 -Wall -lstdc++ -ltbb
LIBS      = ${FASTERAC_LIBS}   ${ROOT_LIBS}

# Set SRCEXE to the name of the source file you want to compile

# The name of the executable will be the same as the source file without the .cpp extension
EXE := $(SRCEXE:.cpp=)

default: $(EXE)
	rm -f *.o

$(EXE): $(SRCEXE:.cpp=.o) ${HEADER_FILES}
	${CC} $< -o $@ ${CFLAGS} ${LIBS}

opt: CFLAGS += -O2
opt: default

opt_clang: CC = clang
opt_clang: CFLAGS += -Wno-unused-command-line-argument
opt_clang: opt

debug: CFLAGS += -g -O0 -DDEBUG -Wextra -ftemplate-depth-50 #-Wold-style-cast -fno-inline
debug: default

debug_threads: CFLAGS += -fsanitize=thread
debug_threads: debug

debugopt: CFLAGS += -g -O2 -DDEBUG -Wextra -ftemplate-depth-25 #-Wold-style-cast -fno-inline
debugopt: default

harddebug: CFLAGS += -g -O0 -DDEBUG -Wextra -ftemplate-depth-25 -Wconversion -Wno-float-conversion -Wold-style-cast # -fno-inline
harddebug: default

debug_clang: CC = clang
debug_clang: CFLAGS += -Wno-unused-command-line-argument
debug_clang: debug

debug_clang_threads: CFLAGS += -fsanitize=thread
debug_clang_threads: debug_clang

debug_clang_16: CC = clang
debug_clang_16: CFLAGS += -Wno-unused-command-line-argument
debug_clang_16: debug

debugvalgrind: CFLAGS += -DDEBUGVALGRIND
debugvalgrind: debug

clean:
	rm -f *.o
	rm -f *__*.*
	rm -f $(EXE)
