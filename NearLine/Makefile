
#
# WARNING : g++, root-system and fasterac are requiered to compile this
#


#
# If pkg-config isn't installed on your system, comment the following lines and
# set the fasterac flags as indicated with your own paths:
#
# FASTERAC_CFLAGS = -I/usr/include
# FASTERAC_LIBS   = -L/usr/lib -lfasterac
#
FASTERAC_CFLAGS = $(shell pkg-config  --cflags libfasterac)
FASTERAC_LIBS   = $(shell pkg-config  --libs   libfasterac)
#

ROOT_CFLAGS     = $(shell root-config --cflags)
ROOT_LIBS       = $(shell root-config --libs)


CC        = g++
CFLAGS    = ${FASTERAC_CFLAGS} ${ROOT_CFLAGS} -pthread -std=c++17
LIBS      = ${FASTERAC_LIBS}   ${ROOT_LIBS}

# SRCEXE    = DSSD_analyse.cpp
SRCEXE    = NearLine.cpp
# SRCEXE    = FasterToRoot.cpp
# SRCEXE    = OffLine.cpp
EXE       = $(SRCEXE:.cpp=)

all : $(EXE)

$(EXE): $(SRCEXE)
	${CC} $@.cpp -o $(EXE) ${CFLAGS} ${LIBS} -O2 -Wall  #-Wextra

rootlogon :
	@echo "---"

clean :
	rm -f *.o
	rm -f *__*.*
	rm -f $(EXE)
