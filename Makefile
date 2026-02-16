ROOT_CFLAGS := $(shell root-config --cflags)
ROOT_LIBS   := $(shell root-config --libs)
LIBS    		:= -lz
OPT 			  := -march=native 
# MT := -DMULTITHREAD=4
MT := 
TEST :=
# TEST := -ffast-math

exec: clean
exec: $(HEADERS) forDebugOnly.cpp
	g++ -o exec forDebugOnly.cpp $(ROOT_CFLAGS) $(ROOT_LIBS) $(LIBS) $(OPT) $(MT) $(TEST) -Wall -Wextra 

opt: OPT += -O3 #-flto=auto -funroll-loops -fomit-frame-pointer
opt: exec

debug: OPT += -g
debug: exec

gen_opt: OPT += -fprofile-generate
gen_opt: cleanOpt
gen_opt: opt

use_opt: OPT += -fprofile-use
use_opt: clean
use_opt: opt

clean:
	rm -f exec

cleanOpt:
	rm *.gcda *.gcno