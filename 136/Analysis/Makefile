ROOT_CFLAGS := $(shell root-config --cflags)
ROOT_LIBS   := $(shell root-config --libs)
OPT 			  := -march=native 

analysis: clean
analysis: analysis.cpp
	g++ -o analysis analysis.cpp $(ROOT_CFLAGS) $(ROOT_LIBS) $(OPT) -Wall -Wextra 

opt: OPT += -O3 #-flto=auto -funroll-loops -fomit-frame-pointer
opt: analysis

debug: OPT += -g -DDEBUG -fno-inline
debug: analysis

gen_opt: OPT += -fprofile-generate
gen_opt: cleanOpt
gen_opt: opt

use_opt: OPT += -fprofile-use
use_opt: clean
use_opt: opt

clean:
	rm -f analysis

cleanOpt:
	rm -f *.gcda *.gcno