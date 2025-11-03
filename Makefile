ROOT_CFLAGS     := $(shell root-config --cflags)
ROOT_LIBS       := $(shell root-config --libs)
# FASTERAC_CFLAGS := $(shell pkg-config --cflags libfasterac)
# FASTERAC_LIBS   := $(shell pkg-config --libs libfasterac)

exec: $(HEADERS) forDebugOnly.cpp
	g++ -o exec forDebugOnly.cpp $(ROOT_CFLAGS) $(ROOT_LIBS) $(FASTERAC_CFLAGS) $(FASTERAC_LIBS) -lm -lz -g -DDEBUG -Wall -Wextra